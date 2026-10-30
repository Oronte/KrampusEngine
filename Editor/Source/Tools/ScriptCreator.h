#pragma once
#include "EditorContext.h"

namespace Krampus
{

	/// <summary>What a new script is, from the root of its parent: it decides the macro and the constructor.</summary>
	enum class ScriptBase
	{
		Actor,				// KACTOR, Class(Level*)        (diegetic widgets are actors)
		Component,			// KCOMPONENT, Class(Actor*)
		Widget,				// KWIDGET, Class(Level*)       (HUD)
		WidgetComponent		// KWIDGETCOMPONENT, Class(Widget*)
	};

	/// <summary>A class a new script can derive from: one of the engine or one of the game's scripts.</summary>
	struct ScriptParent
	{
		std::string		name;
		ScriptBase		base		= ScriptBase::Actor;
		std::string		include;				// header to include ("Components/SpriteComponent.h", "Enemies/Enemy.h"), empty if unknown
		std::string		extraArguments;			// what the parent's constructor needs after the owner/level ("RectangleShapeData()")
		bool			isScript	= false;		// a class of Content/Scripts
		bool			isDiegetic	= false;		// a diegetic widget (an actor kept at a place of the screen)
	};

	/// <returns>Every class a script can derive from, the bases first (Actor, Component, Widget, WidgetComponent, DiegeticWidget).</returns>
	std::vector<ScriptParent> ScriptParents();

	/// <returns>Empty when _name can be the name of a new class; else why not (C++ rules, a class or a type that already exists...).</returns>
	std::string CheckNewClassName(const std::string& _name);

	/// <summary>
	/// Writes _folder/Name.h and Name.cpp: a reflected class deriving from _parent, ready to edit and to build.
	/// _folder must be Content/Scripts or one of its folders.
	/// </summary>
	/// <returns>Empty when it worked; else why not.</returns>
	std::string CreateScript(const std::string& _name, const ScriptParent& _parent, const std::filesystem::path& _folder);

	const char* NameOfBase(const ScriptBase& _base);

	/// <summary>The class names of the sources are read again at the next check (call it when the New Script window opens).</summary>
	void RefreshClassNames();
}
