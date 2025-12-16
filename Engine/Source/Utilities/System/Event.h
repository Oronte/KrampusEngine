#pragma once
#include "Utilities/Debug/Logger.h"


namespace Krampus
{

    template<typename... Args>
    class Event
    {
        using Callback = std::function<void(Args...)>;
        using ListenerId = unsigned int;

        struct Listener
        {
            ListenerId id = 0;
            Callback callback;
            bool isOnce = false;
            bool isActive = true;
            int priority = 0;
        };

        std::vector<Listener> listeners;
        ListenerId nextId = 1;
        bool needsCleanup = false;
        
        using Iterator = typename std::vector<Listener>::iterator;

    public:
        Event() = default;

        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

        ListenerId AddListener(const Callback& _callback, const bool& _once = false, const int& _priority = 0)
        {
            if (!_callback)
            {
                LOG(VerbosityType::Error, "There is no callback for the event");
                return 0;
            }

            ListenerId _id = nextId++;
            Listener _listener;
            _listener.id = _id;
            _listener.callback = _callback;
            _listener.isOnce = _once;
            _listener.priority = _priority;

            Iterator _iterator = std::upper_bound(
                listeners.begin(), listeners.end(), _listener,
                [](const Listener& _a, const Listener& _b) { return _a.priority > _b.priority; });

            listeners.insert(_iterator, std::move(_listener));
            return _id;
        }

        ListenerId AddListener(Callback&& _callback, const bool& _once = false, const int& _priority = 0)
        {
            if (!_callback)
            {
                LOG(VerbosityType::Error, "There is no callback for the event");
                return 0;
            }

            ListenerId _id = nextId++;
            Listener _listener;
            _listener.id = _id;
            _listener.callback = _callback;
            _listener.isOnce = _once;
            _listener.priority = _priority;

            Iterator _iterator = std::upper_bound(
                listeners.begin(), listeners.end(), _listener,
                [](const Listener& a, const Listener& b) { return a.priority > b.priority; });

            listeners.insert(_iterator, std::move(_listener));
            return _id;
        }

        template<typename T, typename MemFn>
        ListenerId AddListener(const T* _instance, const MemFn& _memFn,
            const bool& once = false, const int& priority = 0)
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

            return AddListener(std::move(_callback), once, priority);
        }

        void RemoveListener(const ListenerId& _id)
        {
            if (_id == 0)
            {
                LOG(VerbosityType::Error, "Incorrect id, cant be 0");
                return;
            }

            Iterator _iterator = std::find_if(listeners.begin(), listeners.end(),
                [_id](const Listener& _listener) { return _listener.id == _id; });

            if (_iterator != listeners.end())
            {
                _iterator->isActive = false;
                needsCleanup = true;
            }
            else 
                LOG(VerbosityType::Error, "Incorrect id, cant remove listener");
        }

        void Clear()
        {
            listeners.clear();
            needsCleanup = false;
        }

        size_t Count() const
        {
            size_t _count = 0;
            for (const Listener& _listener : listeners) if (_listener.isActive) ++_count;
            return _count;
        }

        void Broadcast(const Args&... _args)
        {
            for (Listener& _listener : listeners)
            {
                if (!_listener.isActive) continue;
                _listener.callback(_args...);
                if (_listener.isOnce)
                {
                    _listener.isActive = false;
                    needsCleanup = true;
                }
            }

            CleanupIfNeeded();
        }

        void operator()(const Args&... _args)
        {
            Broadcast(_args...);
        }

        void operator += (const Callback& _callback)
        {
            AddListener(_callback);
        }

        void operator += (Callback&& _callback)
        {
            AddListener(_callback);
        }

        void operator -= (const ListenerId& _toRemove)
        {
            RemoveListener(_toRemove);
        }

    private:
        void CleanupIfNeeded()
        {
            if (!needsCleanup) return;

            listeners.erase(std::remove_if(listeners.begin(), listeners.end(),
                [](const Listener& _listener) { return !_listener.isActive; }), listeners.end());

            needsCleanup = false;
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
        void SetCallback(const T* _instance, const MemFn& _memFn)
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
