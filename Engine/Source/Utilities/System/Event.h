#pragma once

/*
 * Krampus event system - core
 * ---------------------------------------------------------------------------
 *  Event<Args...>     Multicast event (single-threaded, designed for the game loop).
 *  ListenerHandle     RAII subscription token, move-only, NOT templated: any handle
 *                     can be stored in the same container whatever the event type.
 *  ListenerGroup      Bag of handles: one object owns all the subscriptions of a system.
 *
 *  Features
 *   - Priorities (higher first, FIFO among equals)
 *   - One-shot listeners (fire exactly once, even if the event is re-broadcast from a listener)
 *   - Propagation control: a listener may return Propagation::Stop to cancel the broadcast
 *   - Safe re-entrancy: add / remove / clear / nested Broadcast from inside a listener
 *   - Safe lifetime: a handle can outlive its event, an event can be moved
 *   - Subscribe / unsubscribe in O(1) (generational slots), no heap allocation for small
 *     callbacks, and a Broadcast with nobody listening is a single branch
 *
 *  Usage
 *      Event<int, const std::string&> onScore;
 *
 *      ListenerHandle h = onScore.AddListener([](int _points, const std::string& _who) { ... });
 *      auto h2 = onScore.AddListener(this, &Player::OnScore, { .priority = EventPriority::High });
 *      auto h3 = onScore.AddOnce([] (int, const std::string&) { ... });
 *
 *      ListenerGroup group;                                  // members of a system
 *      onScore.AddListener(group, [](int, const std::string&) { ... });
 *
 *      onScore.Broadcast(10, "Alice");
 *
 *  Notes
 *   - Arguments are forwarded as declared: use Event<const Foo&> for anything heavier than a
 *     few bytes, a value type is copied once per listener.
 *   - Listeners added during a Broadcast start receiving events at the next Broadcast.
 *   - Listeners removed during a Broadcast are not called anymore, even if they were next.
 *   - Do not destroy or move an Event from inside one of its own listeners.
 *   - Not thread-safe. For cross-thread communication use EventBus::Post (see EventBus.h).
 */

#include "Utilities/Debug/Logger.h"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

namespace Krampus
{
    // ------------------------------------------------------------------------------------------
    //  Public vocabulary
    // ------------------------------------------------------------------------------------------

    namespace EventPriority
    {
        inline constexpr std::int32_t Highest   =  1000;
        inline constexpr std::int32_t High      =   500;
        inline constexpr std::int32_t Normal    =     0;
        inline constexpr std::int32_t Low       =  -500;
        inline constexpr std::int32_t Lowest    = -1000;
    }

    /// Returned by a listener to decide if the remaining listeners must still be called.
    enum class Propagation : std::uint8_t
    {
        Continue,
        Stop
    };

    struct ListenerOptions
    {
        std::int32_t    priority    = EventPriority::Normal;
        bool            once        = false;
    };

    /// A callable usable as a listener: invocable with Args..., returning void or Propagation.
    template<typename F, typename... Args>
    concept EventCallable =
        std::invocable<std::decay_t<F>&, Args...> &&
        (std::is_void_v<std::invoke_result_t<std::decay_t<F>&, Args...>> ||
         std::same_as<std::invoke_result_t<std::decay_t<F>&, Args...>, Propagation>);

    /// A member function pointer usable as a listener on an instance of T.
    template<typename MemFn, typename T, typename... Args>
    concept EventMemberCallable =
        std::is_member_pointer_v<MemFn> &&
        std::invocable<MemFn, T*, Args...> &&
        (std::is_void_v<std::invoke_result_t<MemFn, T*, Args...>> ||
         std::same_as<std::invoke_result_t<MemFn, T*, Args...>, Propagation>);

    // ------------------------------------------------------------------------------------------
    //  Implementation details
    // ------------------------------------------------------------------------------------------

    namespace Detail
    {
        inline constexpr std::size_t EventInlineBufferSize = 32;

        /// Move-only type-erased callable with a small inline buffer (no allocation when the
        /// callable fits and is nothrow-move-constructible). Always returns a Propagation.
        template<std::size_t BufferSize, typename... Args>
        class Delegate
        {
            static_assert(BufferSize >= sizeof(void*), "The buffer must at least hold a pointer");

        public:
            Delegate() noexcept = default;

            template<typename F>
            requires (!std::same_as<std::remove_cvref_t<F>, Delegate> && EventCallable<F, Args...>)
            explicit Delegate(F&& _callable)
            {
                using D = std::decay_t<F>;

                if constexpr (StoredInline<D>)
                    ::new (static_cast<void*>(storage)) D(std::forward<F>(_callable));
                else
                    ::new (static_cast<void*>(storage)) D*(new D(std::forward<F>(_callable)));

                vtable = VTableFor<D>();
            }

            Delegate(const Delegate&) = delete;
            Delegate& operator=(const Delegate&) = delete;

            Delegate(Delegate&& _other) noexcept
            {
                MoveFrom(_other);
            }

            Delegate& operator=(Delegate&& _other) noexcept
            {
                if (this != &_other)
                {
                    Reset();
                    MoveFrom(_other);
                }
                return *this;
            }

            ~Delegate()
            {
                Reset();
            }

            explicit operator bool() const noexcept
            {
                return vtable != nullptr;
            }

            void Reset() noexcept
            {
                if (vtable)
                {
                    const VTable* _table = vtable;
                    vtable = nullptr;               // already empty if the destructor of a capture re-enters
                    _table->destroy(storage);
                }
            }

            Propagation Invoke(Args... _args)
            {
                return vtable->invoke(storage, std::forward<Args>(_args)...);
            }

        private:
            struct VTable
            {
                Propagation (*invoke)(void* _storage, Args... _args);
                void        (*relocate)(void* _dst, void* _src) noexcept;
                void        (*destroy)(void* _storage) noexcept;
            };

            template<typename F>
            static constexpr bool StoredInline =
                sizeof(F) <= BufferSize &&
                alignof(F) <= alignof(void*) &&
                std::is_nothrow_move_constructible_v<F>;

            template<typename F>
            static F* Get(void* _storage) noexcept
            {
                if constexpr (StoredInline<F>)
                    return std::launder(static_cast<F*>(_storage));
                else
                    return *std::launder(static_cast<F**>(_storage));
            }

            template<typename F>
            static Propagation InvokeImpl(void* _storage, Args... _args)
            {
                F& _callable = *Get<F>(_storage);

                if constexpr (std::is_void_v<std::invoke_result_t<F&, Args...>>)
                {
                    std::invoke(_callable, std::forward<Args>(_args)...);
                    return Propagation::Continue;
                }
                else
                {
                    return std::invoke(_callable, std::forward<Args>(_args)...);
                }
            }

            template<typename F>
            static void RelocateImpl(void* _dst, void* _src) noexcept
            {
                if constexpr (StoredInline<F>)
                {
                    F* _from = Get<F>(_src);
                    ::new (_dst) F(std::move(*_from));
                    _from->~F();
                }
                else
                {
                    ::new (_dst) F*(Get<F>(_src));
                }
            }

            template<typename F>
            static void DestroyImpl(void* _storage) noexcept
            {
                if constexpr (StoredInline<F>)
                    Get<F>(_storage)->~F();
                else
                    delete Get<F>(_storage);
            }

            template<typename F>
            static const VTable* VTableFor() noexcept
            {
                static constexpr VTable _table{ &InvokeImpl<F>, &RelocateImpl<F>, &DestroyImpl<F> };
                return &_table;
            }

            void MoveFrom(Delegate& _other) noexcept
            {
                vtable = _other.vtable;
                if (vtable)
                {
                    vtable->relocate(storage, _other.storage);
                    _other.vtable = nullptr;
                }
            }

            const VTable*   vtable = nullptr;
            alignas(void*) unsigned char storage[BufferSize];
        };

        /// Shared between an Event and the handles it created. owner == nullptr once the Event is gone.
        struct EventControl
        {
            void* owner = nullptr;
        };

        /// Type-erased access to the Event a handle belongs to (one static instance per Event type).
        struct EventOps
        {
            bool (*remove)(void* _owner, std::uint64_t _id);
            bool (*isActive)(void* _owner, std::uint64_t _id);
        };
    }

    template<typename... Args>
    class Event;

    // ------------------------------------------------------------------------------------------
    //  ListenerHandle
    // ------------------------------------------------------------------------------------------

    /// Owns a subscription: the listener is removed when the handle is destroyed or Disconnect()ed.
    /// Safe to use after the event has been destroyed. Not tied to a specific event type.
    class ListenerHandle
    {
    public:
        using Id = std::uint64_t;

        ListenerHandle() noexcept = default;

        ListenerHandle(const ListenerHandle&) = delete;
        ListenerHandle& operator=(const ListenerHandle&) = delete;

        ListenerHandle(ListenerHandle&& _other) noexcept
            : control(std::move(_other.control)), ops(_other.ops), id(_other.id)
        {
            _other.Release();
        }

        ListenerHandle& operator=(ListenerHandle&& _other) noexcept
        {
            if (this != &_other)
            {
                Disconnect();
                control = std::move(_other.control);
                ops = _other.ops;
                id = _other.id;
                _other.Release();
            }
            return *this;
        }

        ~ListenerHandle()
        {
            Disconnect();
        }

        /// Removes the listener now. Does nothing if already disconnected or if the event is gone.
        void Disconnect() noexcept
        {
            if (control && control->owner && ops)
                ops->remove(control->owner, id);

            Release();
        }

        /// Gives up ownership: the listener stays registered until the event dies (or is cleared).
        void Detach() noexcept
        {
            Release();
        }

        /// True while the listener is registered (false once removed, fired for a one-shot, or event destroyed).
        bool IsConnected() const noexcept
        {
            return control && control->owner && ops && ops->isActive(control->owner, id);
        }

        explicit operator bool() const noexcept
        {
            return IsConnected();
        }

        Id GetId() const noexcept
        {
            return id;
        }

    private:
        template<typename... Args>
        friend class Event;

        ListenerHandle(std::shared_ptr<Detail::EventControl> _control, const Detail::EventOps* _ops, Id _id) noexcept
            : control(std::move(_control)), ops(_ops), id(_id)
        {
        }

        void Release() noexcept
        {
            control.reset();
            ops = nullptr;
            id = 0;
        }

        std::shared_ptr<Detail::EventControl>   control;
        const Detail::EventOps*                 ops = nullptr;
        Id                                      id = 0;
    };

    // ------------------------------------------------------------------------------------------
    //  ListenerGroup
    // ------------------------------------------------------------------------------------------

    /// Owns several subscriptions, possibly on different events. All of them are removed when the
    /// group is destroyed or cleared: perfect as a member of a component / system.
    class ListenerGroup
    {
    public:
        ListenerGroup() = default;
        ListenerGroup(const ListenerGroup&) = delete;
        ListenerGroup& operator=(const ListenerGroup&) = delete;
        ListenerGroup(ListenerGroup&&) noexcept = default;
        ListenerGroup& operator=(ListenerGroup&&) noexcept = default;

        void Add(ListenerHandle&& _handle)
        {
            handles.push_back(std::move(_handle));
        }

        ListenerGroup& operator+=(ListenerHandle&& _handle)
        {
            Add(std::move(_handle));
            return *this;
        }

        void Clear() noexcept
        {
            handles.clear();
        }

        std::size_t Size() const noexcept   { return handles.size(); }
        bool        IsEmpty() const noexcept { return handles.empty(); }

    private:
        std::vector<ListenerHandle> handles;
    };

    // ------------------------------------------------------------------------------------------
    //  Event
    // ------------------------------------------------------------------------------------------

    template<typename... Args>
    class Event : public IPrintable
    {
    public:
        /// (generation << 32) | slot index. 0 is never a valid id. A recycled slot gets a new generation,
        /// so a stale id can never remove a newer listener.
        using ListenerId        = std::uint64_t;
        using ListenerHandle    = Krampus::ListenerHandle;

    private:
        using Callback = Detail::Delegate<Detail::EventInlineBufferSize, Args...>;

        // Listeners live in slots whose address never changes (a callback may be running while others
        // are added). A separate array of slot indices keeps the call order.
        struct Slot
        {
            Callback        callback;
            std::uint32_t   generation  = 1;
            std::int32_t    priority    = 0;
            bool            once        = false;
            bool            alive       = false;    // false: removed, waiting to be released
        };

        static constexpr std::uint32_t  ChunkShift  = 6;
        static constexpr std::uint32_t  ChunkSize   = 1u << ChunkShift;
        static constexpr std::uint32_t  ChunkMask   = ChunkSize - 1;
        static constexpr std::uint32_t  MinGarbage  = 32;

        static bool RemoveThunk(void* _owner, std::uint64_t _id)
        {
            return static_cast<Event*>(_owner)->RemoveListener(_id);
        }

        static bool ActiveThunk(void* _owner, std::uint64_t _id)
        {
            return static_cast<Event*>(_owner)->HasListener(_id);
        }

        static constexpr Detail::EventOps Ops{ &RemoveThunk, &ActiveThunk };

        /// Keeps the structure stable while listeners run; applies deferred work when the outermost broadcast ends.
        struct BroadcastScope
        {
            Event& event;

            explicit BroadcastScope(Event& _event) noexcept : event(_event)
            {
                ++event.depth;
            }

            ~BroadcastScope()
            {
                if (--event.depth == 0 && (event.hasGarbage || !event.pendingOrder.empty()))
                    event.Flush();
            }
        };

        std::vector<std::unique_ptr<Slot[]>>    chunks;             // slot storage, ChunkSize slots per chunk
        std::vector<std::uint32_t>              order;              // slot indices in call order (may hold removed slots)
        std::vector<std::uint32_t>              pendingOrder;       // registered while broadcasting, merged afterwards
        std::vector<std::uint32_t>              freeSlots;
        std::shared_ptr<Detail::EventControl>   control;            // created lazily, only if somebody listens
        std::uint32_t                           slotCount       = 0;
        std::uint32_t                           liveCount       = 0;
        std::uint32_t                           garbageCount    = 0;    // removals since the last compaction
        std::uint32_t                           depth           = 0;    // broadcast nesting level
        bool                                    hasGarbage      = false;
        bool                                    needsSort       = false;    // 'order' is not sorted by priority yet

    public:
        Event() = default;

        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

        Event(Event&& _other) noexcept
            : IPrintable()
        {
            MoveFrom(_other);
        }

        Event& operator=(Event&& _other) noexcept
        {
            if (this != &_other)
            {
                // The previous content moves to a temporary: its handles become inert when it dies, and
                // its callbacks are destroyed once *this is consistent again.
                Event _previous(std::move(*this));
                MoveFrom(_other);
            }
            return *this;
        }

        ~Event()
        {
            assert(depth == 0 && "An Event must not be destroyed from inside one of its own listeners");

            if (control)
                control->owner = nullptr;           // outliving handles become inert
        }

        // ---------------------------------------------------------------- Subscription

        /// Registers any callable taking Args... and returning void or Propagation.
        template<EventCallable<Args...> F>
        NO_DISCARD ListenerHandle AddListener(F&& _callback, const ListenerOptions& _options = {})
        {
            if constexpr (requires { static_cast<bool>(_callback); })
            {
                if (!static_cast<bool>(_callback))
                {
                    LOG_WARNING("A null callback is detected; an invalid ListenerHandle is returned");
                    return ListenerHandle();
                }
            }

            return Register(Callback(std::forward<F>(_callback)), _options);
        }

        /// Registers a member function (const or not) of an instance. The instance must outlive the handle.
        template<typename T, typename MemFn>
        NO_DISCARD ListenerHandle AddListener(T* _instance, MemFn _memFn, const ListenerOptions& _options = {})
            requires EventMemberCallable<MemFn, T, Args...>
        {
            if (!_instance)
            {
                LOG_WARNING("A null instance is detected; an invalid ListenerHandle is returned");
                return ListenerHandle();
            }

            using Result = std::invoke_result_t<MemFn, T*, Args...>;

            return AddListener(
                [_instance, _memFn](Args... _args) -> Result
                {
                    return std::invoke(_memFn, _instance, std::forward<Args>(_args)...);
                },
                _options);
        }

        /// Registers a callable that fires at most once.
        template<EventCallable<Args...> F>
        NO_DISCARD ListenerHandle AddOnce(F&& _callback, std::int32_t _priority = EventPriority::Normal)
        {
            return AddListener(std::forward<F>(_callback), ListenerOptions{ .priority = _priority, .once = true });
        }

        /// Same as AddListener, the subscription is owned by the group.
        template<EventCallable<Args...> F>
        void AddListener(ListenerGroup& _group, F&& _callback, const ListenerOptions& _options = {})
        {
            _group.Add(AddListener(std::forward<F>(_callback), _options));
        }

        template<typename T, typename MemFn>
        void AddListener(ListenerGroup& _group, T* _instance, MemFn _memFn, const ListenerOptions& _options = {})
            requires EventMemberCallable<MemFn, T, Args...>
        {
            _group.Add(AddListener(_instance, _memFn, _options));
        }

        /// Removes a listener by id (prefer ListenerHandle). Returns true if something was removed.
        bool RemoveListener(ListenerId _id) noexcept
        {
            Slot* _slot = FindSlot(_id);

            if (!_slot)
                return false;

            _slot->alive = false;
            --liveCount;
            ++garbageCount;
            hasGarbage = true;

            if (depth == 0)
            {
                // Nobody is running it: release what the callback captured right now. The slot itself
                // is recycled by the next compaction (amortized, so removing n listeners costs O(n)).
                _slot->callback.Reset();

                if (garbageCount > MinGarbage && garbageCount > liveCount)
                    Flush();
            }

            return true;
        }

        /// Removes every listener (callable from inside a listener).
        void Clear() noexcept
        {
            const std::uint32_t _count = slotCount;
            std::uint32_t _removed = 0;

            for (std::uint32_t _i = 0; _i < _count; ++_i)
            {
                Slot& _slot = SlotAt(_i);

                if (_slot.alive)
                {
                    _slot.alive = false;
                    ++_removed;
                }
            }

            liveCount -= _removed;
            garbageCount += _removed;
            hasGarbage = hasGarbage || _removed > 0;

            if (depth == 0 && hasGarbage)
                Flush();
        }

        // ---------------------------------------------------------------- Dispatch

        /// Calls every listener in priority order. Returns true if one of them stopped the propagation.
        bool Broadcast(Args... _args)
        {
            if (depth == 0)
            {
                if (hasGarbage)
                    Flush();

                if (needsSort)
                    SortOrder();
            }

            if (liveCount == 0)
                return false;

            BroadcastScope _scope(*this);

            // 'order' cannot change while depth > 0 (additions are deferred, removals only flag the slot),
            // so indexing up to the initial size is safe even with nested broadcasts.
            const std::size_t _count = order.size();

            for (std::size_t _i = 0; _i < _count; ++_i)
            {
                Slot& _slot = SlotAt(order[_i]);

                if (!_slot.alive)
                    continue;

                if (_slot.once)
                {
                    _slot.alive = false;            // disabled before the call: fires once even if re-entered
                    --liveCount;
                    ++garbageCount;
                    hasGarbage = true;
                }

                if (_slot.callback.Invoke(_args...) == Propagation::Stop)
                    return true;
            }

            return false;
        }

        // ---------------------------------------------------------------- Queries

        bool HasListener(ListenerId _id) const noexcept
        {
            return FindSlot(_id) != nullptr;
        }

        std::size_t ListenerCount() const noexcept
        {
            return liveCount;
        }

        bool HasListeners() const noexcept
        {
            return liveCount > 0;
        }

        bool IsBroadcasting() const noexcept
        {
            return depth > 0;
        }

        virtual std::string ToString() const override
        {
            std::string _types;
            ((_types += (_types.empty() ? "" : ", ") + std::string(typeid(Args).name())), ...);
            return std::format("Event of type(s) {} with {} listener(s)", _types, ListenerCount());
        }

    private:
        Slot& SlotAt(std::uint32_t _index) const noexcept
        {
            return chunks[_index >> ChunkShift][_index & ChunkMask];
        }

        Slot* FindSlot(ListenerId _id) const noexcept
        {
            const std::uint32_t _index = static_cast<std::uint32_t>(_id);
            const std::uint32_t _generation = static_cast<std::uint32_t>(_id >> 32);

            if (_index >= slotCount)
                return nullptr;

            Slot& _slot = SlotAt(_index);
            return (_slot.alive && _slot.generation == _generation) ? &_slot : nullptr;
        }

        std::uint32_t AcquireSlot()
        {
            if (!freeSlots.empty())
            {
                const std::uint32_t _index = freeSlots.back();
                freeSlots.pop_back();
                return _index;
            }

            if ((slotCount & ChunkMask) == 0)
                chunks.push_back(std::make_unique<Slot[]>(ChunkSize));

            return slotCount++;
        }

        /// Destroys the callback and makes the slot reusable (with a new generation).
        void ReleaseSlot(std::uint32_t _index)
        {
            Slot& _slot = SlotAt(_index);

            _slot.alive = false;
            _slot.callback.Reset();                 // may run user code (destructors of captured objects)
            ++_slot.generation;

            if (_slot.generation == 0)
                _slot.generation = 1;

            freeSlots.push_back(_index);
        }

        ListenerHandle Register(Callback&& _callback, const ListenerOptions& _options)
        {
            if (!control)
            {
                control = std::make_shared<Detail::EventControl>();
                control->owner = this;
            }

            const std::uint32_t _index = AcquireSlot();
            Slot& _slot = SlotAt(_index);

            _slot.callback  = std::move(_callback);
            _slot.priority  = _options.priority;
            _slot.once      = _options.once;
            _slot.alive     = true;
            ++liveCount;

            if (depth > 0)
                pendingOrder.push_back(_index);
            else
                AppendToOrder(_index);

            return ListenerHandle(control, &Ops, (static_cast<ListenerId>(_slot.generation) << 32) | _index);
        }

        /// Appends to 'order' in O(1). If the new listener has a higher priority than the last one, the array
        /// is flagged and sorted (stable: priority descending, insertion order among equals) by the next
        /// Broadcast, so registering many listeners with mixed priorities costs one sort, not one shift each.
        void AppendToOrder(std::uint32_t _index)
        {
            if (!needsSort && !order.empty() && SlotAt(order.back()).priority < SlotAt(_index).priority)
                needsSort = true;

            order.push_back(_index);
        }

        void SortOrder()
        {
            needsSort = false;

            std::stable_sort(order.begin(), order.end(),
                [this](std::uint32_t _a, std::uint32_t _b) { return SlotAt(_a).priority > SlotAt(_b).priority; });
        }

        /// Applies the deferred work: releases removed slots, merges the listeners added while broadcasting.
        /// Destroying a callback can run user code (captured objects) that touches this event again:
        /// depth stays > 0 meanwhile (so everything is deferred) and we loop until nothing is left.
        void Flush()
        {
            ++depth;

            while (hasGarbage || !pendingOrder.empty())
            {
                if (hasGarbage)
                {
                    hasGarbage = false;

                    std::size_t _kept = 0;
                    for (std::size_t _i = 0; _i < order.size(); ++_i)
                    {
                        const std::uint32_t _index = order[_i];

                        if (SlotAt(_index).alive)
                            order[_kept++] = _index;
                        else
                            ReleaseSlot(_index);
                    }
                    order.resize(_kept);
                }

                for (std::size_t _i = 0; _i < pendingOrder.size(); ++_i)
                {
                    const std::uint32_t _index = pendingOrder[_i];

                    if (SlotAt(_index).alive)
                        AppendToOrder(_index);
                    else
                        ReleaseSlot(_index);
                }
                pendingOrder.clear();
            }

            garbageCount = 0;
            --depth;
        }

        /// Takes everything from _other (which is left empty). *this must be empty.
        void MoveFrom(Event& _other) noexcept
        {
            assert(_other.depth == 0 && "An Event cannot be moved while it is broadcasting");

            chunks          = std::move(_other.chunks);
            order           = std::move(_other.order);
            pendingOrder    = std::move(_other.pendingOrder);
            freeSlots       = std::move(_other.freeSlots);
            control         = std::move(_other.control);
            slotCount       = _other.slotCount;
            liveCount       = _other.liveCount;
            garbageCount    = _other.garbageCount;
            hasGarbage      = _other.hasGarbage;
            needsSort       = _other.needsSort;
            depth           = 0;

            if (control)
                control->owner = this;

            _other.chunks.clear();
            _other.order.clear();
            _other.pendingOrder.clear();
            _other.freeSlots.clear();
            _other.slotCount    = 0;
            _other.liveCount    = 0;
            _other.garbageCount = 0;
            _other.hasGarbage   = false;
            _other.needsSort    = false;
        }
    };

}