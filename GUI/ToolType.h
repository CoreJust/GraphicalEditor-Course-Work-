#pragma once
#include <cstdint>

// Currently implemented: moving, pen, color-pick
enum class ToolType : uint8_t {
	NONE = 0, // Added just in case, used nowhere as of now

	MOVING, // Moving the image

	SQUARE_AREA, // Selecting a square area to do something (whatever inside/outside area would not be changed)
	CIRCLE_AREA, // Area with a circle as its bounds
	CUSTOM_AREA, // Area with a custom closed line as its bounds
	AUTO_AREA, // Selecting an area with the same color

	PEN, // Just a pen
	SPRAY, // A pen with an effect of spraying
	ERASER, // Just an eraser
	PICK_COLOR, // Copies the color of a chosen pixel

	FILL, // Filling in the area with the same color
	CROP, // Cropping the image
	RESIZE, // Resizing the image
	TEXT, // Allows to choose where to insert some text
};

bool hasWidthAttribute(ToolType type);