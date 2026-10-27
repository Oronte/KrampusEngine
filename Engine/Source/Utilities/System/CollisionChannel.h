#pragma once
#include <cstdint>
#include <type_traits>

class b2Filter;

namespace Krampus
{


	/// <summary>
	/// Bitmask flags representing collision filtering layers and channels.
	/// Maps 1:1 onto b2Filter::categoryBits / maskBits (uint64_t since Box2D v3.1).
	/// </summary>
	enum class CollisionChannel : uint64_t
	{
		None				= 0,
		Wall			    = 1ull << 0,
		Player			    = 1ull << 1,
		Enemy			    = 1ull << 2,
		Projectile		    = 1ull << 3,
		Trigger			    = 1ull << 4,

		// Environment & World
		StaticWorld		    = 1ull << 5,   // Non-destructible static geometry
		DynamicWorld	    = 1ull << 6,   // Moving objects (platforms, elevators)
		Destructible	    = 1ull << 7,   // Breakable elements (crates, glass)
		Terrain			    = 1ull << 8,   // Ground, landscape, heightmaps
		Water			    = 1ull << 9,   // Water volumes or fluids
		Foliage			    = 1ull << 10,  // Trees, vegetation (often ignored by heavy physics)

		// Entities & Characters
		NPC				    = 1ull << 11,  // Friendly or neutral NPCs
		Boss			    = 1ull << 12,  // Bosses or large entities
		Vehicle			    = 1ull << 13,  // Vehicles, mounts
		Companion		    = 1ull << 14,  // Pets, summons

		// Projectiles & Specializations
		PlayerProjectile	= 1ull << 15,  // Player shots (for friendly-fire filtering)
		EnemyProjectile     = 1ull << 16,  // Enemy shots
		AreaOfEffect	    = 1ull << 17,  // Temporary damage zones (AoE, hazard fields, gas)

		// Physics & Props
		PhysicsProp		    = 1ull << 18,  // Rigid bodies simulating physics/gravity
		Ragdoll			    = 1ull << 19,  // Physics-driven bodies / ragdolls
		Pickup			    = 1ull << 20,  // Loot, ground weapons, power-ups
		Shield			    = 1ull << 21,  // Shields, energy barriers

		// Sensors & Triggers
		Interactable	    = 1ull << 22,  // Interactable objects (doors, buttons)
		Vision			    = 1ull << 23,  // AI line-of-sight / perception raycasts
		AudioTrigger	    = 1ull << 24,  // Sound modification / reverb zones
		Checkpoint		    = 1ull << 25,  // Progression save points / triggers
		KillZone		    = 1ull << 26,  // Instant-death volumes / out-of-bounds

		// Camera, System & Utilities
		Camera			    = 1ull << 27,  // Camera occlusion / collision avoidance
		Cursor			    = 1ull << 28,  // Mouse cursor / screen-to-world raycasts
		NavMesh			    = 1ull << 29,  // Obstacles considered for AI pathfinding

		// Localized Hitboxes (if handled at channel level)
		HitboxHead		    = 1ull << 30,  // Critical hit zone
		HitboxLimb		    = 1ull << 31,  // Limbs (arms, legs)

		// Add new channels here, continuing the bit sequence (up to 64 total).

		WorldAll			= Wall | StaticWorld | DynamicWorld | Terrain,
		CharactersAll		= Player | Enemy | NPC | Boss,
		ProjectilesAll		= Projectile | PlayerProjectile | EnemyProjectile,
		SolidsAll			= WorldAll | CharactersAll | PhysicsProp,
		All					= ~0ull
	};

	// -----------------------------------------------------------------------
	// Bitwise operators
	// -----------------------------------------------------------------------

	/// <summary>Bitwise OR — combines two channel masks.</summary>
	constexpr CollisionChannel operator|(CollisionChannel _a, CollisionChannel _b) noexcept
	{
		return static_cast<CollisionChannel>(static_cast<uint64_t>(_a) | static_cast<uint64_t>(_b));
	}

	/// <summary>Bitwise AND — intersects two channel masks.</summary>
	constexpr CollisionChannel operator&(CollisionChannel _a, CollisionChannel _b) noexcept
	{
		return static_cast<CollisionChannel>(static_cast<uint64_t>(_a) & static_cast<uint64_t>(_b));
	}

	/// <summary>Bitwise XOR — toggles the bits present in exactly one of the two masks.</summary>
	constexpr CollisionChannel operator^(CollisionChannel _a, CollisionChannel _b) noexcept
	{
		return static_cast<CollisionChannel>(static_cast<uint64_t>(_a) ^ static_cast<uint64_t>(_b));
	}

	/// <summary>Bitwise NOT — inverts a channel mask (useful for exclusion masks, e.g. All & ~Trigger).</summary>
	constexpr CollisionChannel operator~(CollisionChannel _a) noexcept
	{
		return static_cast<CollisionChannel>(~static_cast<uint64_t>(_a));
	}

	constexpr CollisionChannel& operator|=(CollisionChannel& _a, CollisionChannel _b) noexcept { return _a = _a | _b; }
	constexpr CollisionChannel& operator&=(CollisionChannel& _a, CollisionChannel _b) noexcept { return _a = _a & _b; }
	constexpr CollisionChannel& operator^=(CollisionChannel& _a, CollisionChannel _b) noexcept { return _a = _a ^ _b; }

	// -----------------------------------------------------------------------
	// Queries
	// -----------------------------------------------------------------------

	/// <summary>Raw uint64_t view of a channel mask (e.g. for storage/serialization).</summary>
	[[nodiscard]] constexpr uint64_t ToBits(CollisionChannel _channel) noexcept
	{
		return static_cast<uint64_t>(_channel);
	}

	/// <summary>True if _mask shares at least one bit with _flags.</summary>
	[[nodiscard]] constexpr bool HasAny(CollisionChannel _mask, CollisionChannel _flags) noexcept
	{
		return (_mask & _flags) != CollisionChannel::None;
	}

	/// <summary>True if _mask contains every bit of _flags.</summary>
	[[nodiscard]] constexpr bool HasAll(CollisionChannel _mask, CollisionChannel _flags) noexcept
	{
		return (_mask & _flags) == _flags;
	}

	static_assert(std::is_same_v<std::underlying_type_t<CollisionChannel>, uint64_t>,
		"CollisionChannel must stay uint64_t to match b2Filter::categoryBits/maskBits.");

	[[nodiscard]] ::b2Filter MakeFilter(CollisionChannel _category, CollisionChannel _mask, int32_t _groupIndex = 0) noexcept;
}