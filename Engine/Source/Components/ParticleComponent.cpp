#include "ParticleComponent.h"
#include "Actors/Actor.h"
#include "Core/Engine.h"
#include "GameFramework/Level.h"
#include "Graphics/Window.h"
#include "Managers/CameraManager.h"
#include "Managers/TextureManager.h"
#include "Utilities/Debug/Debug.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <cmath>

using namespace Krampus;

namespace
{
	constexpr float PREVIEW_PAUSE = 0.8f;	// in the editor, a one-shot effect plays again after this many seconds

	// The picture of a particle without texture: a white dot that fades to its edge (the color of the particle tints it).
	const sf::Texture& SoftDot()
	{
		static std::unique_ptr<sf::Texture> _dot;
		if (!_dot)
		{
			constexpr unsigned SIZE = 64;
			sf::Image _image({ SIZE, SIZE }, sf::Color::Transparent);
			for (unsigned _y = 0; _y < SIZE; ++_y)
				for (unsigned _x = 0; _x < SIZE; ++_x)
				{
					const float _dx = (_x + 0.5f) / SIZE * 2.f - 1.f, _dy = (_y + 0.5f) / SIZE * 2.f - 1.f;
					const float _edge = std::max(0.f, 1.f - std::sqrt(_dx * _dx + _dy * _dy));
					_image.setPixel({ _x, _y }, sf::Color(255, 255, 255, static_cast<std::uint8_t>(255.f * _edge * _edge * (3.f - 2.f * _edge))));
				}
			_dot = std::make_unique<sf::Texture>();
			if (!_dot->loadFromImage(_image)) LOG_WARNING("ParticleComponent: the soft dot texture could not be made");
			_dot->setSmooth(true);
		}
		return *_dot;
	}
}

ParticleComponent::ParticleComponent(Actor* _owner, const ParticlePreset& _preset)
	: Component(_owner)
{
	name = NAME_OF(ParticleComponent);
	IDrawable::SetZOrder(ZOrder::Particles);	// not in the world yet: Construct puts it there
	ApplyPreset(_preset);
}

void ParticleComponent::Construct()
{
	Component::Construct();
	GetLevel()->GetCameraManager().AddToWorld(this);
	if (Settings().playOnStart && !IsPlaying()) emitter.Play();	// shown at once, in the editor too
}

void ParticleComponent::Deconstruct()
{
	Component::Deconstruct();
	GetLevel()->GetCameraManager().RemoveToWorld(this);
}

void ParticleComponent::BeginPlay()
{
	Component::BeginPlay();
	emitter.Clear();	// the game starts from nothing, not from what the editor preview had
	if (Settings().playOnStart) emitter.Play();
	else emitter.Stop();
}

void ParticleComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);
	Simulate(static_cast<float>(_deltaTime));
}

void ParticleComponent::Simulate(const float& _deltaTime)
{
	const bool _running = !GetWorld()->IsLevelPaused();
	emitter.collectDeaths = _running && static_cast<ParticleStainMode>(Settings().stainMode) == ParticleStainMode::OnDeath;
	const bool _finished = emitter.Update(_deltaTime, GetPosition(), static_cast<float>(GetRotation().ToDegrees()), GetScale());
	if (_running) LeaveStains();
	if (!_finished) return;

	onFinished.Broadcast();
	if (destroyOwnerWhenDone && !GetWorld()->IsLevelPaused()) GetOwner()->Destroy();
}

bool ParticleComponent::StainRoll()
{
	const float _chance = Settings().stainChance;
	if (_chance >= 1.f) return true;
	stainRandom ^= stainRandom << 13;
	stainRandom ^= stainRandom >> 17;
	stainRandom ^= stainRandom << 5;
	return static_cast<float>(stainRandom >> 8) * (1.f / 16777216.f) < _chance;
}

void ParticleComponent::LeaveStains()
{
	const ParticleSettings& _s = Settings();
	const ParticleStainMode _mode = static_cast<ParticleStainMode>(_s.stainMode);
	if (_mode == ParticleStainMode::None) return;

	const sf::Vector2f _offset = _s.worldSpace ? sf::Vector2f() : sf::Vector2f(GetPosition().x, GetPosition().y);
	const StainStyle _style = static_cast<StainStyle>(std::clamp(_s.stainStyle, 0, 2));
	Level* _level = GetLevel();

	if (_mode == ParticleStainMode::OnDeath)
	{
		for (const sf::Vector2f& _death : emitter.GetDeaths())
			if (StainRoll())
				StainComponent::StainAt(_level, FVector2(_death.x + _offset.x, _death.y + _offset.y), _s.stainSize, _s.stainColor, _style, GetOwner());
		return;
	}

	// On hit: each particle is tested against the rectangles of the markable actors first (cheap), their exact shape only when inside.
	StainComponent::GatherCandidates(_level, GetOwner(), stainTargets);
	if (stainTargets.empty()) return;
	constexpr float GRACE = 0.05f;	// a newborn particle may start inside something: it is not stopped at once
	emitter.RemoveIf([&](const sf::Vector2f& _position, const float& _age)
	{
		if (_age < GRACE) return false;
		const sf::Vector2f _world = _position + _offset;
		for (const StainComponent::Candidate& _target : stainTargets)
		{
			if (!_target.bounds.contains(_world)) continue;
			const FVector2 _point(_world.x, _world.y);
			if (!_target.stain->Contains(_point)) continue;
			if (StainRoll()) _target.stain->Stain(_point, _s.stainSize, _s.stainColor, _style);
			return true;	// the drop ends on what it touched
		}
		return false;
	});
}

void ParticleComponent::ApplyPreset(const ParticlePreset& _preset)
{
	preset = static_cast<int>(_preset);
	Settings().ApplyPreset(_preset);
	if (Settings().playOnStart) Restart();	// what the new values look like, at once
}

void ParticleComponent::SetZOrder(const uint8_t& _zOrder)
{
	const uint8_t _old = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	GetLevel()->GetCameraManager().ChangeWorldZOrder(this, _old);
}

void ParticleComponent::SetZOrder(const ZOrder& _zOrder)
{
	SetZOrder(static_cast<uint8_t>(_zOrder));
}

void ParticleComponent::Draw(Window& _window)
{
	if (!isActive) return;

	// The level does not tick in the editor: the preview moves the effect on with its own clock, and replays one-shot effects.
	if (GetWorld()->IsLevelPaused())
	{
		const float _elapsed = previewClock.restart().asSeconds();
		if (previewInEditor)
		{
			if (!previewStarted) previewStarted = true;
			else
			{
				Simulate(_elapsed);
				if (!IsPlaying() && GetAliveCount() == 0 && Settings().playOnStart)
				{
					previewWait += _elapsed;
					if (previewWait >= PREVIEW_PAUSE) { emitter.Play(); previewWait = 0.f; }
				}
			}
		}
	}
	else previewStarted = false;

	// Out of the view (or nothing to draw): no vertex is written.
	if (GetAliveCount() == 0) return;
	const sf::Vector2f _offset = Settings().worldSpace ? sf::Vector2f() : sf::Vector2f(GetPosition().x, GetPosition().y);
	sf::FloatRect _bounds = emitter.GetBounds();
	_bounds.position += _offset;

	sf::RenderWindow& _target = _window.GetRenderWindow();
	const sf::View& _view = _target.getView();
	sf::Vector2f _viewSize = _view.getSize();
	if (_view.getRotation() != sf::Angle::Zero)	// a turned view: the square around it
	{
		const float _diagonal = std::sqrt(_viewSize.x * _viewSize.x + _viewSize.y * _viewSize.y);
		_viewSize = sf::Vector2f(_diagonal, _diagonal);
	}
	const sf::FloatRect _viewRect(_view.getCenter() - _viewSize * 0.5f, _viewSize);
	if (!_viewRect.findIntersection(_bounds)) return;

	if (loadedTexture != Settings().texture)
	{
		loadedTexture = Settings().texture;
		texture = loadedTexture.empty() ? nullptr : M_TEXTURE.GetSharedTexture(String(loadedTexture));
	}
	const sf::Texture& _picture = texture ? *texture : SoftDot();

	const std::size_t _count = emitter.BuildVertices(vertices, _offset, sf::Vector2f(_picture.getSize()));
	sf::RenderStates _states;
	_states.texture = &_picture;
	_states.blendMode = static_cast<ParticleBlend>(Settings().blend) == ParticleBlend::Add ? sf::BlendAdd : sf::BlendAlpha;
	_target.draw(vertices.data(), _count, sf::PrimitiveType::Triangles, _states);	// every particle in one draw call
}

void ParticleComponent::DrawDebug()
{
#ifdef DEBUG
	const ParticleSettings& _s = Settings();
	const FVector2 _scale = GetScale();
	switch (static_cast<ParticleShape>(_s.shape))
	{
	case ParticleShape::Circle:
		Debug::DrawDebugCircle(GetLevel(), GetPosition(), _s.shapeRadius * std::max(std::abs(_scale.x), std::abs(_scale.y)), 24, Color::Yellow());
		break;
	case ParticleShape::Rectangle:
		Debug::DrawDebugRect(GetLevel(), GetPosition(), FVector2(_s.shapeSize.x * std::abs(_scale.x), _s.shapeSize.y * std::abs(_scale.y)), GetRotation(), Color::Yellow());
		break;
	case ParticleShape::Line:
		Debug::DrawDebugRect(GetLevel(), GetPosition(), FVector2(_s.shapeSize.x * std::abs(_scale.x), 1.f), GetRotation(), Color::Yellow());
		break;
	default:
		Debug::DrawDebugCircle(GetLevel(), GetPosition(), 4.f, 8, Color::Yellow());
		break;
	}
#endif
}

std::string ParticleComponent::ToString() const
{
	return name + " -> " + std::to_string(GetAliveCount()) + " particle(s)" + (IsPlaying() ? ", playing" : "");
}
