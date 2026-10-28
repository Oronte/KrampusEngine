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
    /// <param name="key">Field key.</param>
    /// <param name="value">Boolean value.</param>
    void Set(const String& key, const Bool&     value) { fields[key] = value; }

    /// <summary>Sets a Byte integer field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Byte value.</param>
    void Set(const String& key, const Byte&     value) { fields[key] = value.ToLongLong(); }

    /// <summary>Sets a Short integer field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Short value.</param>
    void Set(const String& key, const Short&    value) { fields[key] = value.ToLongLong(); }

    /// <summary>Sets an Int integer field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Int value.</param>
    void Set(const String& key, const Int&      value) { fields[key] = value.ToLongLong(); }

    /// <summary>Sets a Long integer field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Long value.</param>
    void Set(const String& key, const Long&     value) { fields[key] = value.ToLongLong(); }

    /// <summary>Sets a LongLong 64-bit integer field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">LongLong value.</param>
    void Set(const String& key, const LongLong& value) { fields[key] = value; }

    /// <summary>Sets a Char character field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Char value.</param>
    void Set(const String& key, const Char&     value) { fields[key] = LongLong(value.operator int()); }

    /// <summary>Sets a Float floating point field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Float value.</param>
    void Set(const String& key, const Float&      value) { fields[key] = value.ToDouble(); }

    /// <summary>Sets a Double floating point field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Double value.</param>
    void Set(const String& key, const Double&     value) { fields[key] = value; }

    /// <summary>Sets a LongDouble floating point field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">LongDouble value.</param>
    void Set(const String& key, const LongDouble& value) { fields[key] = value.ToDouble(); }

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

    /// <summary>Sets a vector of Int integers field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Vector of Int.</param>
    void Set(const String& key, const std::vector<Int>& value) 
    {
        std::vector<LongLong> out;
        out.reserve(value.size());
        for (const auto& v : value) out.push_back(v.ToLongLong());
        fields[key] = std::move(out);
    }

    /// <summary>Sets a vector of Float numbers field.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="value">Vector of Float.</param>
    void Set(const String& key, const std::vector<Float>& value) 
    {
        std::vector<Double> out;
        out.reserve(value.size());
        for (const auto& v : value) out.push_back(v.ToDouble());
        fields[key] = std::move(out);
    }

    /// <summary>Sets a nested child SaveNode by copy.</summary>
    /// <param name="key">Field key.</param>
    /// <param name="node">SaveNode to nest.</param>
    void Set(const String& key, std::vector<SaveNodePtr> value) { fields[key] = std::move(value); }

    NO_DISCARD std::vector<SaveNodePtr> GetNodes(const String& key) const
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
    NO_DISCARD std::optional<T> TryGet(const String& key) const
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
    NO_DISCARD T Get(const String& key, T defaultValue = T{}) const
    {
        return TryGet<T>(key).value_or(std::move(defaultValue));
    }

    /// <summary>Retrieves a nested child SaveNode if present.</summary>
    /// <param name="key">Field key.</param>
    /// <returns>Optional child SaveNode.</returns>
    NO_DISCARD std::optional<SaveNode> GetNode(const String& key) const
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
    NO_DISCARD std::optional<T> GetAs(const String& key) const
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
    NO_DISCARD std::vector<T> GetArray(const String& key) const
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
    NO_DISCARD Bool   Has(const String& key)  const { return Bool(fields.contains(key)); }

    /// <summary>Removes a field by key.</summary>
    /// <param name="key">Field key to erase.</param>
    void                 Remove(const String& key)     { fields.erase(key); }

    /// <summary>Clears all fields in the node.</summary>
    void                 Clear()                       { fields.clear(); }

    /// <summary>Returns the total number of fields stored in this node.</summary>
    /// <returns>Field count.</returns>
    NO_DISCARD Int    FieldCount()            const { return Int(fields.size()); }

    /// <summary>Checks whether the node contains no fields.</summary>
    /// <returns>True if empty, false otherwise.</returns>
    NO_DISCARD Bool   IsEmpty()               const { return Bool(fields.empty()); }

    /// <summary>Gets the name of the save node.</summary>
    /// <returns>Const reference to node name.</returns>
    NO_DISCARD const  String& GetName()       const { return name; }

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
    NO_DISCARD const auto& GetFields() const { return fields; }

    /// <summary>Gets a mutable reference to the internal fields map.</summary>
    /// <returns>Mutable reference to fields map.</returns>
    NO_DISCARD       auto& GetFields()       { return fields; }

private:
    template<typename T>
    static std::optional<T> CastValue(const SaveValue& val) 
    {
        if constexpr (std::is_same_v<T, Bool>) 
        {
            if (const auto* v = std::get_if<Bool>(&val))     return *v;
            if (const auto* v = std::get_if<LongLong>(&val)) return Bool(v->IsZero().Not());
        }
        else if constexpr (std::is_same_v<T, LongLong>) 
        {
            if (const auto* v = std::get_if<LongLong>(&val)) return *v;
            if (const auto* v = std::get_if<Double>(&val))   return v->ToLongLong();
        }
        else if constexpr (std::is_same_v<T, Int>) 
        {
            if (const auto* v = std::get_if<LongLong>(&val)) return v->ToInt();
            if (const auto* v = std::get_if<Double>(&val))   return v->ToInt();
        }
        else if constexpr (std::is_same_v<T, Long>) 
        {
            if (const auto* v = std::get_if<LongLong>(&val)) return v->ToLong();
            if (const auto* v = std::get_if<Double>(&val))   return v->ToLong();
        }
        else if constexpr (std::is_same_v<T, Short>) 
        {
            if (const auto* v = std::get_if<LongLong>(&val)) return v->ToShort();
        }
        else if constexpr (std::is_same_v<T, Byte>) 
        {
            if (const auto* v = std::get_if<LongLong>(&val)) return v->ToByte();
        }
        else if constexpr (std::is_same_v<T, Double>) 
        {
            if (const auto* v = std::get_if<Double>(&val))   return *v;
            if (const auto* v = std::get_if<LongLong>(&val)) return v->ToDouble();
        }
        else if constexpr (std::is_same_v<T, Float>) 
        {
            if (const auto* v = std::get_if<Double>(&val))   return v->ToFloat();
            if (const auto* v = std::get_if<LongLong>(&val)) return v->ToFloat();
        }
        else if constexpr (std::is_same_v<T, LongDouble>) 
        {
            if (const auto* v = std::get_if<Double>(&val))   return v->ToLongDouble();
            if (const auto* v = std::get_if<LongLong>(&val)) return v->ToLongDouble();
        }
        else if constexpr (std::is_same_v<T, String>) 
        {
            if (const auto* v = std::get_if<String>(&val)) return *v;
        }
        else if constexpr (std::is_same_v<T, std::vector<Byte>>) 
        {
            if (const auto* v = std::get_if<std::vector<Byte>>(&val)) return *v;
        }
        else if constexpr (std::is_same_v<T, std::vector<LongLong>>) 
        {
            if (const auto* v = std::get_if<std::vector<LongLong>>(&val)) return *v;
        }
        else if constexpr (std::is_same_v<T, std::vector<Int>>) 
        {
            if (const auto* v = std::get_if<std::vector<LongLong>>(&val)) 
            {
                std::vector<Int> out;
                out.reserve(v->size());
                for (const auto& x : *v) out.push_back(x.ToInt());
                return out;
            }
        }
        else if constexpr (std::is_same_v<T, std::vector<Double>>) 
        {
            if (const auto* v = std::get_if<std::vector<Double>>(&val)) return *v;
        }
        else if constexpr (std::is_same_v<T, std::vector<Float>>) 
        {
            if (const auto* v = std::get_if<std::vector<Double>>(&val)) 
            {
                std::vector<Float> out;
                out.reserve(v->size());
                for (const auto& x : *v) out.push_back(x.ToFloat());
                return out;
            }
        }
        else if constexpr (std::is_same_v<T, std::vector<String>>) 
        {
            if (const auto* v = std::get_if<std::vector<String>>(&val)) return *v;
        }
        return std::nullopt;
    }
};
