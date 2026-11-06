#pragma once
#include "EditorContext.h"
#include "Reflection/Reflection.h"
#include <string>

// What the files InspectorPanel*.cpp share (one Inspector, split in several files to stay readable).
namespace Krampus::InspectorInternal
{
	enum class AssetKind { Texture, Font, Sound };

	extern bool g_propertyEdited;				// set by DrawProperty, read by DrawObjectProperties
	extern EditorContext* g_historyContext;		// set while the Inspector draws: its edits can be undone

	void RecordEdit(const std::string& _label, const std::string& _key = "");
	std::string KeyOf(const void* _object, const std::string& _what);
	std::string Lower(std::string _text);

	// InspectorPickers.cpp
	bool DrawAssetPicker(const char* _label, std::string& _path, const AssetKind& _kind, const char* _emptyChoice = nullptr);
	bool DrawShapePicker(const char* _label, std::string& _shape);
	bool DrawScenePicker(const char* _label, std::string& _scene);
	bool DrawFlags(const PropertyInfo& _property, int& _bits);

	// InspectorEvents.cpp
	bool DrawEventList(const PropertyInfo& _property, IGameObject& _owner, std::string& _text);
	bool DrawInputBindings(const PropertyInfo& _property, IGameObject& _owner, std::string& _text, bool _messages = false);
	bool DrawReferencePicker(const PropertyInfo& _property, IGameObject& _owner, std::string& _text);

	// InspectorPanel.cpp
	void DrawProperties(IGameObject& _object);
	void DrawTransform(ITransformable& _actor);
}
