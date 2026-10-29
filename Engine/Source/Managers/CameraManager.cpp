#include "CameraManager.h"
#include "Managers/LevelManager.h"
#include "GameFramework/Level.h"
#include "UI/Widgets/Widget.h"

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
			current = GetWorld()->GetLevelManager()->GetCurrentLevel()->GetDefaultCamera();
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

	GetWorld()->GetWindowRef().SetView(GetWorld()->GetLevelManager()->GetCurrentLevel()->GetHUD().GetView()); // Widget View
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

		for (const std::shared_ptr<Vertices>& _vert : screenDebugs)
			_window.Draw(*_vert);
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
	for (const std::shared_ptr<Vertices> _vert : debugs)
	{
		_window.Draw(*_vert);
	}
}

void Krampus::CameraManager::RenderWidgets(Window& _window, const Bool& splitScreen)
{
	for (const auto& [_zOrder, _sprites] : widgets)
		for (IDrawable* _sprite : _sprites)
			_sprite->Draw(_window);
}
