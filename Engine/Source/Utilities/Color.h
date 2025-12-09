#pragma once
#include "Macro.h"

#define TXT_BLACK "\x1B[38;5;232m"
#define TXT_DARK_GRAY "\x1B[38;5;237m"
#define TXT_GRAY "\x1B[38;5;244m"
#define TXT_LIGHT_GRAY "\x1B[38;5;249m"
#define TXT_WHITE "\x1B[38;5;255m"
#define TXT_DARK_RED "\x1B[38;5;124m"
#define TXT_RED "\x1B[38;5;196m"
#define TXT_DARK_ORANGE "\x1B[38;5;130m"
#define TXT_ORANGE "\x1B[38;5;208m"
#define TXT_DARK_YELLOW "\x1B[38;5;136m"
#define TXT_YELLOW "\x1B[38;5;226m"
#define TXT_LIME "\x1B[38;5;82m"
#define TXT_GREEN "\x1B[38;5;106m"
#define TXT_BLUE "\x1B[38;5;63m"
#define TXT_LIGHT_BLUE "\x1B[38;5;12m"
#define TXT_CYAN "\x1B[38;5;51m"
#define TXT_PINK "\x1B[38;5;219m"
#define TXT_MAGENTA "\x1B[38;5;199m"
#define TXT_PURPLE "\x1B[38;5;99m"
#define TXT_BROWN "\x1B[38;5;130m"

#define BG_BLACK "\x1B[48;5;232m"
#define BG_DARK_GRAY "\x1B[48;5;237m"
#define BG_GRAY "\x1B[48;5;244m"
#define BG_LIGHT_GRAY "\x1B[48;5;249m"
#define BG_WHITE "\x1B[48;5;255m"
#define BG_DARK_RED "\x1B[48;5;124m"
#define BG_RED "\x1B[48;5;196m"
#define BG_DARK_ORANGE "\x1B[48;5;130m"
#define BG_ORANGE "\x1B[48;5;208m"
#define BG_DARK_YELLOW "\x1B[48;5;136m"
#define BG_YELLOW "\x1B[48;5;226m"
#define BG_LIME "\x1B[48;5;82m"
#define BG_GREEN "\x1B[48;5;106m"
#define BG_BLUE "\x1B[48;5;63m"
#define BG_LIGHT_BLUE "\x1B[48;5;12m"
#define BG_CYAN "\x1B[48;5;51m"
#define BG_PINK "\x1B[48;5;219m"
#define BG_MAGENTA "\x1B[48;5;199m"
#define BG_PURPLE "\x1B[48;5;99m"
#define BG_BROWN "\x1B[48;5;130m"

#define COLOR_RESET "\033[0m"
#define UNDERLINE "\033[4m"
#define SWAP "\033[7m" // Swap background and foreground colors

#define BLINK_TEXT "\033[5m"	
#define BLINK_TEXT_OFF "\033[25m"	
#define ITALIC "\033[3m"
#define ITALIC_OFF "\033[23m"
#define CROSSED "\033[9m"
#define CROSSED_OFF "\033[0m"

#define BOLD "\x1B[1m"
#define RESET_BOLD "\x1B[0m"

	// x => Color code between 0 and 255
#define COLOR(x) string("\x1B[38;5;" + x) + "m"
#define BG_COLOR(x) "\x1B[48;5;"<<x<<"m"

#define CUSTOM_RGB(r,g,b)"\x1b[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m"
#define BG_RGB(r,g,b) "\x1b[48;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m"
#define TEXT_RGB(r,g,b) "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m"


namespace Krampus
{

	class Color
	{
	public:
		std::uint8_t r;
		std::uint8_t g;
		std::uint8_t b;
		std::uint8_t a;

		constexpr Color() noexcept;
		constexpr Color(const std::uint8_t& _r, const std::uint8_t& _g, const std::uint8_t& _b, const std::uint8_t& _a = 255) noexcept;
		constexpr explicit Color(const std::uint32_t& _color) noexcept;
		constexpr Color(const sf::Color& _color) noexcept;

		constexpr std::uint32_t ToInteger() const noexcept;
		std::string ToString(const bool& _textOnly) const noexcept;

		INLINE operator sf::Color() const
		{
			return sf::Color(r, g, b, a);
		}

		
		INLINE static constexpr Color Black() noexcept
		{
			return Color();
		}
		INLINE static constexpr Color White() noexcept 
		{ 
			return Color(255, 255, 255);
		};
		INLINE static constexpr Color Red() noexcept 
		{ 
			return Color(255, 0, 0);
		};
		INLINE static constexpr Color Green() noexcept 
		{ 
			return Color(0, 255, 0);
		};
		INLINE static constexpr Color Blue() noexcept 
		{ 
			return Color(0, 0, 255);
		};
		INLINE static constexpr Color Yellow() noexcept 
		{ 
			return Color(255, 255, 0);
		};
		INLINE static constexpr Color Magenta() noexcept 
		{ 
			return Color(255, 0, 255);
		};
		INLINE static constexpr Color Cyan() noexcept 
		{ 
			return Color(0, 255, 255);
		};
		INLINE static constexpr Color Transparent() noexcept 
		{ 
			return Color(0, 0, 0, 0);
		};
	};


	struct Gradient
	{
		Color gradA;
		Color gradB;

		Gradient() = default;
		Gradient(const Color& _a, const Color& _b);

		std::string GradientString(const std::string& _text, const bool& _textOnly = true) const;
		Color ClampGradient(const int& _index, const int& _maxDisplayChar) const;
	};

}