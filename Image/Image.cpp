#include "Image.h"
#include <cstdio> // For FILE, which is used by libpng
#include <QStringList>
#include <png.h>
#include "../Utils/ScopeExit.h"
#include "../Utils/MathUtils.h"
#include "../GUI/ProgressDialog.h"

// Needed to check the progress of opening/closing
size_t g_rowsCount = 0;
size_t g_passesCount = 0;

Image::Image() {

}

void Image::copyFrom(Image& other) {
    if (m_data.isEmpty() || m_width != other.getWidth() || m_height != other.getHeight()) {
        return;
    }

    std::memcpy(m_data.data(), other.m_data.data(), size_t(m_width) * m_height * m_data.ElemSize);
}

utils::Result<Void> Image::allocateImage(uint32_t width, uint32_t height) {
    if (auto result = m_data.realloc(width * height); !result.isOk()) {
        return result.extractError();
    }

    m_width = width;
    m_height = height;

    return utils::Success();
}

utils::Result<Void> Image::open(const QString& fileName) {
    m_fileName = fileName;

    QString extension = fileName.split('.').back();
    if (extension == "png") {
        return openPng(fileName);
    }

    return utils::Failure("Unknown file extension: " + fileName);
}

utils::Result<Void> Image::store(const QString& fileName) {
    m_fileName = fileName;

    QString extension = fileName.split('.').back();
    if (extension == "png") {
        return storePng(fileName);
    }

    return utils::Failure("Unknown file extension: " + fileName);
}

utils::Result<Void> Image::newImage(uint32_t width, uint32_t height, Color fillColor) {
    if (width * height > IMAGE_SIZE_MAX) {
        return utils::Failure(
            "Image size is too big: "
            + QString::number(width)
            + " x "
            + QString::number(height)
            + "; Maximal size is "
            + QString::number(IMAGE_SIZE_MAX)
            + " pixels"
        );
    }

    if (auto result = allocateImage(width, height); !result.isOk()) {
        return result.extractError();
    }

    m_fileName = "";
    m_extension = Extension::NONE;

    for (uint32_t y = 0; y < m_height; y++) {
        drawQuickXStroke(0, y, m_width, fillColor);
    }

    return utils::Success();
}

utils::Result<QImage> Image::toQImage() const {
    if (m_data.isEmpty()) {
        return utils::Failure("Image is empty: nothing to return");
    }

    auto img = QImage((uint8_t*)m_data.data(), m_width, m_height, QImage::Format_ARGB32);
    return img;
}

void Image::channelFilter(Color::Channel channel, uint8_t value) {
    Color changeColor = Color::getColorByChannels(channel, value);
    auto mask = Color::getChannelMask(channel);

    for (uint32_t y = 0; y < m_height; y++) {
        for (uint32_t x = 0; x < m_width; x++) {
            atFast(x, y).setColorWithMask(mask, changeColor);
        }
    }
}

void Image::splitImageIntoSubimages(uint32_t N, uint32_t M, const QString& basicName) {
    Image tmp;
    float gridWidth = m_width / float(N);
    float gridHeight = m_height / float(M);

    tmp.allocateImage(int(ceil(gridWidth)), int(ceil(gridHeight)));

    for (uint32_t x = 0; x < N; x++) {
        int fromX = int(x * gridWidth);
        int toX = int((x + 1) * gridWidth);

        for (uint32_t y = 0; y < M; y++) {
            int fromY = int(y * gridHeight);
            int toY = int((y + 1) * gridHeight);

            fillWithSubimage(tmp, QPoint(fromX, fromY), toX - fromX, toY - fromY)
                .store(basicName + "_" + QString::number(x) + "_" + QString::number(y) + ".png")
                .isOk(); // Whether it was a successful store, must be handled later
        }
    }
}

void Image::splitImageWithGrid(uint32_t N, uint32_t M, float lineWidth, Color color) {
    float gridWidth = m_width / float(N);
    float gridHeight = m_height / float(M);

    for (float x = gridWidth; x < m_width; x += gridWidth) {
        drawVerticalLine(QPoint(int(x), 0), m_height, lineWidth, color);
    }

    for (float y = gridHeight; y < m_height; y += gridHeight) {
        drawHorizontalLine(QPoint(0, int(y)), m_width, lineWidth, color);
    }
}

void Image::drawComplexCircle(QPoint center, float radius, float lineWidth, Color lineColor, Color fillColor, bool needsFill) {
    if (needsFill) {
        fillCircle(center, radius, fillColor);
    }

    drawCircle(center, radius, lineWidth, lineColor);
}

void Image::drawSquareWithDiagonals(QPoint from, uint32_t side, float lineWidth, Color lineColor, Color fillColor, bool needsFill) {
    if (needsFill) {
        fillRect(from, side, side, fillColor);
    }

    drawRect(from, side, side, lineWidth, lineColor);
    drawLine(from, QPoint(from.x() + side, from.y() + side), lineWidth, lineColor);
    drawLine(QPoint(from.x() + side, from.y()), QPoint(from.x(), from.y() + side), lineWidth, lineColor);
}

void Image::emphasizeWhiteAreas(Color lineColor) {
    for (uint32_t x = 0; x < m_width; x++) {
        for (uint32_t y = 0; y < m_height; y++) {
            if (atFast(x, y) != Color::White && hasColorInNeighborhood(x, y, Color::White)) {
                atFast(x, y) = lineColor;
            }
        }
    }
}

void Image::drawLine(QPoint from, QPoint to, float width, Color color) {
    assert(!m_data.isEmpty() && "Image is empty");

    if (from == to) {
        return;
    }

    // This way, we ensure that from is before to along the Y axis
    if (from.y() > to.y()) {
        std::swap(from, to); // So as to from would be before the to
    }

    if (from.y() == to.y()) {
        if (from.x() > to.x()) {
            std::swap(from, to); // So as to from would be before the to
        }

        drawHorizontalLine(from, to.x() - from.x(), width, color);
        return;
    }

    if (from.x() == to.x()) {
        drawVerticalLine(from, to.y() - from.y(), width, color);
        return;
    }

    // If there is no coinciding coordinates, the line should be drawn fairly
    if (width < 1) {
        drawThinLine(from, to, color);
    } else {
        drawWideLine(from, to, width, color);
    }
}

void Image::drawVerticalLine(QPoint from, uint32_t yOffset, float width, Color color) {
    assert(!m_data.isEmpty() && "Image is empty");

    uint32_t xOffset = int(width);
    from.setX(from.x() - int(width / 2));

    fillRect(from, xOffset, yOffset, color);
}

void Image::drawHorizontalLine(QPoint from, uint32_t xOffset, float height, Color color) {
    assert(!m_data.isEmpty() && "Image is empty");

    uint32_t yOffset = int(height);
    from.setY(from.y() - int(height / 2));

    fillRect(from, xOffset, yOffset, color);
}

void Image::drawRect(QPoint from, uint32_t width, uint32_t height, float lineWidth, Color color) {
    assert(!m_data.isEmpty() && "Image is empty");

    if (from.y() >= m_height || from.x() >= m_width || from.y() + height <= 0 || from.x() + width <= 0) {
        return;
    }

    drawRasterized(utils::HollowRectRasterizer(from, { from.x() + (int)width, from.y() + (int)height }, lineWidth), color);
}

void Image::drawCircle(QPointF center, float radius, float lineWidth, Color color) {
    assert(!m_data.isEmpty() && "Image is empty");

    if (center.x() + radius < 0 || center.y() + radius < 0) {
        return;
    }

    drawRasterized(utils::RingRasterizer(center.toPoint(), radius, lineWidth), color);
}

void Image::fillRect(QPoint from, uint32_t width, uint32_t height, Color color) {
    assert(!m_data.isEmpty() && "Image is empty");

    if (from.y() >= m_height || from.x() >= m_width || from.y() + height <= 0 || from.x() + width <= 0) {
        return;
    }

    drawRasterized(utils::RectRasterizer(from, {from.x() + (int)width - 1, from.y() + (int)height - 1}), color);
}

void Image::fillCircle(QPointF center, float radius, Color color) {
    assert(!m_data.isEmpty() && "Image is empty");

    if (center.x() + radius < 0 || center.y() + radius < 0) {
        return;
    }

    if (radius <= 0.5) {
        drawXStroke(center.x(), center.y(), 1, color);
        return;
    }

    drawRasterized(utils::CircleRasterizer(center.toPoint(), radius), color);
}

void Image::drawXStroke(uint32_t fromX, uint32_t fromY, uint32_t length, Color color) {
    assert(length < INT32_MAX); // Checking for overflow

    if (fromY >= m_height || fromX >= m_width) {
        return; // The from point is out of image, but x can be < 0
    }

    // No color mixing for the time being
    //if (color.a() == 255) { // Can just fill in the color
    drawQuickXStroke(fromX, fromY, length, color);
    /* } else {
        uint32_t toX = utils::min(m_width, fromX + length);
        length = toX - fromX;

        Color* rowPtr = &atFast(fromX, fromY);
        for (uint32_t x = 0; x < length; x++) {
            rowPtr[x].mix(color);
        }
    }*/
}

// Color is supposed to have alpha = 255
void Image::drawQuickXStroke(uint32_t fromX, uint32_t fromY, uint32_t length, Color color) {
    uint32_t toX = utils::min(m_width, fromX + length);
    length = toX - fromX;

    // Fast fill by 2 pixels at a time
    if (length > 1) {
        // 2 pixels value
        uint64_t fillValue = uint64_t(color._data) << 32 | color._data;

        uint64_t* fromPtr = (uint64_t*)&atFast(fromX, fromY);
        uint64_t* toPtr = fromPtr + length / 2;
        for (uint64_t* pixPtr = fromPtr; pixPtr < toPtr; pixPtr++) {
            *pixPtr = fillValue;
        }
    }

    // For an odd length there is one more pixel to fill
    if (length % 2) {
        atFast(toX - 1, fromY) = color;
    }
}

bool Image::hasColorInNeighborhood(uint32_t x, uint32_t y, Color color) {
    for (int ox = -1; ox <= 1; ox++) {
        for (int oy = -1; oy <= 1; oy++) {
            int pix_x = int(x) + ox;
            int pix_y = int(y) + oy;

            if (pix_x >= 0 && pix_x < m_width && pix_y >= 0 && pix_y < m_height) {
                if (atFast(uint32_t(pix_x), uint32_t(pix_y)) == color) {
                    return true;
                }
            }
        }
    }

    return false;
}

Color& Image::at(uint32_t x, uint32_t y) {
    assert(x < m_width && y < m_height);
    return atFast(x, y);
}

Color Image::at(uint32_t x, uint32_t y) const {
    assert(x < m_width && y < m_height);
    return atFast(x, y);
}

Color& Image::atFast(uint32_t x, uint32_t y) {
    return *(Color*)&m_data.at(size_t(y) * m_width + x);
}

Color Image::atFast(uint32_t x, uint32_t y) const {
    return *(Color*)&m_data.at(size_t(y) * m_width + x);
}

QRect Image::getRect() const {
    return QRect(0, 0, m_width, m_height);
}

QSize Image::getSize() const {
    return QSize(m_width, m_height);
}

uint32_t Image::getWidth() const {
    return m_width;
}

uint32_t Image::getHeight() const {
    return m_height;
}

const QString& Image::getFileName() const {
    return m_fileName;
}

QList<QPair<QString, QString>> Image::getFullImageInformation() const {
    QList<QPair<QString, QString>> result;
    result.push_back(QPair("Path", m_fileName.isEmpty() ? "a new image, no path" : m_fileName));
    result.push_back(QPair("Extension", getExtensionAsString()));
    result.push_back(QPair("Width", QString::number(m_width)));
    result.push_back(QPair("Height", QString::number(m_height)));
    result.push_back(QPair("Bit depth", QString::number(m_bitDepth == -1 ? 8 : m_bitDepth)));
    result.push_back(QPair("Color type", getColorTypeAsString()));

    if (m_extension == Extension::PNG) {
        result.push_back(QPair("Interlace method", m_pngFileData.getInterlaceMethodAsString()));
        result.push_back(QPair("Filtering method", m_pngFileData.getFilterMethodAsString()));
        result.push_back(QPair("Compression method", m_pngFileData.getCompressionMethodAsString()));
    }

    return result;
}

void own_png_progress_callback(png_structp png_ptr, png_uint_32 row, int pass) {
    double progress = double(pass * g_rowsCount + row) / (g_passesCount * g_rowsCount);
    progressCallbackFunc(progress * 100);
}

utils::Result<Void> Image::openPng(const QString& fileName) {
    FILE* file = fopen(fileName.toLocal8Bit().data(), "rb");
    if (!file) {
        return utils::Failure("failed to open the file: " + fileName);
    }

    // Closing the file at return
    at_scope_exit{ fclose(file); };

    // Checking whether it is an actual png or just a renamed file
    uint8_t header[9];
    fread(header, 1, 8, file);
    if (png_sig_cmp(header, 0, 9)) {
        return utils::Failure("Wrong png signature, probably not a png file: " + fileName);
    }

    // Reading information from the png
    // The structure to read the png
    png_structp png_sp = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_sp) {
        return utils::Failure("Libpng error: png_create_read_struct");
    }

    // The structure to contain the information about the png
    png_infop png_ip = png_create_info_struct(png_sp);

    // Destroying libpng data on return
    at_scope_exit{ png_destroy_read_struct(&png_sp, &png_ip, NULL); };

    if (!png_ip) {
        return utils::Failure("Libpng error: png_create_info_struct");
    }

    // Initializing I/O for png file
    png_init_io(png_sp, file);
    png_set_sig_bytes(png_sp, 8); // Standard procedure of telling libpng how many header's bytes were read previously

    if (setjmp(png_jmpbuf(png_sp))) {
        return utils::Failure("Internal libpng error while initializing I/O");
    }

    // Reading the png
    // First of all, the information on the png
    png_read_info(png_sp, png_ip);
    
    png_get_IHDR(
        png_sp, 
        png_ip, 
        (png_uint_32p)&m_width, 
        (png_uint_32p)&m_height, 
        &m_bitDepth, 
        &m_colorType, 
        &m_pngFileData.interlace_method, 
        &m_pngFileData.compression_method, 
        &m_pngFileData.filter_method
    );

    if (setjmp(png_jmpbuf(png_sp))) {
        return utils::Failure("Internal libpng error while reading png");
    }

    if (m_width * m_height > IMAGE_SIZE_MAX) {
        return utils::Failure(
            "Image size is too big: "
            + QString::number(m_width)
            + " x "
            + QString::number(m_height)
            + "; Maximal size is "
            + QString::number(IMAGE_SIZE_MAX)
            + " pixels"
        );
    }

    // Setting up the libpng so that we read the image in the required format
    // Note: the Image's required format is: RGBA, 8 bit depth
    if (m_bitDepth == 16) { // in case the image is in 16 bit depth format, we strip it to 8 bit depth
        png_set_strip_16(png_sp);
    }

    if (m_colorType == PNG_COLOR_TYPE_PALETTE) { // Set the format to RGB
        png_set_palette_to_rgb(png_sp);
    }

    if (m_colorType == PNG_COLOR_TYPE_GRAY && m_bitDepth < 8) { // Lower bit depth gray to 8 bit depth
        png_set_expand_gray_1_2_4_to_8(png_sp);
    }

    if (png_get_valid(png_sp, png_ip, PNG_INFO_tRNS)) { // Some internal convertion related to data chunks. Not sure that I actually understand how it works
        png_set_tRNS_to_alpha(png_sp);
    }

    if (m_colorType == PNG_COLOR_TYPE_RGB ||
        m_colorType == PNG_COLOR_TYPE_GRAY ||
        m_colorType == PNG_COLOR_TYPE_PALETTE) { // Add the alpha channel for the formats that do not have one
        png_set_filler(png_sp, 0xFF, PNG_FILLER_AFTER);
    }

    if (m_colorType == PNG_COLOR_TYPE_GRAY ||
        m_colorType == PNG_COLOR_TYPE_GRAY_ALPHA) { // Gray to RGB
        png_set_gray_to_rgb(png_sp);
    }

    png_read_update_info(png_sp, png_ip); // Updating the png_ip

    // And now we can read the image itself, finally
    utils::SizedBuffer2D<png_byte> rows;
    const png_uint_32 rowbytes = png_get_rowbytes(png_sp, png_ip);
    if (auto result = utils::SizedBuffer2D<png_byte>::allocNonInitialized(m_height, rowbytes); !result.isOk()) {
        return result.extractError();
    } else {
        rows = result.extract();
    }

    // Setting up the progress check
    if (g_isProgressDialogOpen) {
        g_passesCount = m_pngFileData.interlace_method == PNG_INTERLACE_ADAM7 ? PNG_INTERLACE_ADAM7_PASSES : 1;
        g_rowsCount = m_height;
        png_set_read_status_fn(png_sp, own_png_progress_callback);
    }

    png_set_rows(png_sp, png_ip, rows.data());
    png_read_image(png_sp, rows.data());

    // Now all we have to do is to convert the read data to our internal format
    if (auto result = m_data.realloc(m_width * m_height); !result.isOk()) {
        return result.extractError();
    }

    for (uint32_t y = 0; y < m_height; y++) {
        png_bytep row = rows[y];
        for (uint32_t x = 0; x < m_width; x++) {
            Color &pix = atFast(x, y);
            uint32_t byte_idx = x * 4;

            pix.r() = row[byte_idx];
            pix.g() = row[byte_idx + 1];
            pix.b() = row[byte_idx + 2];
            pix.a() = row[byte_idx + 3];
        }
    }

    m_extension = Extension::PNG;
    return utils::Success();
}

utils::Result<Void> Image::storePng(const QString& fileName) {
    FILE* file = fopen(fileName.toLocal8Bit().data(), "wb");
    if (!file) {
        return utils::Failure("failed to open the file: " + fileName);
    }

    // Closing the file at return
    at_scope_exit{ fclose(file); };

    // The structure to write png
    png_structp png_sp = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_sp) {
        return utils::Failure("Libpng error: png_create_write_struct");
    }

    // The structure to contain the information about the png
    png_infop png_ip = png_create_info_struct(png_sp);

    // Destroying libpng data on return
    at_scope_exit{ png_destroy_read_struct(&png_sp, &png_ip, NULL); };

    if (!png_ip) {
        return utils::Failure("Libpng error: png_create_info_struct");
    }

    // Initializing I/O for png file
    png_init_io(png_sp, file);

    if (setjmp(png_jmpbuf(png_sp))) {
        return utils::Failure("Internal libpng error while initializing I/O");
    }

    // Writing the png
    // First of all, the information on the png
    int interlaceMethod = m_extension == Extension::PNG ? m_pngFileData.interlace_method : PNG_INTERLACE_NONE;
    int compressionMethod = m_extension == Extension::PNG ? m_pngFileData.compression_method : PNG_COMPRESSION_TYPE_BASE;
    int filterMethod = m_extension == Extension::PNG ? m_pngFileData.filter_method : PNG_FILTER_TYPE_BASE;
    png_set_IHDR(
        png_sp,
        png_ip,
        (png_uint_32)m_width,
        (png_uint_32)m_height,
        8,
        PNG_COLOR_TYPE_RGBA,
        interlaceMethod,
        compressionMethod,
        filterMethod
    );

    if (setjmp(png_jmpbuf(png_sp))) {
        return utils::Failure("Internal libpng error while writing png");
    }

    // Allocating memory and filling it with pixels' data
    utils::SizedBuffer2D<png_byte> rows;
    const png_uint_32 rowbytes = png_get_rowbytes(png_sp, png_ip);
    if (auto result = utils::SizedBuffer2D<png_byte>::allocNonInitialized(m_height, rowbytes); !result.isOk()) {
        return result.extractError();
    } else {
        rows = result.extract();
    }

    for (uint32_t y = 0; y < m_height; y++) {
        for (uint32_t x = 0; x < m_width; x++) {
            Color pix = atFast(x, y);
            png_bytep byte_p = &rows[y][x * 4];

            byte_p[0] = pix.r();
            byte_p[1] = pix.g();
            byte_p[2] = pix.b();
            byte_p[3] = pix.a();
        }
    }

    // Setting up the progress check
    if (g_isProgressDialogOpen) {
        g_passesCount = (m_extension == Extension::PNG && m_pngFileData.interlace_method == PNG_INTERLACE_ADAM7)
            ? PNG_INTERLACE_ADAM7_PASSES : 1;
        g_rowsCount = m_height;
        png_set_write_status_fn(png_sp, own_png_progress_callback);
    }

    // Writing the image pixels
    png_write_info(png_sp, png_ip);

    png_set_rows(png_sp, png_ip, rows.data());
    png_write_image(png_sp, rows.data());
    png_write_end(png_sp, png_ip);

    if (setjmp(png_jmpbuf(png_sp))) {
        return utils::Failure("Internal libpng error in the end of writing");
    }

    m_extension = Extension::PNG;
    return utils::Success();
}

Image& Image::fillWithSubimage(Image& result, QPoint from, uint32_t width, uint32_t height) {
    assert(from.x() >= 0 && from.y() >= 0);
    assert(from.x() + width <= m_width);
    assert(from.y() + height <= m_height);
    assert(result.m_width >= width && result.m_height >= height);

    result.m_width = width;
    result.m_height = height;

    for (uint32_t y = from.y(); y < from.y() + height; y++) {
        Color* rowPtrSrc = &atFast(width, y);
        Color* rowPtrDst = &result.atFast(0, y - from.y());

        std::memcpy(rowPtrDst, rowPtrSrc, width * sizeof(uint32_t));
    }

    return result;
}

void Image::drawWideLine(QPoint from, QPoint to, float width, Color color) {
    drawRasterized(utils::WideLineRasterizer(from, to, width), color);
}

void Image::drawThinLine(QPoint from, QPoint to, Color color) {
    drawRasterized(utils::LineRasterizer(from, to, m_width, m_height), color);
}

QString Image::getColorTypeAsString() const {
    switch (m_colorType) {
    case PNG_COLOR_TYPE_GRAY: return "G (gray)";
    case PNG_COLOR_TYPE_RGB: return  "RGB (red-green-blue)";
    case PNG_COLOR_TYPE_PALETTE: return "palette - indexed colors";
    case PNG_COLOR_TYPE_GRAY_ALPHA: return "GA (gray-alpha)";
    case PNG_COLOR_TYPE_RGB_ALPHA: return "RGBA (red-green-blue-alpha)";
    default: return "RGBA (red-green-blue-alpha)";
    }
}

QString Image::getExtensionAsString() const {
    switch (m_extension) {
    case Image::Extension::NONE: return "none - no image loaded from file";
    case Image::Extension::PNG: return "PNG - Portable Network Graphics";
    case Image::Extension::JPG: return "JP(E)G = Joint Photographic (Experts) Group";
    case Image::Extension::BMP: return "BMP - BitMap Picture";
    default: return "unknown extension";
    }
}

QString Image::PngFileData::getInterlaceMethodAsString() const {
    switch (interlace_method) {
        case PNG_INTERLACE_NONE: return "none";
        case PNG_INTERLACE_ADAM7: return "adam 7 algorithm";
        case PNG_INTERLACE_LAST: // incorrect value
    default: return "unknown interlace method";
    }
}

QString Image::PngFileData::getCompressionMethodAsString() const {
    switch (compression_method) {
        case PNG_COMPRESSION_TYPE_BASE: return "basic png compression method";
    default: return "unknown compression method";
    }
}

QString Image::PngFileData::getFilterMethodAsString() const {
    switch (filter_method) {
        case PNG_FILTER_TYPE_BASE: return "basic png filtering method";
        case PNG_INTRAPIXEL_DIFFERENCING: return "intrapixel differencing";
    default: return "unknown filtering method";
    }
}
