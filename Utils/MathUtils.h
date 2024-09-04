#pragma once
#include <QPoint>

namespace utils {
	// Bit masks for Cohen-Sutherland algorythm
	enum CohenSutherland : uint8_t {
		NONE = 0,
		LEFT = 1,
		RIGHT = 2,
		BOTTOM = 4,
		TOP = 8
	};

	// The length of a vector from (0, 0) to point
	qreal length(const QPointF& point);

	// Returns normilized vector vec
	QPointF normalize(QPointF vec);

	// Returns upper of 2 normals to the vector
	QPointF getUpperNormalToVec(QPointF vec);

	// Returns a point advanced from pos in the chosen direction for advanceFor
	// direction must be a normalized vector
	QPoint advanceInDirectionFor(QPoint pos, QPointF direction, float advanceFor);

	// The distance between 2 points
	qreal distance(const QPointF& a, const QPointF& b);

	// Returns for how much X changes if Y changes for +1
	double offsetXperY(QPoint from, QPoint to);

	// Clips the line so that it lies within rect(0, 0, width, height)
	// Returns false if the line is out of rect
	// On returning false, the passed arguments are not modified
	bool clipLine(QPoint& from, QPoint& to, uint32_t width, uint32_t height);

	// Returns true if the line lies completely out of rect (0, 0, width, height) and false otherwise
	bool isLineOutOfRect(QPoint from, QPoint to, uint32_t width, uint32_t height);

	// Returns the code of pos in rect (0, 0, width, height) according to Cohen-Sutherland algorythm
	uint8_t getCohenSutherlandCode(QPoint pos, uint32_t width, uint32_t height);

	auto max(auto x, auto y) {
		return x > y ? x : y;
	}

	auto min(auto x, auto y) {
		return x < y ? x : y;
	}

	auto clamp(auto x, auto lower, auto upper) {
		return x < lower ? lower : (x > upper ? upper : x);
	}
}