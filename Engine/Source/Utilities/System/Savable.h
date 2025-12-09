#pragma once
// RapidJson
#include "RapidJson/document.h"
#include "RapidJson/writer.h"
#include "RapidJson/stringbuffer.h"
#include "RapidJson/prettywriter.h"

namespace Krampus
{

    class ISavable
    {
    public:
        virtual rapidjson::Value ToJson(rapidjson::Document::AllocatorType& _allocator) const = 0;
        virtual void FromJson(const rapidjson::Value& _json) = 0;
    };

}