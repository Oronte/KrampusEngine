#pragma once
#pragma warning (disable : 4275) // non dll-interface class 'std::runtime_error' used as base for dll-interface class 'sf::Exception'
//#pragma warning( disable : 4244)

// STL
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <functional>
#include <typeindex>
#include <typeinfo>
#include <exception>
#include <random>
#include <filesystem>
#include <any>
#include <thread>
#include <mutex>
#include <format>
#include <limits>
#include <type_traits>
#include <utility>
#include <cstddef>
#include <memory>
#include <cmath>

// Collections
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <set>

// SFML
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include "SFML/System.hpp"
#include "SFML/Audio.hpp"
#include "SFML/Network.hpp"


#define DEBUG_PATH
#ifdef DEBUG_PATH
#define PATH __func__
#else
#define PATH __FILE__
#endif // DEBUG_FILE

#ifdef _MSC_VER
	//#define new							new(_NORMAL_BLOCK, __FILE__, __LINE__)// Memory leaks
#endif

//#define Super							__super // IDEA Find alternative
#define FILE_NAME						std::filesystem::path(__FILE__).filename().string()
#define DEBUG_INFO						std::string("(File : " + FILE_NAME + " | Func : " + CAST(std::string, PATH) + " | Line : " + std::to_string(__LINE__) + ")")
#define INLINE							inline  //__inline
#define FORCEINLINE						INLINE	//__forceinline
#define CAST(_type, _expr)				static_cast<_type>(_expr)
#define SLEEP(_time)					sf::sleep(sf::seconds(_time))
#define SLEEP_MILLISECOND(_time)		sf::sleep(sf::milliseconds(_time))
#define SLEEP_MICROSECONDS(_time)		sf::sleep(sf::microseconds(_time))
#define NO_DISCARD						[[nodiscard]]
#define SAME_VALUE(_first, _second)		std::is_same_v<_first, _second>
#define ENABLE_IF(_element)				std::enable_if_t<_element>
#define IS_BASE_OF(_base, _type)		typename = std::enable_if_t<std::is_base_of_v<_base, _type>>
#define TYPE(_type)						std::decay_t<decltype(_type)>
#define TYPE_ID(_type)					typeid(_type)
#define TYPE_NAME(_type)				CAST(std::string, TYPE_ID(_type).name())
#define TYPE_INDEX_NAME(_typeIndex)		CAST(std::string, _typeIndex.name())
#define NAME_OF(var)					#var
#define CURRENT_PATH					std::filesystem::current_path().string()