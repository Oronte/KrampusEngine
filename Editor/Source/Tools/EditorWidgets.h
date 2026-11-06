#pragma once
#include <imgui.h>

namespace Krampus::EditorUI
{
	// Sliders of the editor: drag as usual, but a simple click (no movement) turns the slider into a text field to type the value
	// (ImGui only does it with Ctrl+click). The value is left unchanged by the click. Enter validates, Escape cancels.
	// The DragXXX widgets do it by themselves (io.ConfigDragClickToInputText, EditorLayer.cpp).

	namespace Detail
	{
		inline ImGuiID& PendingType()
		{
			static ImGuiID _id = 0;
			return _id;
		}

		// Runs the slider. _before is the value before the click (put back when the click was not a drag).
		template <typename T, typename Slider>
		bool ClickToType(const char* _label, T* _value, Slider _slider)
		{
			ImGuiID& _pending = PendingType();
			const ImGuiID _id = ImGui::GetID(_label);
			if (_pending == _id)
			{
				_pending = 0;
				ImGui::SetKeyboardFocusHere();	// a slider that gets the keyboard focus is a text field
			}

			const T _before = *_value;
			bool _changed = _slider();

			const ImGuiIO& _io = ImGui::GetIO();
			if (ImGui::IsItemDeactivated() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsItemHovered()
				&& _io.MouseDragMaxDistanceSqr[ImGuiMouseButton_Left] < 16.f && !_io.KeyCtrl)
			{
				*_value = _before;
				_changed = false;
				_pending = _id;
			}
			return _changed;
		}
	}

	inline bool SliderFloat(const char* _label, float* _value, const float _min, const float _max, const char* _format = "%.3f", const ImGuiSliderFlags _flags = 0)
	{
		return Detail::ClickToType(_label, _value, [&]() { return ImGui::SliderFloat(_label, _value, _min, _max, _format, _flags); });
	}

	inline bool SliderInt(const char* _label, int* _value, const int _min, const int _max, const char* _format = "%d", const ImGuiSliderFlags _flags = 0)
	{
		return Detail::ClickToType(_label, _value, [&]() { return ImGui::SliderInt(_label, _value, _min, _max, _format, _flags); });
	}
}
