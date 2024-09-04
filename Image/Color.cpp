#include "Color.h"
#include "../Utils/MathUtils.h"

const Color Color::White = Color(255, 255, 255);
const Color Color::Black = Color(0, 0, 0);
const Color Color::Gray = Color(128, 128, 128);
const Color Color::Green = Color(0, 128, 0);
const Color Color::Blue = Color(0, 0, 255);
const Color Color::Red = Color(255, 0, 0);
const Color Color::Yellow = Color(255, 255, 0);
const Color Color::Pink = Color(255, 192, 203);
const Color Color::Brown = Color(165, 42, 42);
const Color Color::Orange = Color(255, 165, 0);
const Color Color::Cyan = Color(0, 255, 255);
const Color Color::Magenta = Color(255, 0, 255);
const Color Color::Silver = Color(192, 192, 192);
const Color Color::Lime = Color(0, 255, 0);
const Color Color::Purple = Color(128, 0, 128);
const Color Color::Navy = Color(0, 0, 128);
const Color Color::Olive = Color(128, 128, 0);

// Some constants to accelerate color mixing
uint8_t g_multblTable[256][256];

Color::Channel Color::getChannel(bool red, bool green, bool blue, bool alpha) {
    uint32_t result = 0;
    if (red) {
        result |= uint32_t(R);
    } if (green) {
        result |= uint32_t(G);
    } if (blue) {
        result |= uint32_t(B);
    } if (alpha) {
        result |= uint32_t(A);
    }

    return Channel(result);
}

Color::ChannelMask Color::getChannelMask(Channel channels) {
    uint32_t result = 0;
    if (channels & A) {
        result |= ChannelMask::ALPHA;
    } if (channels & R) {
        result |= ChannelMask::RED;
    } if (channels & G) {
        result |= ChannelMask::GREEN;
    } if (channels & B) {
        result |= ChannelMask::BLUE;
    }

    return ChannelMask(result);
}

Color Color::getColorByChannels(Channel channels, uint8_t value) {
    return Color(
        channels & R ? value : 0,
        channels & G ? value : 0,
        channels & B ? value : 0,
        channels & A ? value : 0
    );
}

Color::Color()
    : _data(0) {

}

Color::Color(uint32_t data)
    : _data(data) {

}

Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    this->r() = r;
    this->g() = g;
    this->b() = b;
    this->a() = a;
}

bool Color::operator==(Color other) const {
    return _data == other._data;
}

void Color::initColorModule() {
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 256; ++j) {
            int k = (i * j) / 255;
            g_multblTable[i][j] = (uint8_t)k;
        }
    }
}

uint8_t& Color::r() {
    return *(((uint8_t*)&_data) + 2);
}

uint8_t& Color::g() {
    return *(((uint8_t*)&_data) + 1);
}

uint8_t& Color::b() {
    return *(((uint8_t*)&_data) + 0);
}

uint8_t& Color::a() {
    return *(((uint8_t*)&_data) + 3);
}

uint8_t Color::r() const {
    return *(((uint8_t*)&_data) + 2);
}

uint8_t Color::g() const {
    return *(((uint8_t*)&_data) + 1);
}

uint8_t Color::b() const {
    return *(((uint8_t*)&_data) + 0);
}

uint8_t Color::a() const {
    return *(((uint8_t*)&_data) + 3);
}

Color Color::brighter() const {
    return multiplyBy(1.25f);
}

Color Color::darker() const {
    return multiplyBy(0.8f);
}

Color Color::multiplyBy(float by) const {
    Color res(_data);
    res.r() = utils::min(255, uint32_t(float(res.r()) * by));
    res.g() = utils::min(255, uint32_t(float(res.g()) * by));
    res.b() = utils::min(255, uint32_t(float(res.b()) * by));

    return res;
}

void Color::mix(Color over) {
    _data = mix(over, *this)._data;
}

Color Color::mix(Color over, Color under) {
    uint32_t ao = over.a(), au = under.a();
    if (ao == 255 || au == 0) {
        return over;
    }

    if (ao == 0) {
        return under;
    }

    // Quick algorythm
    // Just what the heck it does?
    return under._data + ((g_multblTable[ao][over._data & 0xff]) |
        (((uint32_t)g_multblTable[ao][(over._data >> 8) & 0xff]) << 8) |
        (((uint32_t)g_multblTable[ao][(over._data >> 16) & 0xff]) << 16) |
        (((uint32_t)g_multblTable[ao][(over._data >> 24) & 0xff]) << 24));
}

void Color::setColorWithMask(ChannelMask mask, Color color) {
    _data = (_data & ~mask) | (color._data & mask);
}

QString Color::toString() {
    return "(" + QString::number(r())
        + ", " + QString::number(g())
        + ", " + QString::number(b())
        + ", " + QString::number(a()) + ")";
}
