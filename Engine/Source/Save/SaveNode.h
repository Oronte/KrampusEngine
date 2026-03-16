#pragma once

#include "Utilities/Macro.h"

class SaveNode;
using SaveNodePtr = std::shared_ptr<SaveNode>;

template<typename T>
concept Serializable = requires(const T _ct, T _t, SaveNode& _node, const SaveNode& _cnode) 
{
    { _ct.Save(_node)  } -> std::same_as<void>;
    { _t.Load(_cnode)  } -> std::same_as<void>;
};

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

class SaveNode 
{
    String                                  name;
    std::unordered_map<String, SaveValue>   fields;

public:
    SaveNode() = default;
    SaveNode(String _name) : name(std::move(_name)) {}

    void Set(const String& key, const Bool&     value) { fields[key] = value; }
    void Set(const String& key, const Byte&     value) { fields[key] = value.ToLongLong(); }
    void Set(const String& key, const Short&    value) { fields[key] = value.ToLongLong(); }
    void Set(const String& key, const Int&      value) { fields[key] = value.ToLongLong(); }
    void Set(const String& key, const Long&     value) { fields[key] = value.ToLongLong(); }
    void Set(const String& key, const LongLong& value) { fields[key] = value; }
    void Set(const String& key, const Char&     value) { fields[key] = LongLong(value.operator int()); }

    void Set(const String& key, const Float&      value) { fields[key] = value.ToDouble(); }
    void Set(const String& key, const Double&     value) { fields[key] = value; }
    void Set(const String& key, const LongDouble& value) { fields[key] = value.ToDouble(); }

    void Set(const String& key, const String& value) { fields[key] = value; }
    void Set(const String& key, String&&      value) { fields[key] = std::move(value); }

    void Set(const String& key, std::vector<Byte>     value) { fields[key] = std::move(value); }
    void Set(const String& key, std::vector<LongLong> value) { fields[key] = std::move(value); }
    void Set(const String& key, std::vector<Double>   value) { fields[key] = std::move(value); }
    void Set(const String& key, std::vector<String>   value) { fields[key] = std::move(value); }

    void Set(const String& key, const std::vector<Int>& value) 
    {
        std::vector<LongLong> out;
        out.reserve(value.size());
        for (const auto& v : value) out.push_back(v.ToLongLong());
        fields[key] = std::move(out);
    }
    void Set(const String& key, const std::vector<Float>& value) 
    {
        std::vector<Double> out;
        out.reserve(value.size());
        for (const auto& v : value) out.push_back(v.ToDouble());
        fields[key] = std::move(out);
    }

    void Set(const String& key, const SaveNode& node) { fields[key] = std::make_shared<SaveNode>(node); }
    void Set(const String& key, SaveNode&&      node) { fields[key] = std::make_shared<SaveNode>(std::move(node)); }
    void Set(const String& key, SaveNodePtr     node) { fields[key] = std::move(node); }

    template<Serializable T>
    void Set(const String& key, const T& obj) 
    {
        SaveNode child(key);
        obj.Save(child);
        Set(key, std::move(child));
    }

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

    template<typename T>
    NO_DISCARD std::optional<T> TryGet(const String& key) const
    {
        auto it = fields.find(key);
        if (it == fields.end()) return std::nullopt;
        return CastValue<T>(it->second);
    }

    template<typename T>
    NO_DISCARD T Get(const String& key, T defaultValue = T{}) const
    {
        return TryGet<T>(key).value_or(std::move(defaultValue));
    }

    NO_DISCARD std::optional<SaveNode> GetNode(const String& key) const
    {
        auto it = fields.find(key);
        if (it == fields.end()) return std::nullopt;
        if (const auto* ptr = std::get_if<SaveNodePtr>(&it->second))
            if (*ptr) return **ptr;
        return std::nullopt;
    }

    template<Serializable T>
    NO_DISCARD std::optional<T> GetAs(const String& key) const
    {
        auto nodeOpt = GetNode(key);
        if (!nodeOpt) return std::nullopt;
        T obj{};
        obj.Load(*nodeOpt);
        return obj;
    }

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

    NO_DISCARD Bool   Has(const String& key)  const { return Bool(fields.contains(key)); }
    void                 Remove(const String& key)     { fields.erase(key); }
    void                 Clear()                       { fields.clear(); }
    NO_DISCARD Int    FieldCount()            const { return Int(fields.size()); }
    NO_DISCARD Bool   IsEmpty()               const { return Bool(fields.empty()); }
    NO_DISCARD const  String& GetName()       const { return name; }
    void                 SetName(String name)          { name = std::move(name); }

    void ForEach(const std::function<void(const String&, const SaveValue&)>& fn) const
    {
        for (const auto& [k, v] : fields) fn(k, v);
    }

    NO_DISCARD const auto& GetFields() const { return fields; }
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
