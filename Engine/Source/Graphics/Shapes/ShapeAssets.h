#pragma once
#include "Utilities/System/Event.h"
#include "Utilities/Math/Polygon2D.h"

namespace Krampus
{

	/// <summary>
	/// The complex shapes of the game: Content/Shapes/Name.kshape (made in the editor: Tools > Shape Editor), used by name by the
	/// components that have a shape (SpriteComponent, ImageWidgetComponent, PhysicsComponent: "shape" = Polygon).
	/// A name may have folders: "Enemies/Bat". Files are read once and kept; Save() updates every component that uses the shape.
	///
	///   File: one line "points = x, y; x, y; ..." (pixels around the actor's position), lines starting with # are comments.
	/// </summary>
	class ShapeAssets
	{
	public:
		static constexpr const char*	EXTENSION		= ".kshape";
		static constexpr const char*	DEFAULT_NAME	= "Hexagon";	// what a component gets when it turns into a polygon

		/// <returns>Content/Shapes.</returns>
		static std::filesystem::path Folder();

		/// <returns>The file of a shape: Content/Shapes/Name.kshape.</returns>
		static std::filesystem::path PathOf(const std::string& _name);

		/// <returns>The points of a shape. A missing or broken file gives a hexagon (and a warning, once).</returns>
		static const Polygon2D& Get(const std::string& _name);

		static bool Exists(const std::string& _name);

		/// <summary>Writes the shape and updates everything that uses it (OnChanged).</summary>
		static bool Save(const std::string& _name, const Polygon2D& _polygon);

		static bool Delete(const std::string& _name);

		static bool Rename(const std::string& _from, const std::string& _to);

		/// <returns>The names of every shape of Content/Shapes (with their folders), sorted.</returns>
		static std::vector<std::string> List();

		/// <summary>Forgets the files read: the next Get reads them again.</summary>
		static void ClearCache();

		/// <summary>Broadcast with the name of a shape that was saved, renamed or deleted.</summary>
		static Event<const std::string&>& OnChanged();

		/// <summary>Writes a few shapes to start with (triangle, hexagon, star...) when Content/Shapes has none.</summary>
		static void CreateStarterShapes();
	};

}
