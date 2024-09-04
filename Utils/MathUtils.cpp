#include "MathUtils.h"

qreal utils::length(const QPointF& point) {
    return sqrt(point.x() * point.x() + point.y() * point.y());
}

QPointF utils::normalize(QPointF vec) {
    return vec / length(vec);
}

QPointF utils::getUpperNormalToVec(QPointF vec) {
    vec = normalize(vec);
    if (vec.x() <= 0) {
        return QPointF(-vec.y(), vec.x());
    } else {
        return QPointF(vec.y(), -vec.x());
    }
}

QPoint utils::advanceInDirectionFor(QPoint pos, QPointF direction, float advanceFor) {
    direction *= advanceFor;
    return QPoint(int(pos.x() + direction.x() + 0.5), int(pos.y() + direction.y() + 0.5));
}

qreal utils::distance(const QPointF& a, const QPointF& b) {
    return length(a - b);
}

double utils::offsetXperY(QPoint from, QPoint to) {
    int modifier = to.y() < from.y() ? -1 : 1;
    return double(to.x() - from.x()) / (to.y() - from.y() + modifier);
}

bool utils::clipLine(QPoint& from, QPoint& to, uint32_t width, uint32_t height) {
    uint8_t cs1 = getCohenSutherlandCode(from, width, height);
    uint8_t cs2 = getCohenSutherlandCode(to, width, height);

    // Both ends are on the same side from the rect and the line is out of the rect
    if (cs1 & cs2) {
        return false;
    }

    // Both ends are within the rect, nothing to clip
    if (cs1 == 0 && cs2 == 0) {
        return true;
    }

    double fromX = from.x();
    double fromY = from.y();
    double toX = to.x();
    double toY = to.y();

    uint8_t cs_chosen = 0;
    double* chosenX = nullptr;
    double* chosenY = nullptr;

    while (cs1 | cs2) {
        if (cs1 & cs2) { // Line is out of the rect
            return false;
        }

        // Choosing the end to clip
        if (cs1) {
            cs_chosen = cs1;
            chosenX = &fromX;
            chosenY = &fromY;
        } else {
            cs_chosen = cs2;
            chosenX = &toX;
            chosenY = &toY;
        }

        // Clipping the chosen end
        if (cs_chosen & CohenSutherland::LEFT) {
            *chosenY += (fromY - toY) * (-*chosenX) / (fromX - toX);
            chosenX = 0;
        } else if (cs_chosen & CohenSutherland::RIGHT) {
            *chosenY += (fromY - toY) * (width - 1 - *chosenX) / (fromX - toX);
            *chosenX = width - 1;
        }

        else if (cs_chosen & CohenSutherland::BOTTOM) {
            *chosenX += (fromX - toX) * (-*chosenY) / (fromY - toY);
            *chosenY = 0;
        } else if (cs_chosen & CohenSutherland::TOP) {
            *chosenX += (fromX - toX) * (height - 1 - *chosenY) / (fromY - toY);
            *chosenY = height - 1;
        }

        // Update the codes
        if (cs_chosen == cs1) {
            cs1 = getCohenSutherlandCode(QPoint(fromX, fromY), width, height);
        } else { // The second one was modified
            cs2 = getCohenSutherlandCode(QPoint(toX, toY), width, height);
        }
    }

    from.setX(fromX + 0.5);
    from.setY(fromY + 0.5);
    to.setX(toX + 0.5);
    to.setY(toY + 0.5);

    return true;
}

bool utils::isLineOutOfRect(QPoint from, QPoint to, uint32_t width, uint32_t height) {
    return !clipLine(from, to, width, height);
}

uint8_t utils::getCohenSutherlandCode(QPoint pos, uint32_t width, uint32_t height) {
    return (pos.x() < 0 ? CohenSutherland::LEFT : 0) |
        (pos.x() >= width ? CohenSutherland::RIGHT : 0) |
        (pos.y() < 0 ? CohenSutherland::BOTTOM : 0) |
        (pos.y() >= height ? CohenSutherland::TOP : 0);
}
