#include "CameraManager.h"
#include "Managers/LevelManager.h"
#include "GameFramework/Level.h"
#include "UI/Widgets/Widget.h"
#include "Utilities/Debug/Debug.h"
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

	UpdateView(); // World View
	RenderActors(_window, splitScreen);
	RenderDebuggables(_window, false);	// the outlines of components that use debug, then the ones the code asked for
	RenderDebugs(_window, splitScreen);

	if (Level* _hudLevel = GetWorld()->GetLevelManager()->GetCurrentLevel()) GetWorld()->GetWindowRef().SetView(_hudLevel->GetHUD().GetView()); // Widget View
	RenderWidgets(_window, splitScreen);
	RenderDebuggables(_window, true);	// widgets are outlined in the HUD view, where they are drawn

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
