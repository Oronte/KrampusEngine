#pragma once
#include "Component.h"

namespace Krampus
{

	class InputComponent : public Component
	{
		std::vector<Event<>::ListenerHandle> handlesVoid;
		std::vector<Event<float>::ListenerHandle> handlesFloat;
		std::vector<Event<FVector2>::ListenerHandle> handlesFVector2;
		std::vector<Event<IVector2>::ListenerHandle> handlesIVector2;

	public:
		InputComponent(Actor* _owner);

		void Bind(Event<>* _event, std::function<void()> _callback, bool _once = false);
		void Bind(Event<float>* _event, std::function<void(float)> _callback, bool _once = false);
		void Bind(Event<FVector2>* _event, std::function<void(FVector2)> _callback, bool _once = false);
		void Bind(Event<IVector2>* _event, std::function<void(IVector2)> _callback, bool _once = false);

		template<typename T, typename MemFn>
		void Bind(Event<>* _event, T* _instance, MemFn _memFn, bool _once = false)
		{
			Bind(_event, [_instance, _memFn]() { std::invoke(_memFn, _instance); }, _once);
		}
		template<typename T, typename MemFn>
		void Bind(Event<float>* _event, T* _instance, MemFn _memFn, bool _once = false)
		{
			Bind(_event, [_instance, _memFn](float _arg) { std::invoke(_memFn, _instance, _arg); }, _once);
		}
		template<typename T, typename MemFn>
		void Bind(Event<FVector2>* _event, T* _instance, MemFn _memFn, bool _once = false)
		{
			Bind(_event, [_instance, _memFn](FVector2 _arg) { std::invoke(_memFn, _instance, _arg); }, _once);
		}
		template<typename T, typename MemFn>
		void Bind(Event<IVector2>* _event, T* _instance, MemFn _memFn, bool _once = false)
		{
			Bind(_event, [_instance, _memFn](IVector2 _arg) { std::invoke(_memFn, _instance, _arg); }, _once);
		}
	};

}

