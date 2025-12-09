#pragma once
#include "Component.h"
#include "Graphics/Ressources/Vertices.h"
#include "Utilities/System/Drawable.h"

namespace Krampus
{

	class SoftBodyComponent : public Component, public IDrawable
	{
	public:
		enum class ShapeKind { Ring, Grid };

		struct Particle
		{
			FVector2 positionLocal;
			FVector2 velocity;
			float mass = 1.0f;
			bool pinned = false;
		};

		struct Spring
		{
			int a, b;
			float restLength;
			float k;
			float damping;
		};

	private:
		std::vector<Particle> particles;
		std::vector<Spring> springs;

		ShapeKind kind = ShapeKind::Ring;
		// ring
		int ringCount = 24;
		float ringRadius = 100.f;
		// grid
		int gridW = 0, gridH = 0;
		FVector2 gridSize{ 0.f, 0.f };

		// simulation params
		FVector2 gravity{ 0.f, 800.f };
		float globalDamping = 0.999f;
		float dt = 1.f / 120.f;
		int substeps = 2;

		// drawing params
		bool drawSprings = true;
		bool drawFill = true;
		Color fillColor = Color(60, 120, 200, 100);
		Color springColor = Color(140, 140, 220, 255);
		Color particleColor = Color::Yellow();

		// interaction
		int grabbedIndex = -1;
		float grabRadius = 12.f;

		// helpers
		FVector2 ToWorld(const FVector2& local) const noexcept;
		FVector2 ToLocal(const FVector2& world) const noexcept;

		// builders
		void BuildRing(int pointCount, float radius, float kstruct, float kshear, float kradial);
		void BuildGrid(int width, int height, const FVector2& size, float kstruct, float kshear);

	public:
		// ctor ring
		SoftBodyComponent(Actor* _owner,
			int pointCount = 24,
			float radius = 100.f,
			float kstruct = 1200.f,
			float kshear = 800.f,
			float kradial = 700.f);

		// ctor grid
		SoftBodyComponent(Actor* _owner,
			int width, int height,
			const FVector2& size,
			float kstruct = 900.f,
			float kshear = 700.f);

		virtual void Construct() override;
		virtual void Deconstruct() override;
		virtual void Tick(const float& _deltaTime) override;

		// render (uses Vertices) - window.Draw(vertices) attendu
		virtual void Draw(Window& window) override;

		// interaction API (à appeler depuis ton code d'input en pixel->world)
		// -> worldPos = camera.MapPixelToCoords(mousePixel);
		bool GrabAtWorldPos(const FVector2& worldPos);    // returns true if grabbed
		void MoveGrabbedTo(const FVector2& worldPos);     // set grabbed particle to this pos
		void ReleaseGrab();

		// runtime tweaks
		void SetGravity(const FVector2& g) { gravity = g; }
		void SetDrawSprings(bool v) { drawSprings = v; }
		void SetDrawFill(bool v) { drawFill = v; }
		void PinParticle(int index, bool pinned = true);
		const std::vector<Particle>& GetParticles() const { return particles; }
	};

}

