#pragma once
#include <QList>

enum class BasicType : uint8_t {
	NUMBER = 0,
	POINT,
	COLOR
};

typedef QList<BasicType> Type;

inline bool operator==(const Type& a, const Type& b) {
	if (a.size() != b.size()) {
		return false;
	}

	for (size_t i = 0; i < a.size(); i++) {
		if (a[i] != b[i]) {
			return false;
		}
	}

	return true;
}

inline bool operator==(const Type& a, BasicType bb) {
	if (a.size() != 1 || a[0] != bb) {
		return false;
	}

	return true;
}

inline uint32_t getTypeSize(BasicType type) {
	switch (type) {
		case BasicType::NUMBER: return 1;
		case BasicType::POINT: return 2;
		case BasicType::COLOR: return 4;
	default: return 0;
	}
}