#pragma once
#include <cstdint>
#include <QString>

// Own internal color representation using RGBA
struct Color {
	// Channels of color
	enum Channel : uint8_t {
		A = 1,
		R = 2,
		G = 4,
		B = 8
	};

	// Mask for a channel
	enum ChannelMask : uint32_t {
		BLUE = 0xff,
		GREEN = 0xff00,
		RED = 0xff0000,
		ALPHA = 0xff000000
	};

	static Channel getChannel(bool red, bool green, bool blue, bool alpha);
	static ChannelMask getChannelMask(Channel channels);
	static Color getColorByChannels(Channel channels, uint8_t value);

	// Color implementation
	uint32_t _data; // Actually 0xAARRGGBB

	Color();
	Color(uint32_t data); // Initializing with a row data
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255); // Initializing with channels

	bool operator==(Color other) const;
	
	// Initializing some internal tables
	static void initColorModule();
	 
	// Instead of field access
	// Allows to make internal channels representation independent from the interface
	uint8_t& r();
	uint8_t& g();
	uint8_t& b();
	uint8_t& a();

	// Constant versions of component-access functions
	uint8_t r() const;
	uint8_t g() const;
	uint8_t b() const;
	uint8_t a() const;

	// Returns a brighter version of the color
	Color brighter() const;

	// Returns a darker version of the color
	Color darker() const;

	// Returns a color with RGB channels multiplied by -by-, but no more than 255
	Color multiplyBy(float by) const;

	// Overlaying the current color with another one
	// Works strange, better not touch it for now
	void mix(Color over);

	// Mixing 2 colors as if over is overlayed over under
	// If over has alpha = 255 it just replaces under
	static Color mix(Color over, Color under);

	// Sets the color to another one for masked channels
	void setColorWithMask(ChannelMask mask, Color color);

	QString toString();

	// Some color constants
	// White and black ones are pretty useful, others may be used for color-choosing later
	const static Color White;
	const static Color Black;
	const static Color Gray;
	const static Color Green;
	const static Color Blue;
	const static Color Red;
	const static Color Yellow;
	const static Color Pink;
	const static Color Brown;
	const static Color Orange;
	const static Color Cyan;
	const static Color Magenta;
	const static Color Silver;
	const static Color Lime;
	const static Color Purple;
	const static Color Navy;
	const static Color Olive;
};