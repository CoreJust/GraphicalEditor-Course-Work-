#pragma once
#include <cstdint>
#include <QString>
#include <QImage>
#include <QRect>
#include "Color.h"
#include "../Utils/Result.h"
#include "../Utils/Buffer.h"
#include "../Utils/Rasterization.h"


// Maximal number of pixels in an image (2^31 - 1)
constexpr uint32_t IMAGE_SIZE_MAX = 2147483647;


// Our own class to handle images: load from a file, store to a file, modify, convert to QPixMap
// Color representation is RGBA, 8 bit depth
// Works with .png files only, but jpg and bmp support can be added anytime
class Image final {
public:
	enum class Extension : uint8_t {
		NONE = 0, // For new images that are not loaded from any file

		PNG = 1,

		// Not supported yet, more like a reminder as of now
		JPG = 2,
		BMP = 3
	};

	struct PngFileData { // Some information peculiar to png files
		int interlace_method = -1;	// The method how the data is accomodated within the file
									// E.g. an image can be loaded from up to down or with a gradual detalization (when it is blurred originally)
		int compression_method = -1; // The way the png is compressed. Nothing to add
		int filter_method = -1; // Some stuff used on the bytes of the image, not actually important for the Image class

		QString getInterlaceMethodAsString() const;
		QString getCompressionMethodAsString() const;
		QString getFilterMethodAsString() const;
	};

private:
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	QString m_fileName = "";
	utils::SizedBuffer<uint32_t> m_data;

	///   Additional data on the original image file   ///
	// The number of bits for a single color component: 1, 2, 4, 8, or 16
	// The current image class works with 8-bit depth only
	int m_bitDepth = -1; // Non-initialized

	// The color scheme (used color channels, e.g. rgb, rgba, gray, etc)
	// Uses the libpng values, but for adding more file formats it must be rewritten
	// The current image class works with RGBA only
	int m_colorType = -1; // Non-initialized

	// The extension of the file currently opened
	Extension m_extension = Extension::NONE;

	// Extension-specific data
	// Since there is png only now, the union part looks pretty useless...
	union {
		PngFileData m_pngFileData;
		uint8_t __fileData[sizeof(PngFileData)] = { 0 };
	};
	

public:
	Image(); // Actually does nothing

	// Copies the other image
	// The current image must already be created and have the same size as the other
	void copyFrom(Image& other);

	// Allocates an empty image with width and height
	utils::Result<Void> allocateImage(uint32_t width, uint32_t height);

	utils::Result<Void> open(const QString& fileName); // Loads an image from file, automatically recognizes the file extension
	utils::Result<Void> store(const QString& fileName); // Stores an image to file, automatically recognizes the file extension
	
	// Creates a new image (width x height) filled with fillColor
	utils::Result<Void> newImage(uint32_t width, uint32_t height, Color fillColor);

	// Required image functions

	// Changes all the pixel's channel to some value 
	void channelFilter(Color::Channel channel, uint8_t value);

	// Splits the image into NxM subimages and saves each one to a separate file (-basicName-_X_Y.png)
	void splitImageIntoSubimages(uint32_t N, uint32_t M, const QString& basicName);

	// Draws a grid splitting image into NxM parts
	void splitImageWithGrid(uint32_t N, uint32_t M, float lineWidth, Color color);

	// Draws a circle and fills it with a color if needsFill
	void drawComplexCircle(QPoint center, float radius, float lineWidth, Color lineColor, Color fillColor, bool needsFill);

	// Draws a square and its diagonals and fills it with a color if needsFill
	void drawSquareWithDiagonals(QPoint from, uint32_t side, float lineWidth, Color lineColor, Color fillColor, bool needsFill);

	// Wraps all the white areas on the image with a line of lineColor
	void emphasizeWhiteAreas(Color lineColor);

	// Drawing funсtions

	// General line-drawing function
	// Draws a straight line from one point to another
	void drawLine(QPoint from, QPoint to, float width, Color color);

	// Draws a straight line from one point for yOffset pixels along Y axis
	void drawVerticalLine(QPoint from, uint32_t yOffset, float width, Color color);

	// Draws a straight line from one point for xOffset pixels along X axis
	void drawHorizontalLine(QPoint from, uint32_t xOffset, float height, Color color);

	// Draws a hollow rectangle with lineWidth
	void drawRect(QPoint from, uint32_t width, uint32_t height, float lineWidth, Color color);

	// Draws a hollow circle at center with a radius, lineWidth and color
	void drawCircle(QPointF center, float radius, float lineWidth, Color color);

	// Draws a filled rectangle
	void fillRect(QPoint from, uint32_t width, uint32_t height, Color color);

	// Draws a filled circle at center with a radius and color
	void fillCircle(QPointF center, float radius, Color color);

	// Draws a horizontal line along a single row
	// In case of incorrect y or x value just returns
	void drawXStroke(uint32_t fromX, uint32_t fromY, uint32_t length, Color color);

	// Quickly fills a part of row's pixels with color
	// Does not use color blending, so must not be used with color with alpha less than 255
	void drawQuickXStroke(uint32_t fromX, uint32_t fromY, uint32_t length, Color color);

	// Converts the contained data to Qt Image, or returns Failure if there is no image loaded
	utils::Result<QImage> toQImage() const;

	// Returns true if one of the surrounding pixels is of the stated color
	bool hasColorInNeighborhood(uint32_t x, uint32_t y, Color color);

	Color& at(uint32_t x, uint32_t y); // Slower, but safer
	Color at(uint32_t x, uint32_t y) const; // A constant variation of the at() function

	Color& atFast(uint32_t x, uint32_t y); // Faster, but less safe
	Color atFast(uint32_t x, uint32_t y) const; // A constant variation of the atFast() function
	
	QRect getRect() const; // The image rect (0, 0, width, height)
	QSize getSize() const; // The image size (width, height)
	uint32_t getWidth() const;
	uint32_t getHeight() const;
	const QString& getFileName() const;
	QList<QPair<QString, QString>> getFullImageInformation() const; // Image information for displaying somewhere, "value_name": "value"

private:
	utils::Result<Void> openPng(const QString& fileName); // Loads an image from a png file
	utils::Result<Void> storePng(const QString& fileName); // Stores an image to a png file

	// Returns an image filled with the stated part of this image
	Image& fillWithSubimage(Image& result, QPoint from, uint32_t width, uint32_t height);

	// Draws a line with WideLineRasterizer
	void drawWideLine(QPoint from, QPoint to, float width, Color color);

	// Draws a thin line with LineRasterizer
	void drawThinLine(QPoint from, QPoint to, Color color);

	// Draws a shape dictated by the Rasterizer
	template<utils::SingleRasterizerConcept Raster>
	inline void drawRasterized(Raster rast, Color color) {
		while (rast.hasMore()) {
			assert(rast.rowLength() < INT32_MAX);
			drawXStroke(rast.leftX(), rast.getY(), rast.rowLength(), color);
			rast.nextRow();
		}
	}

	// Draws a shape dictated by the Rasterizer
	template<utils::SingleRasterizerConcept Inner, utils::SingleRasterizerConcept Outer>
	inline void drawRasterized(utils::DoubleRasterizer<Inner, Outer> rast, Color color) {
		while (rast.hasMore()) {
			auto rastRow = rast.rasterize();
			if (rastRow.hasFirstPair) {
				assert(rastRow.length1 < INT32_MAX);
				drawXStroke(rastRow.x1, rast.getY(), rastRow.length1, color);

				if (rastRow.hasSecondPair) {
					assert(rastRow.length2 < INT32_MAX);
					drawXStroke(rastRow.x2, rast.getY(), rastRow.length2, color);
				}
			}

			rast.nextRow();
		}
	}

	QString getColorTypeAsString() const; // Returns the current color type as a string
	QString getExtensionAsString() const; // Returns the current file's extension as a string
};