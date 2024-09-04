#pragma once
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include "ColorLabel.h"
#include "../Utils/Result.h"
#include "../Image/Image.h"
#include "ToolType.h"

// The class responsible for operating of the image edit area (the image itself)
// Acts as a mediator between higher-level interface and the image
class ImageEditScene final : public QGraphicsScene {
	Q_OBJECT

private:
	QGraphicsView* m_pView = nullptr;
	Image m_image;

	bool m_isImageSaved = true;

	QList<QCursor> m_cursorsHistory; // The views of the cursor
	QPointF m_oldMousePos;
	bool m_isLMBPressed = false; // whether the left mouse button is pressed
	bool m_isImageOpened = false;
	QRectF m_imageRect;
	qreal m_scale = 1; // zoom

	ToolType m_currentTool = ToolType::MOVING;

	// Tools settings
	Color m_toolColor = Color::Black;

	float m_toolRadius = 10.f;
	
	
	// The hints at the bottom of the window from the parenting MainWindow
	QLabel* m_isImageSavedHint = nullptr;
	QLabel* m_pixelUnderCursorHint = nullptr;
	QLabel* m_imageSizeHint = nullptr;

	// Current color displayed
	ColorLabel* m_colorLabel = nullptr;

public:
	ImageEditScene(QObject* parent, QLabel* isSavedHint, QLabel* pixelHint, QLabel* sizeHint, ColorLabel* colorLabel);

	void setParentView(QGraphicsView* pView);

public:
	void clearScene();
	utils::Result<Void> newImage(QSize size, Color fillColor);
	utils::Result<Void> openImage(const QString& fileName);
	utils::Result<Void> saveImage(const QString& fileName);

	// Required features
	// Transmitted to the image

	// Changes all the pixel's channel to some value
	void channelFilter(bool red, bool green, bool blue, bool alpha, uint8_t value);

	// Splits the image into NxM subimages and saves each one to a separate file (-path-/-basicName-_X_Y.png)
	void splitImageIntoSubimages(uint32_t N, uint32_t M, const QString& basicPath);

	// Draws a grid splitting image into NxM parts
	void splitImageWithGrid(uint32_t N, uint32_t M, float lineWidth, Color color);

	// Draws a circle and fills it with a color if needsFill
	void drawComplexCircle(QPoint center, float radius, float lineWidth, Color lineColor, Color fillColor, bool needsFill);

	// Draws a square and its diagonals and fills it with a color if needsFill
	void drawSquareWithDiagonals(QPoint from, uint32_t side, float lineWidth, Color lineColor, Color fillColor, bool needsFill);

	// Wraps all the white areas on the image with a line of lineColor
	void emphasizeWhiteAreas(Color lineColor);

	
	// Changes the current tool
	void setToolType(ToolType toolType);

	// Moves the image within the scene
	void moveImageBy(QPointF offset);
	void tryMoveImageBy(QPointF offset);

	// Used to regulate cursor changes
	void pushCursor(const QCursor& cursor);
	void popCursor();

	Image& getImage();
	QRectF getImageRect(QSize imageSize);
	bool isOnImage(QPoint pointOnScene, qreal accuracy = 1) const;

	const QString& getImageFileName() const;
	bool hasUnsavedImage() const;
	bool isImageOpened() const;

private: // own triggers
	void onImageOpen(bool isNewImage, QSize imageSize); // Called whenever a new image is created or a file is opened
	void onImageSave(); // Called whenever an image is saved
	void onCursorMove(QPointF pos); // Called whenever the cursor is moved
	void onImageChange(); // Called whenever the image is changes (or if a new image is created)

	void onToolPress(QPointF pos); // Called whenever LMB is pressed with a drawing tool chosen
	void onToolMove(QPointF pos); // Called whenever mouse is moved with a drawing tool chosen while LMB is pressed
	void onToolRelease(QPointF pos); // Called whenever LMB is released with a drawing tool chosen

	utils::Result<Void> updateImage(); // Called to update viewed image after it was changed/opened

public slots:
	void onColorChanged(Color newColor);
	void onToolRadiusChanged(int radius);

public: // events
	void wheelEvent(QGraphicsSceneWheelEvent* event) override;

	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
};