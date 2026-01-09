#pragma once
#include "Utilities/Debug/Logger.h"


namespace Krampus
{

    using ListenerId = unsigned int;

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
        INLINE bool IsEmpty() const noexcept
        {
            return listeners.size() == 0;
        }

        Event() = default;
        ~Event()
        {
            Clear();
        }

        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

        INLINE ListenerId AddListener(Callback _callback, const bool& _once = false, const int& _priority = 0)
        {
            if (!_callback)
            {
                LOG(VerbosityType::Error, "There is no callback for the event");
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

        template<typename T, typename MemFn>
        INLINE ListenerId AddListener(T* _instance, MemFn _memFn,
            const bool& _once = false, const int& _priority = 0)
        {
            if (!_instance)
            {
                LOG(VerbosityType::Error, "The instance for the callback is nullptr");
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

        INLINE void RemoveListener(const ListenerId& _id)
        {
            if (_id == 0) return;

            Iterator _iterator = std::find_if(listeners.begin(), listeners.end(),
                [_id](const Listener& _listener) { return _listener.id == _id; });

            if (_iterator != listeners.end()) listeners.erase(_iterator);
            else LOG(VerbosityType::Warning, "Incorrect id, cant remove listener");
        }

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

        INLINE void Clear()
        {
            for (auto& _listener : listeners)
                _listener.callback = nullptr;
            listeners.clear();
        }

        INLINE size_t Count() const
        {
            return listeners.size();
        }

        INLINE void Broadcast(const Args&... _args)
        {
            for (auto _it = listeners.begin(); _it != listeners.end(); )
            {
                _it->callback(_args...);
                if (_it->isOnce) _it = listeners.erase(_it);
                else ++_it;
            }
        }

        INLINE void operator()(const Args&... _args)
        {
            Broadcast(_args...);
        }

        INLINE void operator += (Callback _callback)
        {
            AddListener(std::move(_callback));
        }

        INLINE void operator -= (const ListenerId& _toRemove)
        {
            RemoveListener(_toRemove);
        }
    };

    //////////////////////////////////////////////////////////////////
    // 
    //  void Add(int _x, int _y) { std::cout << _x + _y; }
    // 
    //  engine::Event<int, int> _firstEvent;
    //  _firstEvent.AddListener(Add);
    //
    //  MyClass _class = MyClass();
    //  _firstEvent.AddListener(&_class, &MyClass::Test);
    // 
    //  const unsigned long long& _callbackId = 0;
    //  _callbackId = _firstEvent.AddListener(ToRemove);
    //  _firstEvent.RemoveListener(_callbackId);
    //
    //  _firstEvent.Broadcast(1, 2);
    // 
    //  engine::Event<> _event; // for Func with no parrams and ne returning type
    // 
    //////////////////////////////////////////////////////////////////
    
    
    template<typename Signature>
    class Delegate;
    
    template<typename ReturnType, typename... Args>
    class Delegate<ReturnType(Args...)>
    {
        using Callback = std::function<ReturnType(Args...)>;
        Callback callback;

    public:
        void SetCallback(Callback&& _callback)
        {
            if (!_callback)
            {
                LOG(VerbosityType::Error, "There is no callback for the delegate");
                return;
            }

            callback = _callback;
        }
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

        void RemoveCallback()
        {
            callback = nullptr;
        }

        ReturnType Broadcast(const Args&... _args)
        {
            if (!callback)
            {
                LOG(VerbosityType::Error, "You broadcast a delegate who does not have a callback");
                return ReturnType();
            }
             
            return callback(_args...);
        }

        ReturnType operator()(const Args&... _args)
        {
            return Broadcast(_args...);
        }

        void operator = (Callback&& _callback)
        {
            SetCallback(_callback);
        }

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
