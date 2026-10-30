#pragma once
#include "Utilities/Math/Vector2D.h"
#include "Utilities/Debug/Color.h"
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace Krampus
{
	/// <summary>Where new particles appear around the emitter (saved in scenes as an int: never reorder).</summary>
	enum class ParticleShape
	{
		Point,
		Circle,		// shapeRadius
		Rectangle,	// shapeSize
		Line		// shapeSize.x long, along the emitter's X axis (rain, snow)
	};

	/// <summary>How particles mix with what is behind them (saved as an int).</summary>
	enum class ParticleBlend
	{
		Alpha,		// normal transparency (smoke, snow)
		Add			// light adds up (fire, sparks, magic)
	};

	/// <summary>Ready-made settings (saved as an int). Custom keeps the values as they are.</summary>
	enum class ParticlePreset
	{
		Custom,
		Fire,
		Smoke,
		Sparks,
		Explosion,
		Snow,
		Rain,
		Magic
	};

	/// <summary>
	/// Everything that says how an emitter behaves. Angles in degrees (0 = right, 90 = down: the screen Y axis goes down),
	/// distances in pixels, times in seconds.
	/// </summary>
	struct ParticleSettings
	{
		// ---- Emission
		bool		playOnStart		= true;
		bool		looping			= true;
		float		duration		= 1.f;		// one cycle: a burst at its start, then 'rate' particles per second during it
		float		rate			= 60.f;		// particles per second
		int			burst			= 0;		// particles at once at the start of each cycle
		int			maxParticles	= 400;		// size of the pool, taken once: a full pool emits nothing more (no memory taken while playing)
		bool		worldSpace		= true;		// true: particles stay where they were born when the actor moves; false: they follow it

		// ---- Shape of the emitter
		int			shape			= static_cast<int>(ParticleShape::Circle);
		float		shapeRadius		= 10.f;
		FVector2	shapeSize		= FVector2(100.f, 20.f);
		bool		fromEdge		= false;	// born on the outline instead of inside
		bool		radial			= false;	// thrown away from the center (explosions) instead of along 'direction'

		// ---- At birth
		float		lifetimeMin		= 0.6f;
		float		lifetimeMax		= 1.1f;
		float		speedMin		= 40.f;
		float		speedMax		= 90.f;
		float		direction		= -90.f;	// up
		float		spread			= 25.f;		// total angle of the cone around 'direction'
		float		sizeStart		= 26.f;
		float		sizeEnd			= 4.f;
		float		sizeRandom		= 0.2f;		// 0: all the same size, 0.5: from half to full size
		float		rotationMin		= 0.f;
		float		rotationMax		= 0.f;
		float		spinMin			= 0.f;		// degrees per second
		float		spinMax			= 0.f;
		Color		colorStart		= Color(255, 190, 60, 230);
		Color		colorEnd		= Color(255, 40, 10, 0);

		// ---- During their life
		FVector2	gravity			= FVector2(0.f, -60.f);	// pixels per second per second
		float		drag			= 0.5f;		// speed lost per second (0: none, 3: stops quickly)
		float		stretch			= 0.f;		// stretched along the speed (rain, sparks): extra length = speed * stretch

		// ---- Look
		std::string	texture;					// "Folder/Name.png" in Content/Textures; empty: a soft round dot
		int			blend			= static_cast<int>(ParticleBlend::Add);

		/// <summary>Replaces the values by the ones of a ready-made effect (Custom changes nothing).</summary>
		void ApplyPreset(const ParticlePreset& _preset);
	};

	/// <summary>
	/// The simulation of one particle effect, made to cost little:
	///   - the particles live in one array taken once (maxParticles); a dead one is replaced by the last one (no shifting, no memory);
	///   - the vertices are written in a buffer that keeps its size (no allocation per frame);
	///   - the bounds of the particles are known after each update, so an emitter out of the view draws nothing.
	/// It knows nothing about actors or windows: ParticleComponent drives it.
	/// </summary>
	class ParticleEmitter
	{
	public:
		ParticleSettings	settings;

		ParticleEmitter();
		~ParticleEmitter();
		ParticleEmitter(const ParticleEmitter&) = delete;
		ParticleEmitter& operator=(const ParticleEmitter&) = delete;

		/// <summary>Starts a new cycle (with its burst at the next update). Particles already there stay.</summary>
		void Play();

		/// <summary>No new particle; the living ones end their life.</summary>
		void Stop();

		/// <summary>Removes every particle at once.</summary>
		void Clear();

		/// <summary>Emits _count particles at once, at the last place the emitter was updated.</summary>
		void Burst(const int& _count);

		/// <summary>
		/// Moves the time on: emits, ages, moves and removes particles. _position, _rotation (degrees) and _scale are the emitter's.
		/// </summary>
		/// <returns>True the frame the effect ends (not looping, cycle over, last particle gone).</returns>
		bool Update(float _deltaTime, const FVector2& _position, const float& _rotation, const FVector2& _scale);

		/// <summary>
		/// Writes two triangles per particle (6 vertices) at the start of _out, growing it only when it is too small.
		/// _offset is added to every position (the emitter position for local space). _textureSize gives the texture coordinates.
		/// </summary>
		/// <returns>How many vertices were written.</returns>
		std::size_t BuildVertices(std::vector<sf::Vertex>& _out, const sf::Vector2f& _offset, const sf::Vector2f& _textureSize) const;

		bool IsPlaying() const { return playing; }
		int GetAliveCount() const { return static_cast<int>(alive); }

		/// <returns>The rectangle the particles are in (their size included), in the emitter's space. Empty without particles.</returns>
		sf::FloatRect GetBounds() const { return bounds; }

		/// <returns>Particles alive in every emitter (the editor status bar shows it).</returns>
		static int GetTotalAlive();

	private:
		struct Particle
		{
			sf::Vector2f	position;
			sf::Vector2f	velocity;
			float			age;
			float			inverseLife;	// 1 / lifetime: the age is turned into 0..1 with one multiplication
			float			scale;			// its own size factor (sizeRandom)
			float			rotation;		// degrees
			float			spin;			// degrees per second
		};

		std::vector<Particle>	particles;
		std::size_t				alive			= 0;
		bool					playing			= false;
		bool					wasAlive		= false;	// had particles or was playing: "finished" is said once
		bool					burstPending	= false;
		float					cycleTime		= 0.f;
		float					emitCarry		= 0.f;		// part of a particle owed from the last frames
		std::uint32_t			random;
		sf::FloatRect			bounds;

		FVector2				lastPosition;
		float					lastRotation	= 0.f;
		FVector2				lastScale		= FVector2(1.f, 1.f);
		bool					hasLast			= false;

		float Random01();
		float Range(const float& _min, const float& _max);
		void ResizePool();
		void Spawn(const FVector2& _position, const float& _rotation, const FVector2& _scale);
	};
}
