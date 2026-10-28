#pragma once
#include "Component.h"

namespace Krampus
{

	/// <summary>
	/// Component that facilitates binding actor methods and callbacks to input manager events.
	/// </summary>
	class InputComponent : public Component
	{
		std::vector<Event<>::ListenerHandle>			handlesVoid;
		std::vector<Event<Float>::ListenerHandle>		handlesFloat;
		std::vector<Event<FVector2>::ListenerHandle>	handlesFVector2;
		std::vector<Event<IVector2>::ListenerHandle>	handlesIVector2;

	public:
		/// <summary>
		/// When false, none of the callbacks bound through this component is called (set in the Inspector or by code).
		/// </summary>
		Bool		enabled	= true;

		/// <summary>
		/// Constructs an InputComponent attached to the specified actor.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		InputComponent(Actor* _owner);

		/// <summary>
		/// Binds a parameterless callback function to an Event.
		/// </summary>
		/// <param name="_event">Pointer to the Event.</param>
		/// <param name="_callback">Function callback to invoke.</param>
		/// <param name="_once">Whether the callback should be invoked only once.</param>
		void Bind(Event<>* _event, std::function<void()> _callback, const Bool& _once = false);

		/// <summary>
		/// Binds a Float-parameter callback function to an Event.
		/// </summary>
		/// <param name="_event">Pointer to the Event.</param>
		/// <param name="_callback">Function callback to invoke.</param>
		/// <param name="_once">Whether the callback should be invoked only once.</param>
		void Bind(Event<Float>* _event, std::function<void(Float)> _callback, const Bool& _once = false);

		/// <summary>
		/// Binds an FVector2-parameter callback function to an Event.
		/// </summary>
		/// <param name="_event">Pointer to the Event.</param>
		/// <param name="_callback">Function callback to invoke.</param>
		/// <param name="_once">Whether the callback should be invoked only once.</param>
		void Bind(Event<FVector2>* _event, std::function<void(FVector2)> _callback, const Bool& _once = false);

		/// <summary>
		/// Binds an IVector2-parameter callback function to an Event.
		/// </summary>
		/// <param name="_event">Pointer to the Event.</param>
		/// <param name="_callback">Function callback to invoke.</param>
		/// <param name="_once">Whether the callback should be invoked only once.</param>
		void Bind(Event<IVector2>* _event, std::function<void(IVector2)> _callback, const Bool& _once = false);

		/// <summary>
		/// Binds a member function without parameters to an Event.
		/// </summary>
		/// <typeparam name="T">Class type owning the member function.</typeparam>
		/// <typeparam name="MemFn">Member function pointer type.</typeparam>
		/// <param name="_event">Pointer to the Event.</param>
		/// <param name="_instance">Pointer to the object instance.</param>
		/// <param name="_memFn">Pointer to the member function.</param>
		/// <param name="_once">Whether the callback should trigger only once.</param>
		template<typename T, typename MemFn>
		void Bind(Event<>* _event, T* _instance, MemFn _memFn, const Bool& _once = false)
		{
			Bind(_event, [_instance, _memFn]() { std::invoke(_memFn, _instance); }, _once);
		}

		/// <summary>
		/// Binds a member function with a float parameter to an Event.
		/// </summary>
		/// <typeparam name="T">Class type owning the member function.</typeparam>
		/// <typeparam name="MemFn">Member function pointer type.</typeparam>
		/// <param name="_event">Pointer to the Event.</param>
		/// <param name="_instance">Pointer to the object instance.</param>
		/// <param name="_memFn">Pointer to the member function.</param>
		/// <param name="_once">Whether the callback should trigger only once.</param>
		template<typename T, typename MemFn>
		void Bind(Event<float>* _event, T* _instance, MemFn _memFn, const Bool& _once = false)
		{
			Bind(_event, [_instance, _memFn](float _arg) { std::invoke(_memFn, _instance, _arg); }, _once);
		}

		/// <summary>
		/// Binds a member function with an FVector2 parameter to an Event.
		/// </summary>
		/// <typeparam name="T">Class type owning the member function.</typeparam>
		/// <typeparam name="MemFn">Member function pointer type.</typeparam>
		/// <param name="_event">Pointer to the Event.</param>
		/// <param name="_instance">Pointer to the object instance.</param>
		/// <param name="_memFn">Pointer to the member function.</param>
		/// <param name="_once">Whether the callback should trigger only once.</param>
		template<typename T, typename MemFn>
		void Bind(Event<FVector2>* _event, T* _instance, MemFn _memFn, const Bool& _once = false)
		{
			Bind(_event, [_instance, _memFn](FVector2 _arg) { std::invoke(_memFn, _instance, _arg); }, _once);
		}

		/// <summary>
		/// Binds a member function with an IVector2 parameter to an Event.
		/// </summary>
		/// <typeparam name="T">Class type owning the member function.</typeparam>
		/// <typeparam name="MemFn">Member function pointer type.</typeparam>
		/// <param name="_event">Pointer to the Event.</param>
		/// <param name="_instance">Pointer to the object instance.</param>
		/// <param name="_memFn">Pointer to the member function.</param>
		/// <param name="_once">Whether the callback should trigger only once.</param>
		template<typename T, typename MemFn>
		void Bind(Event<IVector2>* _event, T* _instance, MemFn _memFn, const Bool& _once = false)
		{
			Bind(_event, [_instance, _memFn](IVector2 _arg) { std::invoke(_memFn, _instance, _arg); }, _once);
		}

		/// <summary>
		/// Returns a string representation of the component.
		/// </summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;
	};

}
