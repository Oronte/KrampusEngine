#pragma once

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
#include <cassert>

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

// Krampus
#include "Utilities/System/Types/Types.h"
#include "Utilities/System/Printable.h"

#define DEBUG_INFO		std::string("(File : " + std::filesystem::path(__FILE__).filename().string() + " | Func : " + __func__ + " | Line : " + std::to_string(__LINE__) + ")")
#define NAME_OF(_name)	#_name
#define PROJECT_ROOT	std::string(PROJECT_ROOT_DIR)
#ifdef SHIP
#define CONTENT_PATH	std::string("Content/")		// a shipped game finds Content next to its exe (the exe moves to its own folder at start)
#else
#define CONTENT_PATH	PROJECT_ROOT + "/Content/"
#endif

/// Pixels per meter between the SFML world and Box2D.
inline constexpr float PX_TO_M = 100.f;
