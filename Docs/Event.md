## `Krampus::Event<Args...>`

### Member Variables

### Constructors

#### `Event()`
- Default constructor.
- Initializes an empty event with no listeners.

#### `Event(const Event&) = delete`
#### `Event& operator=(const Event&) = delete`
- Copy and assignment are disabled.
- Prevents accidental duplication of listener state.

---

### Public Methods

#### `bool IsEmpty() const noexcept`
- Returns `true` if no listeners are registered.

#### `size_t Count() const`
- Returns the number of registered listeners.


#### `ListenerId AddListener(Callback callback, bool once = false, int priority = 0)`
- Registers a free function, lambda, or callable object.

- **callback**: Function invoked on broadcast.
- **once**: If `true`, listener is removed after the next broadcast.
- **priority**: Higher values are executed first.
- Returns the listener identifier, or `0` on failure.


#### `ListenerId AddListener(T* instance, MemFn memFn, bool once = false, int priority = 0)`
- Registers a member function as a listener.

- **`instance`** : Owner object of the callback.
- **`memFn`** : Member function pointer to invoke.
- **`once`** : If `true`, listener is removed after the next broadcast.
- **`priority`**: Execution order relative to other listeners.
- Returns the listener identifier, or `0` on failure.

- Stores the listener identifier for later removal.


#### `void RemoveListener(ListenerId id)`
- Removes a listener by its identifier.
- No effect if `id` is invalid or `0`.


#### `void RemoveAllFrom(void* owner)`
- Removes all listeners associated with a given owner pointer.
- Intended for cleanup before owner destruction.


#### `void Clear()`
- Removes all listeners.


#### `void Broadcast(const Args&... args)`
- Invokes all registered listeners in priority order.
- **`args`**: Forwarded to each callback.
- One-shot listeners are removed after invocation.



### Operators

#### `void operator()(const Args&... args)`
- Convenience alias for `Broadcast`.

#### `ListenerId operator+=(Callback callback)`
- Adds a listener with default options.
- Equivalent to `AddListener(callback)`.

#### `void operator-=(ListenerId id)`
- Removes a listener by identifier.
- Equivalent to `RemoveListener(id)`.

#### `void operator-=(void* owner)`
- Equivalent to `RemoveAllFrom(_owner)`.



### Additional Notes
- Listener removal during broadcast is only supported for `once` listeners.
- Thread safety is not provided.
- Owner lifetime is not managed internally.
