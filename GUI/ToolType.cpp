#include "ToolType.h"

bool hasWidthAttribute(ToolType type) {
	switch (type) {
		case ToolType::NONE: return false;
		case ToolType::MOVING: return false;
		case ToolType::SQUARE_AREA: return false;
		case ToolType::CIRCLE_AREA: return false;
		case ToolType::CUSTOM_AREA: return false;
		case ToolType::AUTO_AREA: return false;
		case ToolType::PEN: return true;
		case ToolType::SPRAY: return true;
		case ToolType::ERASER: return true;
		case ToolType::PICK_COLOR: return false;
		case ToolType::FILL: return false;
		case ToolType::CROP: return false;
		case ToolType::RESIZE: return false;
		case ToolType::TEXT: return false;
	default: return false;
	}
}
