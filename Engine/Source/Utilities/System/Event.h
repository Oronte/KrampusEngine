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
            std::shared_ptr<bool> ownerAlive;
            Event* owner = nullptr;
            ListenerId      id = 0;


            ListenerHandle() = default;
            ListenerHandle(Event* _event, ListenerId _id, std::shared_ptr<bool> _alive) : id(_id), owner(_event), ownerAlive(_alive) {}

            ListenerHandle(const ListenerHandle&) = delete;
            ListenerHandle& operator=(const ListenerHandle&) = delete;

            ListenerHandle(ListenerHandle&& _other) noexcept
                : owner(_other.owner), id(_other.id), ownerAlive(_other.ownerAlive)
            {
                _other.owner = nullptr;
                _other.id = 0;
            }

            ListenerHandle& operator=(ListenerHandle&& _other) noexcept
            {
                if (this != &_other)
                {
                    Reset();
                    owner = _other.owner;
                    id = _other.id;
                    ownerAlive = _other.ownerAlive;
                    _other.owner = nullptr;
                    _other.id = 0;
                    _other.ownerAlive = nullptr;
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
                if (!owner || !ownerAlive.get() || !*ownerAlive.get()) return;

                owner->RemoveListener(id);
                owner = nullptr;
                id = 0;
            }
        };

    private:

        using Callback = std::function<void(Args...)>;

        struct Listener
        {
            ListenerId  id;
            Callback    callback;
            bool        once;
        };

        std::vector<Listener>       listeners;
        std::vector<ListenerId>     pendingRemove;
        std::vector<Listener>       pendingAdd;

        std::shared_ptr<bool>       isAlive = nullptr;

        ListenerId                  nextId = 1;
        bool                        broadcasting = false;

    public:
        Event()
        {
            isAlive = std::make_shared<bool>(true);
        }
        ~Event()
        {
            *isAlive = false;
        }

        NO_DISCARD
            ListenerHandle AddListener(Callback _callback, bool _once = false)
        {
            if (!_callback)
            {
                LOG_WARNING("A null callback is detected; a potentially invalid ListenerHandle is returned");
                return ListenerHandle();
            }

            const ListenerId _id = nextId++; // TODO security

            Listener _listener{ _id, std::move(_callback), _once };

            broadcasting ? pendingAdd.push_back(std::move(_listener)) :
                listeners.push_back(std::move(_listener));

            return ListenerHandle(this, _id, isAlive);
        }

        template<typename T, typename MemFn>
        NO_DISCARD
            ListenerHandle AddListener(T* _instance, MemFn _memFn, bool _once = false)
        {
            if (!_instance)
            {
                LOG_WARNING("A null instance is detected; a potentially invalid ListenerHandle is returned");
                return ListenerHandle();
            }

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
                {
                    const Callback& _callback = _listener.callback;
                    if (!_callback)
                    {
                        LOG_WARNING("A null callback is detected");
                        continue;
                    }
                    _callback(_args...);
                }
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
