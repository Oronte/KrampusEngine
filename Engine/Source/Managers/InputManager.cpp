#include "InputManager.h"
#include "Graphics/Mouse.h"
#include "Graphics/Window.h"

void Krampus::InputManager::Update(const std::optional<sf::Event>& _event)
{
    // Characters typed, as the keyboard layout gives them (an AZERTY "1" is the Shift + the key, not Num1).
    if (_event.has_value())
        if (const sf::Event::TextEntered* _text = _event->getIf<sf::Event::TextEntered>())
            TextEntered.Broadcast(static_cast<std::uint32_t>(_text->unicode));
    // The state of every key and mouse button, from the list of InputKeys.def.
#define KRAMPUS_KEY(Name, SfmlKey) UpdateKey(Name, Key::SfmlKey);
#define KRAMPUS_MOUSE_BUTTON(Name, SfmlName) UpdateMouseButton(Name, MouseButton::SfmlName);
#include "InputKeys.def"

    if (!_event.has_value()) return;

    Engine* _world = GetWorld();

    const Mouse* _mouse = _world->GetMouse();
    const Window& _window = _world->GetWindowRef();
    const sf::View _view = _window.GetView();
    if (_mouse->HasMoved() && _window.HasFocus())
    {
        MouseMovedWorld.Broadcast(_mouse->GetPosition());
        MouseMovedScreen.Broadcast(_mouse->GetScreenPosition());
    }

    if (const sf::Event::MouseWheelScrolled* _mouseWheelScrolled = _event->getIf<sf::Event::MouseWheelScrolled>())
        MouseWheelScroll.Broadcast(_mouseWheelScrolled->delta);
}

void Krampus::InputManager::UpdateOnPerform()
{
    // A held input broadcasts onPerform every frame.
#define KRAMPUS_KEY(Name, SfmlKey) Name.Update();
#define KRAMPUS_MOUSE_BUTTON(Name, SfmlName) Name.Update();
#define KRAMPUS_PAD_BUTTON(Name) Name.Update();
#include "InputKeys.def"
}

void Krampus::InputManager::UpdateSystemEvent(const std::optional<sf::Event>& _event)
{
    if (!_event.has_value()) return;

    if (_event->is<sf::Event::FocusGained>()) FocusGain.Broadcast();
    if (_event->is<sf::Event::FocusLost>()) FocusLost.Broadcast();
    if (_event->is<sf::Event::MouseEntered>()) MouseEntred.Broadcast();
    if (_event->is<sf::Event::MouseLeft>()) MouseLeft.Broadcast();

    if (const sf::Event::Resized* _resized = _event->getIf<sf::Event::Resized>())
        WindowResize.Broadcast(_resized->size);
    if (_event->is<sf::Event::Closed>()) WindowClose.Broadcast();
}

void Krampus::InputManager::UpdateKey(Input& _input, const Key& _key)
{
	if (sf::Keyboard::isKeyPressed(_key)) _input.Press();
	else _input.Release();
}

void Krampus::InputManager::UpdateMouseButton(Input& _input, const MouseButton& _mouseButton)
{
	if (sf::Mouse::isButtonPressed(_mouseButton)) _input.Press();
	else _input.Release();
}
