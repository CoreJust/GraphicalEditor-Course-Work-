#include "ImageEditScene.h"
#include <QPixmap.h>
#include <QGraphicsSceneMouseEvent>
#include <QScrollBar>
#include <QGraphicsItem>
#include "../Utils/MathUtils.h"

constexpr qreal MAX_ZOOM_OUT = 1000;
constexpr qreal MAX_ZOOM_IN = 0.01;

ImageEditScene::ImageEditScene(
	QObject* parent,
	QLabel* isSavedHint,
	QLabel* pixelHint,
	QLabel* sizeHint,
	ColorLabel* colorLabel
) :
	QGraphicsScene(parent),
	m_isImageSavedHint(isSavedHint),
	m_pixelUnderCursorHint(pixelHint),
	m_imageSizeHint(sizeHint),
	m_colorLabel(colorLabel) {
	
}

// Addidng and setting up the QGraphicsView
// Probably it'd better be to have ImageEdit based on QGraphicView, but for time being so be it
void ImageEditScene::setParentView(QGraphicsView* pView) {
	m_pView = pView;
	m_pView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_pView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_pView->setMouseTracking(true);
	m_pView->setStyleSheet("QGraphicsView{"\
		"background-image: url(res/icon/editor_background.png);"\
		"background-repeat: repeat-xy;"\
		"background-origin: content;"\
		"background-clip: margin;"\
		"background-attachment: fixed;"\
		"background-position: center; }");
}

// Clears the loaded image and returns the image editor to the default state
// TODO: add an image to be shown by default (like "nothing here")
void ImageEditScene::clearScene() {
	clear();
	m_scale = 1;
	m_pView->scale(1, 1);
}

// Creates a new image and opens it. Not finished
utils::Result<Void> ImageEditScene::newImage(QSize size, Color fillColor) {
	clear();
	if (auto result = m_image.newImage(size.width(), size.height(), fillColor); !result.isOk()) {
		return result;
	}

	onImageOpen(true, size);

	return updateImage();
}

// Opens a new image
// If failed, returns false, otherwise true
utils::Result<Void> ImageEditScene::openImage(const QString& fileName) {
	clear();
	if (auto result = m_image.open(fileName); !result.isOk()) {
		return result;
	}

	onImageOpen(false, m_image.getSize());

	return updateImage();
}

// Saves the image to the file.
// Returns true on success and false otherwise.
utils::Result<Void> ImageEditScene::saveImage(const QString& fileName) {
	assert(m_isImageOpened);

	onImageSave();
	return m_image.store(fileName);
}

void ImageEditScene::channelFilter(bool red, bool green, bool blue, bool alpha, uint8_t value) {
	Color::Channel channel = Color::getChannel(red, green, blue, alpha);

	m_image.channelFilter(channel, value);

	onImageChange();
}

void ImageEditScene::splitImageIntoSubimages(uint32_t N, uint32_t M, const QString& basicName) {
	m_image.splitImageIntoSubimages(N, M, basicName);
}

void ImageEditScene::splitImageWithGrid(uint32_t N, uint32_t M, float lineWidth, Color color) {
	m_image.splitImageWithGrid(N, M, lineWidth, color);

	onImageChange();
}

void ImageEditScene::drawComplexCircle(
	QPoint center,
	float radius,
	float lineWidth,
	Color lineColor,
	Color fillColor,
	bool needsFill
) {
	m_image.drawComplexCircle(center, radius, lineWidth, lineColor, fillColor, needsFill);

	onImageChange();
}

void ImageEditScene::drawSquareWithDiagonals(
	QPoint from,
	uint32_t side,
	float lineWidth,
	Color lineColor,
	Color fillColor,
	bool needsFill
) {
	m_image.drawSquareWithDiagonals(from, side, lineWidth, lineColor, fillColor, needsFill);

	onImageChange();
}

void ImageEditScene::emphasizeWhiteAreas(Color lineColor) {
	m_image.emphasizeWhiteAreas(lineColor);

	onImageChange();
}

// Changing the tool used
void ImageEditScene::setToolType(ToolType toolType) {
	m_currentTool = toolType;
}

void ImageEditScene::moveImageBy(QPointF offset) {
	for (QGraphicsItem* it : m_pView->items()) {
		it->moveBy(offset.x(), offset.y());
	}

	m_imageRect.translate(offset);
	update();
}

// Does not move image if it will get out of view via this offset
void ImageEditScene::tryMoveImageBy(QPointF offset) {
	QRectF sceneBounds = m_pView->mapToScene(m_pView->viewport()->rect()).boundingRect();
	QRectF newRect = m_imageRect.translated(offset);

	if (sceneBounds.intersects(newRect)) {
		moveImageBy(offset);
	}
}

// Adding a new cursor over the current one
void ImageEditScene::pushCursor(const QCursor& cursor) {
	m_cursorsHistory.push_back(m_pView->cursor());
	m_pView->setCursor(cursor);
}

// Returning the previous cursor
void ImageEditScene::popCursor() {
	if (m_cursorsHistory.size() == 0) {
		m_pView->setCursor(Qt::ArrowCursor);
		return;
	}

	m_pView->setCursor(m_cursorsHistory.back());
	m_cursorsHistory.pop_back();
}

// Temporary, must be either rewritten or deleted
QRectF ImageEditScene::getImageRect(QSize imageSize) {
	return QRectF(0, 0, imageSize.width(), imageSize.height());
}

bool ImageEditScene::hasUnsavedImage() const {
	return m_isImageOpened && !m_isImageSaved;
}

const QString& ImageEditScene::getImageFileName() const {
	return m_image.getFileName();
}

Image& ImageEditScene::getImage() {
	return m_image;
}

bool ImageEditScene::isImageOpened() const {
	return m_isImageOpened;
}

// Whether the point is within the image.
// Accuracy is the value from 0 to 1.
// The lesser the accuracy is, the more distant the point can be from the image but still be considered "on" it.
bool ImageEditScene::isOnImage(QPoint point, qreal accuracy) const {
	assert(accuracy > 0 && accuracy <= 1);

	if (accuracy == 1) {
		return m_imageRect.contains(point);
	}

	QPointF imageCenter = m_imageRect.center();
	qreal dist = utils::distance(point, imageCenter) * accuracy;

	return dist <= utils::length(imageCenter - m_imageRect.topLeft());
}

void ImageEditScene::onImageOpen(bool isNewImage, QSize imageSize) {
	m_isImageOpened = true;

	m_imageRect = QRectF(QPointF(0, 0), imageSize);
	setSceneRect(QRectF(0, 0, imageSize.width(), imageSize.height()));
	m_pView->fitInView(m_imageRect, Qt::KeepAspectRatio);

	if (isNewImage) {
		onImageChange();
	} else {
		m_isImageSaved = true;
		m_isImageSavedHint->setText("<font color='green'>Changes are saved</font>");
	}

	m_imageSizeHint->setText(
		"Image size ("
		+ QString::number(imageSize.width())
		+ " : "
		+ QString::number(imageSize.height())
		+ ")"
	);
}

void ImageEditScene::onImageSave() {
	m_isImageSaved = true;

	m_isImageSavedHint->setText("<font color='green'>Changes are saved</font>");
}

void ImageEditScene::onCursorMove(QPointF pos) {
	if (isOnImage(pos.toPoint())) {
		m_pixelUnderCursorHint->setText(
			"Pixel ("
			+ QString::number(long long(pos.x() - m_imageRect.x()))
			+ " : "
			+ QString::number(long long(pos.y() - m_imageRect.y()))
			+ ")"
		);
	} else {
		m_pixelUnderCursorHint->setText("Not on image");
	}
}

void ImageEditScene::onImageChange() {
	if (m_isImageSaved || m_isImageSavedHint->text().isEmpty()) {
		m_isImageSavedHint->setText("<b><font color='red'>Unsaved changes!</font></b>");
		m_isImageSaved = false;
	}

	update();
}

void ImageEditScene::onToolRadiusChanged(int radius) {
	m_toolRadius = radius;
}

void ImageEditScene::onToolPress(QPointF pos) {
	bool isUpdateRequired = false;

	pos -= m_imageRect.topLeft();
	switch (m_currentTool) {
		case ToolType::PEN:
			m_image.fillCircle(pos.toPoint(), m_toolRadius / 2, m_toolColor);
			isUpdateRequired = true;
			break;
		case ToolType::PICK_COLOR:
			m_toolColor = m_image.at(pos.x(), pos.y());
			m_colorLabel->setColor(m_toolColor);
			break;
	default: break;
	}

	if (isUpdateRequired) {
		onImageChange();
	}
}

void ImageEditScene::onToolMove(QPointF pos) {
	bool isUpdateRequired = false;

	pos -= m_imageRect.topLeft();
	QPointF oldPos = m_oldMousePos - m_imageRect.topLeft();

	switch (m_currentTool) {
		case ToolType::PEN:
			m_image.drawLine(oldPos.toPoint(), pos.toPoint(), m_toolRadius, m_toolColor);
			m_image.fillCircle(pos.toPoint(), m_toolRadius / 2, m_toolColor);
			isUpdateRequired = true;
			break;
	default: break;
	}

	if (isUpdateRequired) {
		onImageChange();
	}
}

void ImageEditScene::onToolRelease(QPointF pos) {
	bool isUpdateRequired = false;

	pos -= m_imageRect.topLeft();
	switch (m_currentTool) {

	default: break;
	}

	if (isUpdateRequired) {
		onImageChange();
	}
}

utils::Result<Void> ImageEditScene::updateImage() {
	auto qimageResult = m_image.toQImage();
	if (!qimageResult.isOk()) {
		return qimageResult.extractError();
	}

	
	addPixmap(QPixmap::fromImage(qimageResult.extract(), Qt::ImageConversionFlag::NoFormatConversion));
	update();

	return utils::Success();
}

void ImageEditScene::onColorChanged(Color newColor) {
	m_toolColor = newColor;
}

// Mouse wheel event, basically zooming
// TODO idea: add changing the tools with mouse wheel
void ImageEditScene::wheelEvent(QGraphicsSceneWheelEvent* event) {
	if (!m_isImageOpened) {
		event->ignore();
		return;
	}

	int delta = event->delta();
	qreal scaleFor = pow(1.001, delta); // for how much should we scale
	qreal newScale = m_scale * scaleFor; // total scale after the event, needed to estimate whether it surpasses the zooming limits

	if (newScale < MAX_ZOOM_IN) {
		scaleFor = MAX_ZOOM_IN / m_scale;
	} else if (newScale > MAX_ZOOM_OUT) {
		scaleFor = MAX_ZOOM_OUT / m_scale;
	}

	m_scale *= scaleFor;

	// Making sure the scale would center on the pointer and zooming
	QPointF pointerPos = event->scenePos();
	QRectF sceneBounds = m_pView->mapToScene(m_pView->viewport()->rect()).boundingRect();

	if (sceneBounds.contains(pointerPos)) {
		// Some magical math, once writen, never read
		// Calculates the shift of the view so as to have the pointer in the same position regarding the image
		QRectF zoomRect = sceneBounds.translated(-pointerPos);
		zoomRect.setRect(
			zoomRect.x() / scaleFor,
			zoomRect.y() / scaleFor,
			zoomRect.width() / scaleFor,
			zoomRect.height() / scaleFor
		);

		zoomRect.translate(pointerPos);
		m_pView->fitInView(zoomRect.normalized(), Qt::KeepAspectRatio);
	} else { // Pointer is out of the scene, so no need to center on it
		m_pView->scale(scaleFor, scaleFor);
	}

	update();
	event->accept();
}

// Mouse button pressed
void ImageEditScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (!m_isImageOpened) {
		event->ignore();
		return;
	}

	if (event->button() == Qt::LeftButton) { // LMB (left mouse button) pressed
		switch (m_currentTool) {
			case ToolType::NONE: break;
			case ToolType::MOVING:
				pushCursor(Qt::ClosedHandCursor);
				break;
		default:
			if (auto pos = event->scenePos(); isOnImage(pos.toPoint())) {
				onToolPress(pos);
			} break;
		}

		m_isLMBPressed = true;
		m_oldMousePos = event->scenePos();
	}
	
	event->accept();
}

// Mouse being moved
void ImageEditScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	if (!m_isImageOpened) {
		event->ignore();
		return;
	}

	onCursorMove(event->scenePos());

	// if the cursor is on image and the LMB is pressed
	if (m_isLMBPressed && (isOnImage(event->scenePos().toPoint()) || m_currentTool == ToolType::MOVING)) {
		switch (m_currentTool) {
			case ToolType::NONE: break;
			case ToolType::MOVING: {
				QPointF offset = event->scenePos() - m_oldMousePos;
				tryMoveImageBy(offset);
			} break;
			default:
				onToolMove(event->scenePos());
				break;
		}
	}

	m_oldMousePos = event->scenePos(); // So as to remove the leaps of the image

	event->accept();
}

// Mouse button released
void ImageEditScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	if (!m_isImageOpened) {
		event->ignore();
		return;
	}

	if (event->button() == Qt::LeftButton) { // LMB (left mouse button) released
		m_isLMBPressed = false;
		popCursor();

		if (isOnImage(event->scenePos().toPoint())) {
			onToolRelease(event->scenePos());
		}
	}

	event->accept();
}
