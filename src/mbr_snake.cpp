#include <cstdint>
#include <iterator>
#include <algorithm>
#include <type_traits>
#include <cstring>

// Don't use macros kids
// Only doing this because I edit the source on Windows, and GCC-specific things break intellisense
#if defined(__GNUG__) && !defined(__clang__)
	#define _ASM_VOLATILE_(...) asm volatile(__VA_ARGS__)
	#define _ASM_VOLATILE_GOTO_(...) asm volatile goto(__VA_ARGS__)
#else
	#define _ASM_VOLATILE_(...)
	#define _ASM_VOLATILE_GOTO_(...)
#endif

// https://en.cppreference.com/w/cpp/numeric/bit_cast
template <typename To, typename From>
inline typename std::enable_if_t<(sizeof(To) == sizeof(From)) && std::is_trivially_copyable_v<From> && std::is_trivial_v<To>, To>
bit_cast(const From& src) noexcept
{
	To dst;
	std::memcpy(&dst, &src, sizeof(To));
	return dst;
}

template <typename T>
struct vec2
{
	using value_type = T;
	T x{};
	T y{};
	inline constexpr bool operator==(const vec2 &rhs) const
	{
		return (x == rhs.x) && (y == rhs.y);
	}
};

using u16Vec2 = vec2<uint16_t>;

class x86
{
	struct rgbColor
	{
		uint8_t r{};
		uint8_t g{};
		uint8_t b{};
	};

	template <uint8_t Red, uint8_t Green, uint8_t Blue>
	static constexpr bool colorComp(const rgbColor &a, const rgbColor &b)
	{
		return (((a.r - Red) * (a.r - Red) + (a.g - Green) * (a.g - Green) + (a.b - Blue) * (a.b - Blue)) <
				((b.r - Red) * (b.r - Red) + (b.g - Green) * (b.g - Green) + (b.b - Blue) * (b.b - Blue)));
	}

public:

	x86() = delete;

	// Only using the low word of the tick count for this project
	// Using all 32 bits increases the binary size a lot
	using ticks_t = uint16_t;

	// Hard coded for 16-color video modes such as 0x0d
	template <uint8_t Red, uint8_t Green, uint8_t Blue>
	static constexpr uint8_t rgb()
	{
		const rgbColor colorTable[] =
		{
			rgbColor{0, 0, 0},
			rgbColor{0, 0, 85},
			rgbColor{0, 85, 0},
			rgbColor{0, 85, 85},
			rgbColor{85, 0, 0},
			rgbColor{85, 0, 85},
			rgbColor{85, 85, 0},
			rgbColor{170, 170, 170},
			rgbColor{85, 85, 85},
			rgbColor{0, 0, 170},
			rgbColor{0, 170, 0},
			rgbColor{0, 170, 170},
			rgbColor{170, 0, 0},
			rgbColor{170, 0, 170},
			rgbColor{170, 170, 85},
			rgbColor{255, 255, 255}
		};
		
		return std::distance
		(
			std::begin(colorTable),
			std::min_element
			(
				std::begin(colorTable),
				std::end(colorTable),
				colorComp<Red, Green, Blue>
			)
		);
	}

	// Not used in this project

	// static void printChar(char c, uint8_t color = 7)
	// {
	// 	_ASM_VOLATILE_
	// 	(
	// 		"int $0x10"
	// 		:
	// 		: "ax"(0x0e00 | c), "bx"(color)
	// 	);
	// }

	// static void printString(const char *c, uint8_t color = 7)
	// {
	// 	while (*c)
	// 	{
	// 		printChar(*c, color);
	// 		std::advance(c, 1);
	// 	}
	// }

	// static auto readKeyboardChar()
	// {
	// 	int16_t ax{};
	// 	_ASM_VOLATILE_
	// 	(
	// 		"int $0x16"
	// 		: "+ax"(ax)
	// 	);
	// 	return *::bit_cast<char*>(&ax); // Low byte
	// }

	static auto readKeyboardScanCode()
	{
		int16_t ax{};
		_ASM_VOLATILE_
		(
			"int $0x16"
			: "+ax"(ax)
		);
		return *(::bit_cast<int8_t*>(&ax) + 1); // High byte
	}

	[[nodiscard]] static auto isKeyAvailable()
	{
		_ASM_VOLATILE_GOTO_
		(
			"movb $0x01, %%ah\n\t"
			"int $0x16\n\t"
			"jz %l[ret_false]"
			:
			:
			:
			: ret_false
		);
		return true;
		ret_false:
		return false;
	}

	static void setPixel(uint8_t color, uint16_t x, uint16_t y)
	{
		_ASM_VOLATILE_
		(
			"xor %%bh, %%bh\n\t"
			"int $0x10"
			:
			: "ax"(0x0c00 | color), "cx"(x), "dx"(y)
		);
	}

	static void setVideoMode(uint16_t mode)
	{
		_ASM_VOLATILE_
		(
			"int $0x10"
			:
			: "ax"(mode)
		);
	}

	[[nodiscard]] static auto getTicks()
	{
		uint16_t ticks[2];
		_ASM_VOLATILE_
		(
			"int $0x1a"
			: [high_word] "=cx"(ticks[1]), [low_word] "=dx"(ticks[0])
			: "ax"(0x0)
		);

		// return ticks_t(ticks[0]) | (ticks_t(ticks[1]) << 16); // If we used all 32 bits
		return ticks[0];
	}

	static void sleepUntil(ticks_t target)
	{
		while (getTicks() < target)
		{
		}
	}

	static void sleepFor(ticks_t duration)
	{
		sleepUntil(getTicks() + duration);
	}
};

template <uint16_t Size>
class game
{
	using dir_type = std::invoke_result_t<decltype(x86::readKeyboardScanCode)>;
	using len_type = int8_t;

	// Not using enum class on purpose
	enum dirKeys : dir_type
	{
		UP		= 0b01001000,	//0x48
		LEFT	= 0b01001011,	//0x4B
		RIGHT	= 0b01001101,	//0x4D
		DOWN	= 0b01010000	//0x50
	};

	// segment[0] is always the head
	u16Vec2* const segment{ ::bit_cast<u16Vec2*>(intptr_t(0x7E00)) };

	static_assert(sizeof(decltype(segment)) == sizeof(intptr_t));

	dir_type	m_dir{};
	len_type	m_len{}, m_nextLen{};
	u16Vec2		m_foodPos{};
	uint8_t		m_rndOffset{};

	template <typename T>
	[[nodiscard]] T randomData()
	{
		static_assert(sizeof(T) <= 255);
		static_assert(sizeof(T*) == sizeof(intptr_t));

		// lol
		T ret;
		std::memcpy(&ret, ::bit_cast<void*>(intptr_t(0x7C00 + m_rndOffset++)), sizeof(T));
		return ret;
	}

	void spawnFood()
	{
		auto ticks	= x86::getTicks();
		auto rnd	= randomData<x86::ticks_t>();
		m_foodPos.x = (rnd ^ ticks) % Size;
		m_foodPos.y = rnd % Size;
	}

	void reset()
	{
		m_len		= 1;
		m_nextLen	= 1;
		segment[0]	= { Size / 2u, Size / 2u };
	}

	void readInput()
	{
		if (x86::isKeyAvailable())
		{
			auto newInput = x86::readKeyboardScanCode();

			auto isSameAxis = [](const auto& dir1, const auto& dir2)
			{
				// LSB is the same between arrow keys on the same axis (left/right and up/down)
				return (dir1 & 0b1) == (dir2 & 0b1);
			};

			auto isValidInput = [&](const auto& input)
			{
				return (input >= dirKeys::UP && input <= dirKeys::DOWN);
			};

			// Validating input and preventing a 180?? direction change if the length is over 1
			if (isValidInput(newInput))
			{
				if (m_len == 1 || !isSameAxis(m_dir, newInput))
				{
					m_dir = newInput;
				}
			}
		}
	}

	void updateHead()
	{
			 if (m_dir == dirKeys::UP)		segment[0].y--;
		else if (m_dir == dirKeys::LEFT)	segment[0].x--;
		else if (m_dir == dirKeys::RIGHT)	segment[0].x++;
		else if (m_dir == dirKeys::DOWN)	segment[0].y++;
	}

public:

	game() : m_dir{ dirKeys::RIGHT }, m_foodPos{ Size / 5u, Size / 3u }
	{
		x86::setVideoMode(0x0d);
		reset();
	}

	auto scopedFrame(x86::ticks_t duration) const
	{
		class Frame
		{
			x86::ticks_t endTime{};
		public:
			Frame(x86::ticks_t time) : endTime( x86::getTicks() + time )
			{
			}
			~Frame()
			{
				x86::sleepUntil(endTime);
			}
		};

		return Frame(duration);
	}

	[[gnu::always_inline]] void update()
	{
		m_len = m_nextLen;

		readInput();

		// Moving the tail by setting each segment (except the head) to the position of the one in front of it
		for (len_type i = m_len - 1; i > 0; i--)
		{
			segment[i] = segment[i - 1];
		}

		// Moving the head
		updateHead();

		auto isCoordInBounds	= []	(const auto& val) { return val <= (Size - 1u);	};
		auto isHeadAtPosition	= [this](const auto& pos) { return pos == segment[0];	};

		// Resetting on edge collision
		if (!isCoordInBounds(segment[0].x) ||
			!isCoordInBounds(segment[0].y))
		{
			reset();
			return;
		}

		// Resetting on self-collision
		for (len_type i{ 1 }; i < m_len; i++)
		{
			if (isHeadAtPosition(segment[i]))
			{
				reset();
				return;
			}
		}

		// Checking if the food has been reached
		if (isHeadAtPosition(m_foodPos))
		{
			// Only effective on the next frame so that we don't render an uninitialized segment
			m_nextLen++;

			spawnFood();
		}
	}

	void draw() const
	{
		// Play area
		for (uint16_t i{}; i < Size; i++)
		{
			for (uint16_t j{}; j < Size; j++)
			{
				x86::setPixel(x86::rgb<0, 0, 160>(), i, j);
			}
		}

		// Snake
		for (len_type i{}; i < m_len; i++)
		{
			x86::setPixel(x86::rgb<160, 0, 160>(), segment[i].x, segment[i].y);
		}

		// Food
		x86::setPixel(x86::rgb<255, 255, 255>(), m_foodPos.x, m_foodPos.y);
	}

	[[gnu::always_inline]] void mainLoop()
	{
		update();
		draw();
	}
};

int main()
{
	game<16> snake;

	// 2 ticks per frame is about 9.1 FPS
	// If that's too fast, set this to 3 which is about 6.1 FPS
	const auto frameTime = x86::ticks_t(2);

	while (true)
	{
		auto frame = snake.scopedFrame(frameTime);
		snake.mainLoop();
	}

	return 0;
}