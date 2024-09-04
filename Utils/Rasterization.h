#pragma once
#include <concepts>
#include <QPoint>
#include "MathUtils.h"

namespace utils {
	class SingleRasterizer;

	// Concept for SingleRasterizer's derived classes
	template<class T>
	concept SingleRasterizerConcept = std::derived_from<T, SingleRasterizer>;

	// Basic class for all rasterizers
	// Splits some shape into rows of X values
	// The X is guaranteed to be >= 0, but it can be out of image width, this should be handled outside
	class BasicRasterizer {
	protected:
		uint32_t m_y = 0; // Current row
		uint32_t m_endY = 0; // The last row

	public:
		virtual void nextRow() = 0;

		uint32_t getY() const;
		uint32_t getEndY() const;

		bool hasMore(); // Are there more rows in this shape
	};

	// For those shapes that have a single piece in each row
	// Splits some shape into a pair of left-most and right-most X for each row
	// Usage example:
	//		SingleRasterizer* some_r = ...;
	//		while (some_r->hasMore()) {
	//			drawStroke(some_r->leftX(), some_r->rightX());
	//			some_t->nextRow();
	//		}
	class SingleRasterizer : public BasicRasterizer {
	public:
		virtual uint32_t leftX() = 0; // Current row's left-most X
		virtual uint32_t rightX() = 0; // Current row's right-most X
		virtual uint32_t rowLength() = 0; // Current row's width
	};

	// Allows to get the pairs of the first and last points in a row for each row
	// For a line with width of 1 pixel
	class LineRasterizer final : public SingleRasterizer {
	private:
		double m_x; // Current X
		double m_xOffset; // dx/dy

	public:
		LineRasterizer(QPoint from, QPoint to, uint32_t width, uint32_t height);

		void nextRow() override;

		uint32_t leftX() override; // Current row's left-most X
		uint32_t rightX() override; // Current row's right-most X
		uint32_t rowLength() override; // Current row's width
	};

	class WideLineRasterizer;

	// Allows to get all the pairs of left-most and right-most pairs of pixel positions for every row
	// For a tetragon
	class TetragonRasterizer : public SingleRasterizer {
	private:
		double m_leftX;
		double m_rightX;

		double m_leftOffset;
		double m_rightOffset;

		QPoint m_B;
		QPoint m_C;
		QPoint m_D;

	protected:
		// Used in WideLineRasterizer
		TetragonRasterizer(QPoint from, QPoint to, float lineWidth);

		// For internal use, to avoid dublicating the code
		void initWith(QPoint A, QPoint B, QPoint C, QPoint D);

	public:
		// For a shape ABCD, A must be the top, B must be to the left from D
		TetragonRasterizer(QPoint A, QPoint B, QPoint C, QPoint D);

		void nextRow() override;

		uint32_t leftX() override; // Current row's left-most X
		uint32_t rightX() override; // Current row's right-most X
		uint32_t rowLength() override; // Current row's width
	};

	// Allows to get all the pairs of left-most and right-most pairs of pixel positions for every row
	// For a line with width more than 1 pixel
	class WideLineRasterizer final : public TetragonRasterizer {
	public:
		WideLineRasterizer(QPoint from, QPoint to, float lineWidth);
	};

	// Allows to get all the pairs of left-most and right-most pairs of pixel positions for every row
	// For a rectangle that was not rotated
	class RectRasterizer final : public SingleRasterizer {
	private:
		uint32_t m_leftX;
		uint32_t m_rightX;
		uint32_t m_rowLength;

	public:
		RectRasterizer(QPoint from, QPoint to);

		void nextRow() override;

		uint32_t leftX() override; // Current row's left-most X
		uint32_t rightX() override; // Current row's right-most X
		uint32_t rowLength() override; // Current row's width
	};

	// Allows to get all the pairs of left-most and right-most pairs of pixel positions for every row
	// For a circle
	class CircleRasterizer final : public SingleRasterizer {
	private:
		QPoint m_center;
		double m_radiusSqr;
		int m_xOffset;

	public:
		CircleRasterizer(QPoint center, float radius);

		void nextRow() override;

		uint32_t leftX() override; // Current row's left-most X
		uint32_t rightX() override; // Current row's right-most X
		uint32_t rowLength() override; // Current row's width
	};


	// Return result of DoubleRasterizer for each row
	struct DoubleRasterizationRow {
		// First pair
		uint32_t x1 = 0;
		uint32_t length1 = 0;
		
		// Second pair
		uint32_t x2 = 0;
		uint32_t length2 = 0;

		bool hasFirstPair = false;
		bool hasSecondPair = false;
	};

	// For those shapes that consist of a shape cut out of another
	// For example a ring is a circle with another circle cut out in the middle
	// Splits some shape into 1 or 2 pairs of left-most and right-most X for each row
	// Usage example:
	//		auto rast = ...;
	//		while (rast.hasMore()) {
	//			auto rastRow = rast.rasterize();
	//			if (rastRow.hasFirstPair) {
	//				drawXStroke(rastRow.x1, rast.getY(), rastRow.length1, color);
	//				if (rastRow.hasSecondPair) {
	//					drawXStroke(rastRow.x2, rast.getY(), rastRow.length2, color);
	//				}
	//			}
	// 
	//			rast.nextRow();
	//		}
	template<SingleRasterizerConcept InnerRasterizer, SingleRasterizerConcept OuterRasterizer>
	class DoubleRasterizer final : public BasicRasterizer {
	private:
		InnerRasterizer m_inner;
		OuterRasterizer m_outer;

	public:
		inline DoubleRasterizer(InnerRasterizer&& inner, OuterRasterizer&& outer)
			: m_inner(std::move(inner)), m_outer(std::move(outer)) {
			m_y = m_outer.getY();
			m_endY = m_outer.getEndY();

			// So that inner is on the same Y as outer or is skipped
			while (m_inner.hasMore() && m_inner.getY() < m_y) {
				m_inner.nextRow();
			}
		}

		// Current row's pair (or one) of strokes
		DoubleRasterizationRow rasterize() {
			DoubleRasterizationRow result;
			uint32_t outerLeft = m_outer.leftX();
			uint32_t outerLength = m_outer.rowLength();

			if (m_y != m_inner.getY() || m_y > m_inner.getEndY()) { // No inner shape for this row
				result.x1 = outerLeft;
				result.length1 = outerLength;
				result.hasFirstPair = true;
				result.hasSecondPair = false;

				return result;
			}

			uint32_t innerLeft = m_inner.leftX();
			uint32_t innerLength = m_inner.rowLength();

			// Inner shape covers the left side of the outer one
			if (innerLeft <= outerLeft && innerLeft + innerLength >= outerLeft) {
				// The outer is completely covered by inner
				if (innerLeft + innerLength >= outerLeft + outerLength) {
					result.hasFirstPair = false;
					result.hasSecondPair = false;

					return result;
				}

				result.x1 = innerLeft + innerLength;
				result.length1 = outerLeft + outerLength - result.x1;
				result.hasFirstPair = true;
				result.hasSecondPair = false;

				return result;
			}

			result.x1 = outerLeft;
			result.hasFirstPair = true;

			// Inner and outer intersect in the right
			if (outerLeft + outerLength > innerLeft) {
				result.length1 = innerLeft - result.x1;
				
				// There is a second stroke
				if (outerLeft + outerLength > innerLeft + innerLength) {
					result.x2 = innerLeft + innerLength;
					result.length2 = outerLeft + outerLength - result.x2;
					result.hasSecondPair = true;

					return result;
				}

				result.hasSecondPair = false;

				return result;
			}

			// Outer in not covered by inner at all
			result.length1 = outerLength;
			result.hasSecondPair = false;

			return result;
		}

		void nextRow() override {
			if (m_y == m_inner.getY() && m_inner.hasMore()) {
				m_inner.nextRow();
			}

			m_outer.nextRow();
			m_y++;
		}
	};

	// Creates a double rasterizer for a ring
	// Allows to get all the pairs of left-most and right-most pairs of pixel positions for every row
	DoubleRasterizer<CircleRasterizer, CircleRasterizer> RingRasterizer(QPoint center, float radius, float lineWidth);

	// Creates a double rasterizer for a hollow rect
	// Allows to get all the pairs of left-most and right-most pairs of pixel positions for every row
	DoubleRasterizer<RectRasterizer, RectRasterizer> HollowRectRasterizer(QPoint from, QPoint to, float lineWidth);
}