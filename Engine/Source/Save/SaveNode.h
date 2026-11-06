#pragma once

#include "Utilities/Macro.h"

class SaveNode;

/// <summary>
/// Shared pointer alias for hierarchical SaveNode instances.
/// </summary>
using SaveNodePtr = std::shared_ptr<SaveNode>;

/// <summary>
/// C++20 concept requiring a type to provide Save(SaveNode&) and Load(const SaveNode&) methods.
/// </summary>
template<typename T>
concept Serializable = requires(const T _ct, T _t, SaveNode& _node, const SaveNode& _cnode) 
{
    { _ct.Save(_node)  } -> std::same_as<void>;
    { _t.Load(_cnode)  } -> std::same_as<void>;
};

/// <summary>
/// Variant type storing any supported save field value (primitives, strings, byte buffers, arrays, nested nodes).
/// </summary>
using SaveValue = std::variant
<
    Bool,
    LongLong,
    Double,
    String,
    std::vector<Byte>,
    std::vector<LongLong>,
    std::vector<Double>,
    std::vector<String>,
    SaveNodePtr,
    std::vector<SaveNodePtr>
>;

/// <summary>
/// Binary serialization type tags for encoded save values.
/// </summary>
enum class SaveType : unsigned char 
{
    Bool            = 0,
    LongLong        = 1,
    Double          = 2,
    String          = 3,
    Bytes           = 4,
    ArrayLongLong   = 5,
    ArrayDouble     = 6,
    ArrayString     = 7,
    Node            = 8,
    ArrayNode       = 9,
};

/// <summary>
/// Hierarchical key-value node used for saving, loading, and serializing game data trees.
/// </summary>
class SaveNode 
{
    String                                  name;
    std::unordered_map<String, SaveValue>   fields;

public:
    /// <summary>Default constructor creating an unnamed SaveNode.</summary>
    SaveNode() = default;

    /// <summary>Constructs a SaveNode with a specified node name.</summary>
    /// <param name="_name">Node name identifier.</param>
    SaveNode(String _name) : name(std::move(_name)) {}

    /// <summary>Sets a Bool field.</summary>
    void Set(const String& key, Bool value) { fields[key] = value; }

    /// <summary>Sets a numeric field: integers are stored as LongLong, floating points as Double.</summary>
    template<typename T> requires (std::is_arithmetic_v<T> && !std::is_same_v<T, Bool>)
    void Set(const String& key, T value)
    {
        if constexpr (std::is_integral_v<T>) fields[key] = static_cast<LongLong>(value);
        else                                 fields[key] = static_cast<Double>(value);
    }

    /// <summary>Sets a String field by copy.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">String value.</param>
    void Set(const String& key, const String& value) { fields[key] = value; }

    /// <summary>Sets a String field by move.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">String rvalue.</param>
    void Set(const String& key, String&&      value) { fields[key] = std::move(value); }

    /// <summary>Sets a binary raw byte buffer field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Vector of bytes.</param>
    void Set(const String& key, std::vector<Byte>     value) { fields[key] = std::move(value); }

    /// <summary>Sets a vector of LongLong integers field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Vector of LongLong.</param>
    void Set(const String& key, std::vector<LongLong> value) { fields[key] = std::move(value); }

    /// <summary>Sets a vector of Double numbers field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Vector of Double.</param>
    void Set(const String& key, std::vector<Double>   value) { fields[key] = std::move(value); }

    /// <summary>Sets a vector of String field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Vector of String.</param>
    void Set(const String& key, std::vector<String>   value) { fields[key] = std::move(value); }

    /// <summary>Sets a vector of Int field (stored as LongLong).</summary>
    void Set(const String& key, const std::vector<Int>& value) { fields[key] = ConvertAll<LongLong>(value); }

    /// <summary>Sets a vector of Float field (stored as Double).</summary>
    void Set(const String& key, const std::vector<Float>& value) { fields[key] = ConvertAll<Double>(value); }

    /// <summary>Sets a nested child SaveNode by copy.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="node">SaveNode to nest.</param>
    void Set(const String& key, std::vector<SaveNodePtr> value) { fields[key] = std::move(value); }

    [[nodiscard]] std::vector<SaveNodePtr> GetNodes(const String& key) const
    {
        auto it = fields.find(key);
        if (it == fields.end()) return {};
        if (const auto* arr = std::get_if<std::vector<SaveNodePtr>>(&it->second)) return *arr;
        return {};
    }

    void Set(const String& key, const SaveNode& node) { fields[key] = std::make_shared<SaveNode>(node); }

    /// <summary>Sets a nested child SaveNode by move.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="node">SaveNode rvalue.</param>
    void Set(const String& key, SaveNode&&      node) { fields[key] = std::make_shared<SaveNode>(std::move(node)); }

    /// <summary>Sets a nested child SaveNode by shared pointer.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="node">Shared pointer to SaveNode.</param>
    void Set(const String& key, SaveNodePtr     node) { fields[key] = std::move(node); }

    /// <summary>Serializes an object satisfying the Serializable concept into a nested node.</summary>
    /// <typeparam name="T">Serializable type.</typeparam>
    /// <param name="key">Field key.</param>
    /// <param name="obj">Object to serialize.</param>
    template<Serializable T>
    void Set(const String& key, const T& obj) 
    {
        SaveNode child(key);
        obj.Save(child);
        Set(key, std::move(child));
    }

    /// <summary>Serializes an array of objects satisfying the Serializable concept.</summary>
    /// <typeparam name="T">Serializable type.</typeparam>
    /// <param name="key">Field key.</param>
    /// <param name="vec">Vector of objects to serialize.</param>
    template<Serializable T>
    void SetArray(const String& key, const std::vector<T>& vec) 
    {
        std::vector<SaveNodePtr> nodes;
        nodes.reserve(vec.size());
        for (const auto& item : vec) {
            auto node = std::make_shared<SaveNode>();
            item.Save(*node);
            nodes.push_back(std::move(node));
        }
        fields[key] = std::move(nodes);
    }

    /// <summary>Attempts to read a field value cast to the requested type T.</summary>
    /// <typeparam name="T">Expected value type.</typeparam>
    /// <param name="key">Field key.</param>
    /// <returns>Optional containing value if found and convertible.</returns>
    template<typename T>
    [[nodiscard]] std::optional<T> TryGet(const String& key) const
    {
        auto it = fields.find(key);
        if (it == fields.end()) return std::nullopt;
        return CastValue<T>(it->second);
    }

    /// <summary>Reads a field value or returns a default fallback.</summary>
    /// <typeparam name="T">Expected value type.</typeparam>
    /// <param name="key">Field key.</param>
    /// <param name="defaultValue">Default value returned if key is missing.</param>
    /// <returns>Field value or defaultValue.</returns>
    template<typename T>
    [[nodiscard]] T Get(const String& key, T defaultValue = T{}) const
    {
        return TryGet<T>(key).value_or(std::move(defaultValue));
    }

    /// <summary>Retrieves a nested child SaveNode if present.</summary>
    /// <param name="key">Field key.</param>
    /// <returns>Optional child SaveNode.</returns>
    [[nodiscard]] std::optional<SaveNode> GetNode(const String& key) const
    {
        auto it = fields.find(key);
        if (it == fields.end()) return std::nullopt;
        if (const auto* ptr = std::get_if<SaveNodePtr>(&it->second))
            if (*ptr) return **ptr;
        return std::nullopt;
    }

    /// <summary>Deserializes a Serializable object from a nested child node.</summary>
    /// <typeparam name="T">Serializable type.</typeparam>
    /// <param name="key">Field key.</param>
    /// <returns>Optional deserialized object.</returns>
    template<Serializable T>
    [[nodiscard]] std::optional<T> GetAs(const String& key) const
    {
        auto nodeOpt = GetNode(key);
        if (!nodeOpt) return std::nullopt;
        T obj{};
        obj.Load(*nodeOpt);
        return obj;
    }

    /// <summary>Deserializes an array of Serializable objects from a nested array node.</summary>
    /// <typeparam name="T">Serializable type.</typeparam>
    /// <param name="key">Field key.</param>
    /// <returns>Vector of deserialized objects.</returns>
    template<Serializable T>
    [[nodiscard]] std::vector<T> GetArray(const String& key) const
    {
        auto it = fields.find(key);
        if (it == fields.end()) return {};
        const auto* arr = std::get_if<std::vector<SaveNodePtr>>(&it->second);
        if (!arr) return {};
        std::vector<T> result;
        result.reserve(arr->size());
        for (const auto& nodePtr : *arr) {
            if (nodePtr) {
                T obj{};
                obj.Load(*nodePtr);
                result.push_back(std::move(obj));
            }
        }
        return result;
    }

    /// <summary>Checks whether a field with the specified key exists.</summary>
    /// <param name="key">Field key.</param>
    /// <returns>True if key exists, false otherwise.</returns>
    [[nodiscard]] Bool   Has(const String& key)  const { return fields.contains(key); }

    /// <summary>Removes a field by key.</summary>
    /// <param name="key">Field key to erase.</param>
    void                 Remove(const String& key)     { fields.erase(key); }

    /// <summary>Clears all fields in the node.</summary>
    void                 Clear()                       { fields.clear(); }

    /// <summary>Returns the total number of fields stored in this node.</summary>
    /// <returns>Field count.</returns>
    [[nodiscard]] Int    FieldCount()            const { return Int(fields.size()); }

    /// <summary>Checks whether the node contains no fields.</summary>
    /// <returns>True if empty, false otherwise.</returns>
    [[nodiscard]] Bool   IsEmpty()               const { return fields.empty(); }

    /// <summary>Gets the name of the save node.</summary>
    /// <returns>Const reference to node name.</returns>
    [[nodiscard]] const  String& GetName()       const { return name; }

    /// <summary>Sets the name of the save node.</summary>
    /// <param name="_name">New node name.</param>
    void                 SetName(String _name)          { name = std::move(_name); }

    /// <summary>Iterates over all key-value fields in this node.</summary>
    /// <param name="fn">Callback invoked for each key and value.</param>
    void ForEach(const std::function<void(const String&, const SaveValue&)>& fn) const
    {
        for (const auto& [k, v] : fields) fn(k, v);
    }

    /// <summary>Gets a const reference to the internal fields map.</summary>
    /// <returns>Const reference to fields map.</returns>
    [[nodiscard]] const auto& GetFields() const { return fields; }

    /// <summary>Gets a mutable reference to the internal fields map.</summary>
    /// <returns>Mutable reference to fields map.</returns>
    [[nodiscard]]       auto& GetFields()       { return fields; }

private:
    template<typename To, typename From>
    static std::vector<To> ConvertAll(const std::vector<From>& _values)
    {
        std::vector<To> _out;
        _out.reserve(_values.size());
        for (const From& _value : _values) _out.push_back(static_cast<To>(_value));
        return _out;
    }

    /// Numbers convert between the stored LongLong/Double; any other type must match the stored alternative.
    template<typename T>
    static std::optional<T> CastValue(const SaveValue& val)
    {
        if constexpr (std::is_same_v<T, Bool>)
        {
            if (const auto* v = std::get_if<Bool>(&val))     return *v;
            if (const auto* v = std::get_if<LongLong>(&val)) return *v != 0;
        }
        else if constexpr (std::is_arithmetic_v<T>)
        {
            if (const auto* v = std::get_if<LongLong>(&val)) return static_cast<T>(*v);
            if (const auto* v = std::get_if<Double>(&val))   return static_cast<T>(*v);
        }
        else if constexpr (std::is_same_v<T, std::vector<Int>>)
        {
            if (const auto* v = std::get_if<std::vector<LongLong>>(&val)) return ConvertAll<Int>(*v);
        }
        else if constexpr (std::is_same_v<T, std::vector<Float>>)
        {
            if (const auto* v = std::get_if<std::vector<Double>>(&val)) return ConvertAll<Float>(*v);
        }
        else if (const auto* v = std::get_if<T>(&val)) return *v;
        return std::nullopt;
    }
};
