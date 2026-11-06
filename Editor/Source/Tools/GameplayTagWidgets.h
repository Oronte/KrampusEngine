#pragma once
#include "EditorContext.h"
#include "GameplayTags/GameplayTag.h"
#include <imgui.h>
#include <string>

namespace Krampus
{

	// The widgets of the gameplay tags, used by the Inspector (GameplayTag, GameplayTagContainer and GameplayTagQuery properties, the
	// argument of AddTag / RemoveTag in an event list) and the window Tools > Gameplay Tags.

	/// <summary>
	/// A button with the name of the tag ("None" when empty); a click opens the list of the tags of the project (a search box and a
	/// tree). _tag is the name of the tag. Returns true when it changed.
	/// </summary>
	bool DrawGameplayTagPicker(const char* _label, std::string& _tag);

	/// <summary>
	/// The tags of a container as small buttons (a click removes one) and a button that adds one from the list.
	/// _encoded is GameplayTagContainer::Encode(). Returns true when it changed.
	/// </summary>
	bool DrawGameplayTagContainerEditor(const char* _label, std::string& _encoded);

	/// <summary>The three containers of a query (all of, any of, none of). _encoded is GameplayTagQuery::Encode().</summary>
	bool DrawGameplayTagQueryEditor(const char* _label, std::string& _encoded);

	/// <summary>Opens the window Tools > Gameplay Tags (declare and remove the tags of the project).</summary>
	void OpenGameplayTagsWindow();
	void DrawGameplayTagsWindow(EditorContext& _context);
}
