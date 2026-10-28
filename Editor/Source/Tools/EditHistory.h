#pragma once
#include "Utilities/Math/Vector2D.h"

namespace Krampus
{

	struct EditorContext;

	/// <summary>
	/// Undo / Redo of what is done to the scene in the editor.
	/// Before each change the scene is saved to a small file (Content/Editor/_undo/); undoing swaps that file back in place of the level.
	/// Edits that follow each other on the same value (dragging a slider, typing a name) count as one step.
	/// </summary>
	class EditHistory
	{
	public:
		/// <summary>What was selected: 0 nothing, 1 an actor, 2 a HUD widget, and its position in the level list.</summary>
		struct Selection
		{
			int		kind	= 0;
			int		index	= -1;
		};

	private:
		struct Entry
		{
			std::filesystem::path	file;
			std::string				label;
			Selection				selection;
		};

		static constexpr size_t		MAX_STEPS	= 64;

		std::vector<Entry>			undoStack;
		std::vector<Entry>			redoStack;
		int							counter			= 0;
		std::string					lastKey;
		double						lastKeyTime		= -10.0;

		// A swap is made at the start of the next frame: what is put back afterwards (selection, viewport camera).
		bool						pending			= false;
		int							pendingSerial	= 0;
		Selection					pendingSelection;
		std::filesystem::path		pendingFile;
		FVector2					cameraPosition;
		float						cameraRotation	= 0.f;
		FVector2					cameraScale		= FVector2(1.f, 1.f);

	public:
		static std::filesystem::path Directory();

		/// <summary>Forgets every step and deletes the files (new or opened scene).</summary>
		void Clear();

		/// <summary>
		/// Saves the scene as it is NOW, as a step that Undo goes back to. Call it just before changing something.
		/// With the same key as the previous call, a moment ago, nothing is saved: it is the same continued change.
		/// </summary>
		void Record(EditorContext& _context, const std::string& _label, const std::string& _key = "");

		bool Undo(EditorContext& _context);
		bool Redo(EditorContext& _context);

		bool CanUndo() const { return !undoStack.empty() && !pending; }
		bool CanRedo() const { return !redoStack.empty() && !pending; }
		std::string UndoLabel() const { return undoStack.empty() ? "" : undoStack.back().label; }
		std::string RedoLabel() const { return redoStack.empty() ? "" : redoStack.back().label; }

		/// <summary>Every frame: finishes a swap once the new level is there.</summary>
		void Update(EditorContext& _context);

	private:
		std::filesystem::path Snapshot(EditorContext& _context);
		Selection SelectionOf(EditorContext& _context);
		void Restore(EditorContext& _context, const Entry& _entry);
		void Forget(std::vector<Entry>& _stack);
	};
}
