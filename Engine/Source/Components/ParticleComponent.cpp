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
	const bool _finished = emitter.Update(_deltaTime, GetPosition(), static_cast<float>(GetRotation().ToDegrees()), GetScale());
	if (!_finished) return;

	onFinished.Broadcast();
	if (destroyOwnerWhenDone && !GetWorld()->IsLevelPaused()) GetOwner()->Destroy();
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
