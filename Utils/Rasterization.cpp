#include "Rasterization.h"

using namespace utils;

uint32_t utils::BasicRasterizer::getY() const {
	return m_y;
}

uint32_t utils::BasicRasterizer::getEndY() const {
	return m_endY;
}

bool utils::BasicRasterizer::hasMore() {
	return m_y <= m_endY;
}

LineRasterizer::LineRasterizer(QPoint from, QPoint to, uint32_t width, uint32_t height) {
	if (from.y() > to.y()) {
		std::swap(from, to);
	}

	if (!clipLine(from, to, width, height)) { // The line is out of rect, so nothing to rasterize
		m_y = m_endY = 0; // So that hasMore() would return false right away
		m_x = 0;
		m_xOffset = 0;
		return;
	}

	m_y = from.y();
	m_x = from.x();
	m_endY = to.y();
	m_xOffset = offsetXperY(from, to);
}

void LineRasterizer::nextRow() {
	m_x += m_xOffset;
	m_y++;
	m_x = max(m_x, 0);
}

uint32_t LineRasterizer::leftX() {
	return uint32_t(m_xOffset < 0 ? m_x + m_xOffset : m_x);
}

uint32_t LineRasterizer::rightX() {
	return uint32_t(m_xOffset > 0 ? m_x + m_xOffset : m_x);
}

uint32_t utils::LineRasterizer::rowLength() {
	return uint32_t(m_xOffset) + 1;
}

utils::TetragonRasterizer::TetragonRasterizer(QPoint from, QPoint to, float lineWidth) {
	assert(lineWidth > 0);

	QPointF offset = getUpperNormalToVec(to - from);
	QPoint A = advanceInDirectionFor(from, offset, lineWidth / 2);
	QPoint B = advanceInDirectionFor(from, -offset, lineWidth / 2);
	QPoint C = advanceInDirectionFor(to, -offset, lineWidth / 2);
	QPoint D = advanceInDirectionFor(to, offset, lineWidth / 2);

	if (offset.x() < 0) { // To keep the order of vertices
		std::swap(B, D);
	}

	assert(A.y() <= B.y() && A.y() <= C.y() && A.y() <= D.y());

	initWith(A, B, C, D);
}

void utils::TetragonRasterizer::initWith(QPoint A, QPoint B, QPoint C, QPoint D) {
	if (A.y() == B.y()) {
		std::swap(B, D); // To simplify
	}

	if (A.y() == D.y()) { // So as not to fail
		m_B = { 0, 0 };
		m_C = { 0, 0 };
		m_D = { 0, 0 };

		m_y = A.y();
		m_endY = C.y();

		m_leftX = min(A.x(), D.x());
		m_rightX = max(A.x(), D.x());
		m_leftOffset = m_rightOffset = offsetXperY(A, B);
	} else {
		m_B = B;
		m_C = C;
		m_D = D;

		m_y = A.y();
		m_endY = max(B.y(), max(C.y(), D.y()));

		m_leftX = m_rightX = A.x();
		m_leftOffset = offsetXperY(A, B);
		m_rightOffset = offsetXperY(A, D);
	}
}

utils::TetragonRasterizer::TetragonRasterizer(QPoint A, QPoint B, QPoint C, QPoint D) {
	initWith(A, B, C, D);
}

void utils::TetragonRasterizer::nextRow() {
	// Changing the moving direction on reaching a vertex
	if (m_y == m_B.y()) {
		m_leftOffset = offsetXperY(m_B, m_C);
		m_leftX = m_B.x();
	} if (m_y == m_D.y()) {
		m_rightOffset = offsetXperY(m_D, m_C);
		m_rightX = m_D.x();
	} else if (m_y == m_C.y()) {
		if (m_C.y() < m_B.y()) {
			m_rightOffset = offsetXperY(m_C, m_B);
			m_rightX = m_C.x();
		} else {
			m_leftOffset = offsetXperY(m_C, m_D);
			m_leftX = m_C.x();
		}
	}

	m_y++;
	m_leftX += m_leftOffset;
	m_rightX += m_rightOffset;
}

uint32_t utils::TetragonRasterizer::leftX() {
	return max(0, int(m_leftOffset < 0 ? m_leftX + m_leftOffset : m_leftX));
}

uint32_t utils::TetragonRasterizer::rightX() {
	return max(0, int(m_rightOffset > 0 ? m_rightX + m_rightOffset : m_rightX));
}

uint32_t utils::TetragonRasterizer::rowLength() {
	uint32_t right = rightX();
	if (right == 0) {
		return 0;
	}

	return right - leftX() + 1;
}

utils::WideLineRasterizer::WideLineRasterizer(QPoint from, QPoint to, float lineWidth) 
	: TetragonRasterizer(from, to, lineWidth) {
	
}

utils::RectRasterizer::RectRasterizer(QPoint from, QPoint to) {
	// Making sure the order is correct
	if (from.y() > to.y()) {
		std::swap(from, to);
	}

	m_y = from.y();
	m_endY = to.y();
	m_leftX = max(0, min(from.x(), to.x()));
	m_rightX = max(0, max(from.x(), to.x()));
	m_rowLength = m_rightX == 0 ? 0 : m_rightX - m_leftX + 1;
}

void utils::RectRasterizer::nextRow() {
	m_y++;
}

uint32_t utils::RectRasterizer::leftX() {
	return m_leftX;
}

uint32_t utils::RectRasterizer::rightX() {
	return m_rightX;
}

uint32_t utils::RectRasterizer::rowLength() {
	return m_rowLength;
}

utils::CircleRasterizer::CircleRasterizer(QPoint center, float radius)
	: m_center(center), m_radiusSqr((radius + 0.5) * (radius + 0.5)) {
	radius += 0.5;
	m_y = utils::max(0, int(ceil(center.y() - radius)));
	m_endY = int(center.y() + radius);

	float yDistSqr = (m_y - m_center.y()) * (m_y - m_center.y());
	m_xOffset = int(sqrt(m_radiusSqr - yDistSqr));
}

void utils::CircleRasterizer::nextRow() {
	m_y++;

	// Updating internal data
	float yDistSqr = (m_y - m_center.y()) * (m_y - m_center.y());
	m_xOffset = int(sqrt(m_radiusSqr - yDistSqr));
}

uint32_t utils::CircleRasterizer::leftX() {
	return utils::max(0, m_center.x() - m_xOffset);
}

uint32_t utils::CircleRasterizer::rightX() {
	return utils::max(0, m_center.x() + m_xOffset);
}

uint32_t utils::CircleRasterizer::rowLength() {
	return rightX() - leftX() + 1;
}

utils::DoubleRasterizer<CircleRasterizer, CircleRasterizer> utils::RingRasterizer(
	QPoint center,
	float radius,
	float lineWidth
) {
	float innerRadius = radius - lineWidth / 2;
	float outerRadius = radius + lineWidth / 2;

	return DoubleRasterizer<CircleRasterizer, CircleRasterizer>(
		CircleRasterizer(center, innerRadius), 
		CircleRasterizer(center, outerRadius)
	);
}

utils::DoubleRasterizer<RectRasterizer, RectRasterizer> utils::HollowRectRasterizer(
	QPoint from,
	QPoint to,
	float lineWidth
) {
	lineWidth /= 2;
	QPoint innerFrom(from.x() + lineWidth, from.y() + lineWidth);
	QPoint innerTo(to.x() - lineWidth, to.y() - lineWidth);

	QPoint outerFrom(from.x() - lineWidth, from.y() - lineWidth);
	QPoint outerTo(to.x() + lineWidth, to.y() + lineWidth);

	return DoubleRasterizer<RectRasterizer, RectRasterizer>(
		RectRasterizer(innerFrom, innerTo),
		RectRasterizer(outerFrom, outerTo)
	);
}
