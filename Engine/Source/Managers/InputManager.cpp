#include "InputManager.h"
#include "Graphics/Mouse.h"
#include "Graphics/Window.h"

void Krampus::InputManager::Update(const std::optional<sf::Event>& _event)
{
    UpdateKey(A, Key::A);
    UpdateKey(B, Key::B);
    UpdateKey(C, Key::C);
    UpdateKey(D, Key::D);
    UpdateKey(E, Key::E);
    UpdateKey(F, Key::F);
    UpdateKey(G, Key::G);
    UpdateKey(H, Key::H);
    UpdateKey(I, Key::I);
    UpdateKey(J, Key::J);
    UpdateKey(K, Key::K);
    UpdateKey(L, Key::L);
    UpdateKey(M, Key::M);
    UpdateKey(N, Key::N);
    UpdateKey(O, Key::O);
    UpdateKey(P, Key::P);
    UpdateKey(Q, Key::Q);
    UpdateKey(R, Key::R);
    UpdateKey(S, Key::S);
    UpdateKey(T, Key::T);
    UpdateKey(U, Key::U);
    UpdateKey(V, Key::V);
    UpdateKey(W, Key::W);
    UpdateKey(X, Key::X);
    UpdateKey(Y, Key::Y);
    UpdateKey(Z, Key::Z);

    UpdateKey(Num0, Key::Num0);
    UpdateKey(Num1, Key::Num1);
    UpdateKey(Num2, Key::Num2);
    UpdateKey(Num3, Key::Num3);
    UpdateKey(Num4, Key::Num4);
    UpdateKey(Num5, Key::Num5);
    UpdateKey(Num6, Key::Num6);
    UpdateKey(Num7, Key::Num7);
    UpdateKey(Num8, Key::Num8);
    UpdateKey(Num9, Key::Num9);

    UpdateKey(Escape, Key::Escape);
    UpdateKey(LControl, Key::LControl);
    UpdateKey(LShift, Key::LShift);
    UpdateKey(LAlt, Key::LAlt);
    UpdateKey(LSystem, Key::LSystem);
    UpdateKey(RControl, Key::RControl);
    UpdateKey(RShift, Key::RShift);
    UpdateKey(RAlt, Key::RAlt);
    UpdateKey(RSystem, Key::RSystem);
    UpdateKey(Menu, Key::Menu);
    UpdateKey(LBracket, Key::LBracket);
    UpdateKey(RBracket, Key::RBracket);
    UpdateKey(Semicolon, Key::Semicolon);
    UpdateKey(Comma, Key::Comma);
    UpdateKey(Period, Key::Period);
    UpdateKey(Quote, Key::Apostrophe);
    UpdateKey(Slash, Key::Slash);
    UpdateKey(Backslash, Key::Backslash);
    //UpdateKey(Tilde, Key::Tilde);
    UpdateKey(Equal, Key::Equal);
    UpdateKey(Hyphen, Key::Hyphen);
    UpdateKey(Space, Key::Space);
    UpdateKey(Enter, Key::Enter);
    UpdateKey(Backspace, Key::Backspace);
    UpdateKey(Tab, Key::Tab);
    UpdateKey(PageUp, Key::PageUp);
    UpdateKey(PageDown, Key::PageDown);
    UpdateKey(End, Key::End);
    UpdateKey(Home, Key::Home);
    UpdateKey(Insert, Key::Insert);
    UpdateKey(Delete, Key::Delete);
    UpdateKey(F1, Key::F1);
    UpdateKey(F2, Key::F2);
    UpdateKey(F3, Key::F3);
    UpdateKey(F4, Key::F4);
    UpdateKey(F5, Key::F5);
    UpdateKey(F6, Key::F6);
    UpdateKey(F7, Key::F7);
    UpdateKey(F8, Key::F8);
    UpdateKey(F9, Key::F9);
    UpdateKey(F10, Key::F10);
    UpdateKey(F11, Key::F11);
    UpdateKey(F12, Key::F12);
    UpdateKey(F13, Key::F13);
    UpdateKey(F14, Key::F14);
    UpdateKey(F15, Key::F15);
    UpdateKey(Pause, Key::Pause);

    UpdateMouseButton(MouseLeftClick, MouseButton::Left);
    UpdateMouseButton(MouseRightClick, MouseButton::Right);
    UpdateMouseButton(MouseMiddleClick, MouseButton::Middle);
    UpdateMouseButton(MouseExtraButton1, MouseButton::Extra1);
    UpdateMouseButton(MouseExtraButton2, MouseButton::Extra2);

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
    A.Update();
    B.Update();
    C.Update();
    D.Update();
    E.Update();
    F.Update();
    G.Update();
    H.Update();
    I.Update();
    J.Update();
    K.Update();
    L.Update();
    M.Update();
    N.Update();
    O.Update();
    P.Update();
    Q.Update();
    R.Update();
    S.Update();
    T.Update();
    U.Update();
    V.Update();
    W.Update();
    X.Update();
    Y.Update();
    Z.Update();

    Num0.Update();
    Num1.Update();
    Num2.Update();
    Num3.Update();
    Num4.Update();
    Num5.Update();
    Num6.Update();
    Num7.Update();
    Num8.Update();
    Num9.Update();

    Escape.Update();
    LControl.Update();
    LShift.Update();
    LAlt.Update();
    LSystem.Update();
    RControl.Update();
    RShift.Update();
    RAlt.Update();
    RSystem.Update();
    Menu.Update();

    LBracket.Update();
    RBracket.Update();
    Semicolon.Update();
    Comma.Update();
    Period.Update();
    Quote.Update();
    Slash.Update();
    Backslash.Update();
    //Tilde.Update();
    Equal.Update();
    Hyphen.Update();

    Space.Update();
    Enter.Update();
    Backspace.Update();
    Tab.Update();

    PageUp.Update();
    PageDown.Update();
    End.Update();
    Home.Update();
    Insert.Update();
    Delete.Update();

    F1.Update();
    F2.Update();
    F3.Update();
    F4.Update();
    F5.Update();
    F6.Update();
    F7.Update();
    F8.Update();
    F9.Update();
    F10.Update();
    F11.Update();
    F12.Update();
    F13.Update();
    F14.Update();
    F15.Update();

    Pause.Update();

    MouseLeftClick.Update();
    MouseRightClick.Update();
    MouseMiddleClick.Update();
    MouseExtraButton1.Update();
    MouseExtraButton2.Update();

    ButtonA.Update();
    ButtonB.Update();
    ButtonX.Update();
    ButtonY.Update();
    ButtonStart.Update();
    ButtonBack.Update();
    ButtonLB.Update();
    ButtonRB.Update();
    ButtonLS.Update();
    ButtonRS.Update();
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
