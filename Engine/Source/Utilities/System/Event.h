#pragma once
#include "Utilities/Debug/Logger.h"


namespace Krampus
{

    template<typename... Args>
    class Event
    {
    public:
        using ListenerId = uint32_t;

        struct ListenerHandle
        {
            Event* owner = nullptr;
            ListenerId id = 0;

            ListenerHandle() = default;
            ListenerHandle(Event* _event, ListenerId _id) : id(_id), owner(_event) {}

            ListenerHandle(const ListenerHandle&) = delete;
            ListenerHandle& operator=(const ListenerHandle&) = delete;

            ListenerHandle(ListenerHandle&& _other) noexcept
            {
                *this = std::move(_other);
            }

            ListenerHandle& operator=(ListenerHandle&& _other) noexcept
            {
                if (this != &_other)
                {
                    Reset();
                    id = _other.id;
                    owner = _other.owner;
                    _other.id = 0;
                    _other.owner = nullptr;
                }
                return *this;
            }

            ~ListenerHandle()
            {
                Reset();
            }

            explicit operator bool() const noexcept
            {
                return owner;
            }

        private:
            void Reset()
            {
                if (!owner) return;

                owner->RemoveListener(id);
                owner = nullptr;
                id = 0;
            }
        };

    private:

        using Callback = std::function<void(Args...)>;

        struct Listener
        {
            ListenerId id;
            Callback callback;
            bool once;
        };

        std::vector<Listener> listeners;
        std::vector<ListenerId> pendingRemove;
        std::vector<Listener> pendingAdd;
        ListenerId nextId = 1;
        bool broadcasting = false;

    public:
        NO_DISCARD
        ListenerHandle AddListener(Callback _callback, bool _once = false)
        {
            const ListenerId _id = nextId++;

            Listener _listener{ _id, _callback, _once };

            broadcasting ? pendingAdd.push_back(_listener) :
                listeners.push_back(_listener);

            return ListenerHandle(this, _id);
        }

        template<typename T, typename MemFn>
        NO_DISCARD
        ListenerHandle AddListener(
            T* _instance,
            MemFn _memFn,
            bool _once = false
        )
        {
            return AddListener(
                [_instance, _memFn](Args... _args)
                {
                    std::invoke(_memFn, _instance, _args...);
                },
                _once
            );
        }

        void RemoveListener(ListenerId _id)
        {
            if (broadcasting)
            {
                pendingRemove.push_back(_id);
                return;
            }

            std::erase_if(listeners,
                [_id](const Listener& _listener) { return _listener.id == _id; });
        }

        void Broadcast(Args... _args) noexcept
        {
            broadcasting = true;

            for (Listener& _listener : listeners)
            {
                if (std::ranges::find(pendingRemove, _listener.id) == pendingRemove.end())
                    _listener.callback(_args...);
            }

            broadcasting = false;

            std::erase_if(listeners,
                [&](const Listener& _listener)
                {
                    return _listener.once ||
                        std::ranges::find(pendingRemove, _listener.id) != pendingRemove.end();
                });

            pendingRemove.clear();

            listeners.insert(listeners.end(), pendingAdd.begin(), pendingAdd.end());

            pendingAdd.clear();
        }
    };

}
