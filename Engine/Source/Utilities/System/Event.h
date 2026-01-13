#pragma once
#include "Utilities/Debug/Logger.h"


namespace Krampus
{

    using ListenerId = unsigned int;

	// Event system that allows adding, removing and broadcasting events with any number of parameters.
    template<typename... Args>
    class Event
    {
        using Callback = std::function<void(Args...)>;

        struct Listener
        {
            ListenerId id = 0;
            void* owner = nullptr;
            Callback callback;
            bool isOnce = false;
            int priority = 0;
        };

        std::vector<Listener> listeners;
        ListenerId nextId = 1;
        
        using Iterator = typename std::vector<Listener>::iterator;

    public:
		// Returns true if there are no listeners registered to the event.
        INLINE bool IsEmpty() const noexcept
        {
            return Count() == 0;
        }

        Event() = default;

        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

        /// <param name="_once"> -> if true, when the next broadcast is called this listener is remove from the event</param>
		/// <returns> The id of the listener added </returns>
        INLINE ListenerId AddListener(Callback _callback, const bool& _once = false, const int& _priority = 0)
        {
            if (!_callback)
            {
                LOG_ERROR("There is no callback for the event");
                return 0;
            }

            ListenerId _id = nextId++;
            Listener _listener;
            _listener.id = _id;
            _listener.callback = std::move(_callback);
            _listener.isOnce = _once;
            _listener.priority = _priority;

            Iterator _iterator = std::upper_bound(
                listeners.begin(), listeners.end(), _listener,
                [](const Listener& _a, const Listener& _b) { return _a.priority > _b.priority; });

            listeners.insert(_iterator, std::move(_listener));
            return _id;
        }

        /// <param name="_once"> -> if true, when the next broadcast is called this listener is remove from the event</param>
		/// <returns> The id of the listener added </returns>
        template<typename T, typename MemFn>
        INLINE ListenerId AddListener(T* _instance, MemFn _memFn,
            const bool& _once = false, const int& _priority = 0)
        {
            if (!_instance)
            {
                LOG_ERROR("The instance for the callback is nullptr");
                return 0;
            }

            Callback _callback = [_instance, _memFn](Args... args)
                {
                    std::invoke(_memFn, _instance, args...);
                };

            ListenerId _id = nextId++;
            Listener _listener;
            _listener.id = _id;
            _listener.owner = _instance;
            _listener.callback = std::move(_callback);
            _listener.isOnce = _once;
            _listener.priority = _priority;

            Iterator _iterator = std::upper_bound(
                listeners.begin(), listeners.end(), _listener,
                [](const Listener& _a, const Listener& _b) { return _a.priority > _b.priority; });

            listeners.insert(_iterator, std::move(_listener));
            return _id;
        }

		// Removes a listener from the event using its id.
        INLINE void RemoveListener(const ListenerId& _id)
        {
            if (_id == 0) return;

            Iterator _iterator = std::find_if(listeners.begin(), listeners.end(),
                [_id](const Listener& _listener) { return _listener.id == _id; });

            if (_iterator != listeners.end()) listeners.erase(_iterator);
            else LOG_WARNING("Incorrect id, cant remove listener");
        }

		// Removes all listeners associated with the given owner pointer.
        INLINE void RemoveAllFrom(void* _owner)
        {
            if (!_owner) return;

            listeners.erase(
                std::remove_if(listeners.begin(), listeners.end(),
                    [_owner](const Listener& _listener)
                    {
                        return _listener.owner == _owner;
                    }),
                listeners.end());
        }

		// Removes all listeners from the event
        INLINE void Clear()
        {
            listeners.clear();
        }

        // Returns the number of registered listeners.
        INLINE size_t Count() const
        {
            return listeners.size();
        }

		// Call the callback of all registered listeners with the provided arguments.
        INLINE void Broadcast(const Args&... _args)
        {
            for (auto _it = listeners.begin(); _it != listeners.end(); )
            {
                _it->callback(_args...);
                if (_it->isOnce) _it = listeners.erase(_it);
                else ++_it;
            }
        }

		// Alias for Broadcast
        INLINE void operator()(const Args&... _args)
        {
            Broadcast(_args...);
        }

		// Alias for AddListener
        INLINE ListenerId operator += (Callback _callback)
        {
            return AddListener(std::move(_callback));
        }

		// Alias for RemoveListener
        INLINE void operator -= (const ListenerId& _toRemove)
        {
            RemoveListener(_toRemove);
        }

		// Alias for RemoveAllfrom
        INLINE void operator -= (void* _owner)
        {
            RemoveAllFrom(_owner);
        }
    };

    //////////////////////////////////////////////////////////////////
    // 
    //  void TestEvent(int _number, char _letter)
    //  {
    //     std::cout << "Number: " << _number << ", Letter: " << _letter << std::endl;
    //  }
    //
    //  class MyClass
    //  {
    //  public:
    //       void TestEvent(int _number, char _letter)
    //      {
    //          std::cout << "Number: " << _number << ", Letter: " << _letter << std::endl;
    //      }
    //  };
    // 
    // 
    //  Event<int, char> _myEvent;
    //
    //  _myEvent.AddListener([](int _number, char _letter)
    //        {
    //         std::cout << "Number: " << _number << ", Letter: " << _letter << std::endl;
    //        });
    //
    //  ListenerId _idToRemove = _myEvent += &TestEvent;
    //  _myEvent -= _idToRemove; // _myEvent.RemoveListener(_idToRemove)
    //
    //  MyClass _myClass;
    //  _myEvent.AddListener(&_myClass, &MyClass::TestEvent);
    //
    //  _myEvent.Broadcast(5, 'A'); // _myEvent(5, 'A);
    // 
    //////////////////////////////////////////////////////////////////
    
    
    template<typename Signature>
    class Delegate;
    
	// Delegate that can hold a single callback with return Type.
    template<typename ReturnType, typename... Args>
    class Delegate<ReturnType(Args...)>
    {
        using Callback = std::function<ReturnType(Args...)>;
        Callback callback;

    public:
		// Replace the callback for the delegate
        void SetCallback(Callback&& _callback)
        {
            if (!_callback)
            {
                LOG(VerbosityType::Error, "There is no callback for the delegate");
                return;
            }

            callback = _callback;
        }

		// Replace the callback for the delegate
        template<typename T, typename MemFn>
        void SetCallback(T* _instance, MemFn _memFn)
        {
            if (!_instance)
            {
                LOG(VerbosityType::Error, "The instance for the callback is nullptr");
                return;
            }

            Callback _callback = [_instance, _memFn](const Args&... args) -> ReturnType
                {
                    return std::invoke(_memFn, _instance, args...);
                };

            SetCallback(std::move(_callback));
        }

		// Remove the callback from the delegate
        void RemoveCallback()
        {
            callback = nullptr;
        }

		// Broadcast the delegate's callback and return the result of the callback
        ReturnType Broadcast(const Args&... _args)
        {
            if (!callback)
            {
                LOG(VerbosityType::Error, "You broadcast a delegate who does not have a callback");
                return ReturnType();
            }
             
            return callback(_args...);
        }

		// Alias for Broadcast
        ReturnType operator()(const Args&... _args)
        {
            return Broadcast(_args...);
        }

		// Alias for SetCallback
        void operator = (Callback&& _callback)
        {
            SetCallback(_callback);
        }

		// Alias for RemoveCallback
        void operator -- ()
        {
            RemoveCallback();
        }
    };

    //////////////////////////////////////////////////////////////////
    // 
    //  int Add(int _x, int _y) { return _x + _y; }
    // 
    //  engine::Delegate<int(int, int)> _firstDelegate;
    //  _firstDelegate.SetCallback(Add);
    // 
    //  _firstDelegate.RemoveCallback();
    //
    //  MyClass _class = MyClass();
    //  _firstDelegate.SetCallback(&_class, &MyClass::Test);
    //  
    //
    //  int _int = _firstDelegate.Broadcast(1, 2);
    // 
    //  engine::Delegate<void()> _delegate; // for Func with no parrams and ne returning type
    // 
    // 
    //////////////////////////////////////////////////////////////////

}
