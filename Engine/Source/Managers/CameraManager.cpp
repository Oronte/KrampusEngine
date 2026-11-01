#include "CameraManager.h"
#include "Managers/LevelManager.h"
#include "GameFramework/Level.h"
#include "UI/Widgets/Widget.h"
#include "Components/PostProcessComponent.h"
#include "Utilities/Debug/Debug.h"
#include "Utilities/Debug/Profiler.h"
#include <cmath>

void Krampus::CameraManager::UpdateView()
{
	if (viewOverride)
	{
		GetWorld()->GetWindowRef().SetView(*viewOverride);
		return;
	}
	if (!current) return;
	GetWorld()->GetWindowRef().SetView(current);
}

void Krampus::CameraManager::Render(const Bool& splitScreen)
{
	static bool warned = false;
	if (!viewOverride)
	{
		if (!current)
			if (Level* _level = GetWorld()->GetLevelManager()->GetCurrentLevel()) current = _level->GetDefaultCamera();
		if (!current)
		{
			if (!warned) LOG_WARNING("There is no current camera ! Objects cannot be drawn on the screen.");
			warned = true;
			return;
		}
		warned = false;
	}

	Window& _window = GetWorld()->GetWindowRef();
	Level* _level = GetWorld()->GetLevelManager()->GetCurrentLevel();

	// The world through each camera: the current one, or each camera of the split screen in its part of the window (not while
	// the editor looks at the scene with its own view).
	std::vector<CameraComponent*> _cameras;
	if (!viewOverride && IsSplitScreen()) _cameras = splitCameras;
	const auto _eachView = [&](const auto& _draw)
	{
		if (_cameras.empty()) { UpdateView(); _draw(); return; }
		for (CameraComponent* _camera : _cameras) { _window.SetView(_camera); _draw(); }
	};

	{
		PROFILE_SCOPE("World");
		_eachView([&]() { RenderActors(_window, splitScreen); });
	}
	{
		PROFILE_SCOPE("Lights");
		_eachView([&]() { lights.Render(_level, _window.GetRenderWindow()); });
	}
	{
		PROFILE_SCOPE("Post process");
		RenderPostProcess(_window);		// on the whole window
	}
	{
		PROFILE_SCOPE("Debug draw");
		_eachView([&]()
			{
				RenderDebuggables(_window, false);	// the outlines of components that use debug, then the ones the code asked for
				RenderDebugs(_window, splitScreen);
			});
	}
	if (!_cameras.empty()) UpdateView();

	if (Level* _hudLevel = GetWorld()->GetLevelManager()->GetCurrentLevel()) GetWorld()->GetWindowRef().SetView(_hudLevel->GetHUD().GetView()); // Widget View
	{
		PROFILE_SCOPE("HUD draw");
		RenderWidgets(_window, splitScreen);
		RenderDebuggables(_window, true);	// widgets are outlined in the HUD view, where they are drawn
	}

	//UpdateView(); // World View

	debugs.clear();
}

void Krampus::CameraManager::RenderDebuggables(Window& _window, const Bool& _screen)
{
#ifdef DEBUG
	Level* _level = GetWorld()->GetLevelManager()->GetCurrentLevel();
	if (!_level) return;

	drawingScreenDebug = _screen;
	if (!_screen)
	{
		for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
		{
			if (!_actor->IsActive()) continue;
			for (const std::unique_ptr<Component>& _component : _actor->GetComponents())
				if (_component->IsActive())
					if (IDebuggable* _debuggable = dynamic_cast<IDebuggable*>(_component.get()))
						_debuggable->RenderDebug();
		}
	}
	else
	{
		for (const std::unique_ptr<Widget>& _widget : _level->GetHUD().GetWidgets())
		{
			if (!_widget->IsActive()) continue;
			for (const std::unique_ptr<WidgetComponent>& _component : _widget->GetComponents())
				if (_component->IsActive())
					if (IDebuggable* _debuggable = dynamic_cast<IDebuggable*>(_component.get()))
						_debuggable->RenderDebug();
		}

		DrawDebugList(_window, screenDebugs);
		screenDebugs.clear();
	}
	drawingScreenDebug = false;
#endif
}

// Each effect takes the picture as it is (a copy of the window) and draws it again through its shader, over the whole window.
void Krampus::CameraManager::RenderPostProcess(Window& _window)
{
	Level* _level = GetWorld()->GetLevelManager()->GetCurrentLevel();
	if (!_level) return;
	const std::vector<PostProcessComponent*> _effects = PostProcessComponent::EffectsOf(_level, GetWorld()->IsLevelPaused());
	if (_effects.empty()) return;

	sf::RenderWindow& _target = _window.GetRenderWindow();
	const sf::Vector2u _size = _target.getSize();
	if (_size.x == 0 || _size.y == 0) return;
	if (screenCopy.getSize() != _size && !screenCopy.resize(_size)) return;

	const sf::View _view = _target.getView();
	_target.setView(sf::View(sf::FloatRect({ 0.f, 0.f }, sf::Vector2f(_size))));
	for (const PostProcessComponent* _effect : _effects)
	{
		screenCopy.update(_target);
		sf::RenderStates _states;
		_states.blendMode = sf::BlendNone;	// the result replaces the picture
		if (!_effect->material.Apply(_states, &screenCopy, sf::Vector2f(_size))) continue;
		_target.draw(sf::Sprite(screenCopy), _states);
	}
	_target.setView(_view);
}

void Krampus::CameraManager::RenderActors(Window& _window, const Bool& splitScreen)
{
	for (const auto& [_zOrder, _sprites] : actors)
		for (IDrawable* _sprite : _sprites)
			_sprite->Draw(_window);
}

void Krampus::CameraManager::RenderDebugs(Window& _window, const Bool& splitScreen)
{
	DrawDebugList(_window, debugs);
}

// The GPU draws lines and points 1 pixel wide: shown smaller (the editor Viewport) or between two pixels, some vanish. They are made
// into thin quads instead, Debug::lineThickness pixels wide on the screen whatever the zoom, and drawn together in one call.
void Krampus::CameraManager::DrawDebugList(Window& _window, const std::vector<std::shared_ptr<Vertices>>& _list)
{
	if (_list.empty()) return;
	sf::RenderWindow& _target = _window.GetRenderWindow();
	const sf::View& _view = _target.getView();
	const float _viewportWidth = static_cast<float>(std::max(1, _target.getViewport(_view).size.x));
	const float _worldPerPixel = _view.getSize().x / _viewportWidth;
	const float _half = std::max(0.5f, Debug::lineThickness) * std::max(1.f, Debug::displayScale) * _worldPerPixel * 0.5f;

	debugTriangles.clear();
	const auto _quad = [&](const sf::Vector2f& _a, const sf::Vector2f& _b, const sf::Vector2f& _c, const sf::Vector2f& _d, const sf::Color& _colorA, const sf::Color& _colorB)
	{
		debugTriangles.push_back(sf::Vertex{ _a, _colorA });
		debugTriangles.push_back(sf::Vertex{ _b, _colorA });
		debugTriangles.push_back(sf::Vertex{ _c, _colorB });
		debugTriangles.push_back(sf::Vertex{ _a, _colorA });
		debugTriangles.push_back(sf::Vertex{ _c, _colorB });
		debugTriangles.push_back(sf::Vertex{ _d, _colorB });
	};
	const auto _segment = [&](const sf::Vertex& _from, const sf::Vertex& _to)
	{
		sf::Vector2f _direction = _to.position - _from.position;
		const float _length = std::sqrt(_direction.x * _direction.x + _direction.y * _direction.y);
		if (_length < 1e-6f) return;
		_direction *= _half / _length;
		const sf::Vector2f _side(-_direction.y, _direction.x);
		// Square ends, half the thickness past each point: the corners of an outline are closed.
		_quad(_from.position - _direction + _side, _from.position - _direction - _side, _to.position + _direction - _side, _to.position + _direction + _side, _from.color, _to.color);
	};
	const auto _point = [&](const sf::Vertex& _vertex)
	{
		const sf::Vector2f _p = _vertex.position;
		_quad({ _p.x - _half, _p.y - _half }, { _p.x + _half, _p.y - _half }, { _p.x + _half, _p.y + _half }, { _p.x - _half, _p.y + _half }, _vertex.color, _vertex.color);
	};

	for (const std::shared_ptr<Vertices>& _vertices : _list)
	{
		const sf::VertexArray& _array = _vertices->Get();
		const std::size_t _count = _array.getVertexCount();
		switch (_array.getPrimitiveType())
		{
		case sf::PrimitiveType::LineStrip:
			for (std::size_t _i = 0; _i + 1 < _count; ++_i) _segment(_array[_i], _array[_i + 1]);
			break;
		case sf::PrimitiveType::Lines:
			for (std::size_t _i = 0; _i + 1 < _count; _i += 2) _segment(_array[_i], _array[_i + 1]);
			break;
		case sf::PrimitiveType::Points:
			for (std::size_t _i = 0; _i < _count; ++_i) _point(_array[_i]);
			break;
		default:
			_target.draw(_array);	// filled shapes already have an area
			break;
		}
	}
	if (!debugTriangles.empty()) _target.draw(debugTriangles.data(), debugTriangles.size(), sf::PrimitiveType::Triangles);
}

void Krampus::CameraManager::RenderWidgets(Window& _window, const Bool& splitScreen)
{
	for (const auto& [_zOrder, _sprites] : widgets)
		for (IDrawable* _sprite : _sprites)
			_sprite->Draw(_window);
}

// ---- Split screen

namespace
{
	// The parts of the window (0 to 1) for _count cameras.
	std::vector<sf::FloatRect> SplitLayout(const size_t _count, const bool _stacked)
	{
		switch (_count)
		{
		case 1: return { sf::FloatRect({ 0.f, 0.f }, { 1.f, 1.f }) };
		case 2: return _stacked
			? std::vector<sf::FloatRect>{ sf::FloatRect({ 0.f, 0.f }, { 1.f, 0.5f }), sf::FloatRect({ 0.f, 0.5f }, { 1.f, 0.5f }) }
			: std::vector<sf::FloatRect>{ sf::FloatRect({ 0.f, 0.f }, { 0.5f, 1.f }), sf::FloatRect({ 0.5f, 0.f }, { 0.5f, 1.f }) };
		case 3: return { sf::FloatRect({ 0.f, 0.f }, { 0.5f, 0.5f }), sf::FloatRect({ 0.5f, 0.f }, { 0.5f, 0.5f }), sf::FloatRect({ 0.f, 0.5f }, { 1.f, 0.5f }) };
		default: return { sf::FloatRect({ 0.f, 0.f }, { 0.5f, 0.5f }), sf::FloatRect({ 0.5f, 0.f }, { 0.5f, 0.5f }), sf::FloatRect({ 0.f, 0.5f }, { 0.5f, 0.5f }), sf::FloatRect({ 0.5f, 0.5f }, { 0.5f, 0.5f }) };
		}
	}

	// A camera put in another part of the window: its view gets the shape of that part, at the same zoom.
	void Place(Krampus::CameraComponent* _camera, const sf::FloatRect& _part, const sf::Vector2f& _window)
	{
		sf::View& _view = *_camera->GetView();
		const sf::FloatRect _old = _view.getViewport();
		const float _zoom = _old.size.x > 0.f && _window.x > 0.f ? _view.getSize().x / (_window.x * _old.size.x) : 1.f;
		_view.setViewport(_part);
		_view.setSize({ _window.x * _part.size.x * _zoom, _window.y * _part.size.y * _zoom });
	}
}

void Krampus::CameraManager::SetSplitScreen(const std::vector<CameraComponent*>& _cameras, const bool& _stacked)
{
	ClearSplitScreen();
	for (CameraComponent* _camera : _cameras)
		if (_camera && std::find(splitCameras.begin(), splitCameras.end(), _camera) == splitCameras.end()) splitCameras.push_back(_camera);
	if (splitCameras.size() > 4)
	{
		LOG_WARNING("Camera: split screen of " + std::to_string(splitCameras.size()) + " cameras, only the first 4 are shown");
		splitCameras.resize(4);
	}
	if (splitCameras.size() < 2) { splitCameras.clear(); return; }

	const sf::Vector2f _window(GetWorld()->GetWindowRef().GetRenderWindow().getSize());
	const std::vector<sf::FloatRect> _layout = SplitLayout(splitCameras.size(), _stacked);
	for (size_t _i = 0; _i < splitCameras.size(); ++_i) Place(splitCameras[_i], _layout[_i], _window);
	LOG(VerbosityType::Log, "Camera: split screen of " + std::to_string(splitCameras.size()) + " cameras");
}

void Krampus::CameraManager::ClearSplitScreen()
{
	const sf::Vector2f _window(GetWorld()->GetWindowRef().GetRenderWindow().getSize());
	for (CameraComponent* _camera : splitCameras) Place(_camera, sf::FloatRect({ 0.f, 0.f }, { 1.f, 1.f }), _window);
	splitCameras.clear();
}

void Krampus::CameraManager::RemoveSplitCamera(CameraComponent* _camera)
{
	if (std::find(splitCameras.begin(), splitCameras.end(), _camera) == splitCameras.end()) return;
	std::vector<CameraComponent*> _others = splitCameras;
	std::erase(_others, _camera);
	const sf::Vector2f _window(GetWorld()->GetWindowRef().GetRenderWindow().getSize());
	Place(_camera, sf::FloatRect({ 0.f, 0.f }, { 1.f, 1.f }), _window);
	splitCameras.clear();
	SetSplitScreen(_others);	// the others share the window again (one left: back to one camera)
}
