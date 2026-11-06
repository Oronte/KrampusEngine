#include "EditorContext.h"
#include "EditorWidgets.h"
#include "EditorThumbnails.h"
#include "Shortcuts.h"
#include "Scene/Scene.h"
#include "Graphics/Shapes/ShapeAssets.h"
#include "ScriptCreator.h"
#include "FileDrop.h"
#include <imgui.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include "ContentInternal.h"

using namespace Krampus;
using namespace Krampus::ContentInternal;

namespace Krampus::ContentInternal
{
	bool IsIdentifier(const std::string& _name)
	{
		if (_name.empty() || std::isdigit(static_cast<unsigned char>(_name[0]))) return false;
		return std::all_of(_name.begin(), _name.end(), [](char _c) { return std::isalnum(static_cast<unsigned char>(_c)) || _c == '_'; });
	}


	void OpenWithSystem(const std::filesystem::path& _path)
	{
		ShellExecuteW(nullptr, L"open", std::filesystem::absolute(_path).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}

	void ShowInExplorer(const std::filesystem::path& _path)
	{
		const std::wstring _arguments = L"/select,\"" + std::filesystem::absolute(_path).wstring() + L"\"";
		ShellExecuteW(nullptr, L"open", L"explorer.exe", _arguments.c_str(), nullptr, SW_SHOWNORMAL);
	}


	std::string LowerExtension(const std::filesystem::path& _path)
	{
		std::string _extension = _path.extension().string();
		std::transform(_extension.begin(), _extension.end(), _extension.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
		return _extension;
	}

	// "Enemies/Bat" for Content/Shapes/Enemies/Bat.kshape: the name the Shape Editor and the "polygon" properties use.
	// "Content/Shaders/Builtin/Wave.frag" -> "Builtin/Wave".
	std::string ShaderNameOf(const std::filesystem::path& _file)
	{
		std::error_code _error;
		std::filesystem::path _relative = std::filesystem::relative(_file, CONTENT_DIRECTORY / "Shaders", _error);
		if (_error || _relative.empty() || _relative.begin()->string() == "..") _relative = _file.filename();
		_relative.replace_extension();
		return _relative.generic_string();
	}

	std::string ShapeNameOf(const std::filesystem::path& _file)
	{
		std::error_code _error;
		std::filesystem::path _relative = std::filesystem::relative(_file, CONTENT_DIRECTORY / "Shapes", _error);
		if (_error || _relative.empty()) _relative = _file.filename();
		_relative.replace_extension();
		return _relative.generic_string();
	}

	Kind KindOf(const std::filesystem::directory_entry& _entry)
	{
		if (_entry.is_directory()) return Kind::Folder;

		const std::string _extension = LowerExtension(_entry.path());
		if (_extension == Scene::EXTENSION)																return Kind::Scene;
		if (_extension == ".h" || _extension == ".cpp" || _extension == ".hpp")						return Kind::Script;
		if (_extension == ".png" || _extension == ".jpg" || _extension == ".jpeg" || _extension == ".bmp"
			|| _extension == ".tga" || _extension == ".gif")											return Kind::Texture;
		if (_extension == ".wav" || _extension == ".mp3" || _extension == ".ogg" || _extension == ".flac") return Kind::Audio;
		if (_extension == ".ttf" || _extension == ".otf")												return Kind::Font;
		if (_extension == ShapeAssets::EXTENSION)															return Kind::Shape;
		if (_extension == ".frag" || _extension == ".vert")													return Kind::Shader;
		return Kind::Other;
	}

	const char* NameOfKind(const Kind& _kind)
	{
		switch (_kind)
		{
		case Kind::Folder:	return "Folder";
		case Kind::Scene:	return "Scene";
		case Kind::Script:	return "Script";
		case Kind::Texture:	return "Texture";
		case Kind::Audio:	return "Audio";
		case Kind::Font:	return "Font";
		case Kind::Shape:	return "Shape";
		case Kind::Shader:	return "Shader";
		default:			return "File";
		}
	}

	// ---- Icons, drawn with the draw list so that no image file is needed.

	void DrawCenteredText(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const char* _text, const float& _fontSize, const ImU32& _color)
	{
		ImFont* _font = ImGui::GetFont();
		const ImVec2 _size = _font->CalcTextSizeA(_fontSize, FLT_MAX, 0.f, _text);
		_draw->AddText(_font, _fontSize, ImVec2((_min.x + _max.x - _size.x) * 0.5f, (_min.y + _max.y - _size.y) * 0.5f), _color, _text);
	}

	void DrawBadge(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const ImU32& _top, const ImU32& _bottom, const char* _text)
	{
		_draw->AddRectFilledMultiColor(_min, _max, _top, _top, _bottom, _bottom);
		_draw->AddRect(_min, _max, IM_COL32(255, 255, 255, 50), 6.f);
		DrawCenteredText(_draw, _min, _max, _text, (_max.y - _min.y) * 0.32f, IM_COL32(255, 255, 255, 235));
	}

	void DrawFolderIcon(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const bool& _open)
	{
		const float _w = _max.x - _min.x;
		const float _h = _max.y - _min.y;
		const ImVec2 _a(_min.x + _w * 0.06f, _min.y + _h * 0.20f);
		const ImVec2 _b(_max.x - _w * 0.06f, _max.y - _h * 0.14f);
		const ImU32 _back = IM_COL32(214, 150, 40, 255);
		const ImU32 _front = _open ? IM_COL32(255, 214, 110, 255) : IM_COL32(246, 190, 70, 255);

		_draw->AddRectFilled(_a, ImVec2(_a.x + _w * 0.40f, _a.y + _h * 0.16f), _back, 4.f);					// tab
		_draw->AddRectFilled(ImVec2(_a.x, _a.y + _h * 0.07f), _b, _back, 5.f);								// back
		_draw->AddRectFilled(ImVec2(_a.x, _a.y + _h * 0.20f), _b, _front, 5.f);								// front
		_draw->AddRect(ImVec2(_a.x, _a.y + _h * 0.20f), _b, IM_COL32(120, 80, 10, 120), 5.f);
	}

	void DrawAudioIcon(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max)
	{
		DrawBadge(_draw, _min, _max, IM_COL32(150, 90, 220, 255), IM_COL32(90, 50, 150, 255), "");
		const float _w = _max.x - _min.x;
		const float _h = _max.y - _min.y;
		const ImU32 _white = IM_COL32(255, 255, 255, 235);
		const ImVec2 _head(_min.x + _w * 0.40f, _min.y + _h * 0.68f);
		_draw->AddCircleFilled(_head, _w * 0.11f, _white);
		_draw->AddLine(ImVec2(_head.x + _w * 0.10f, _head.y), ImVec2(_head.x + _w * 0.10f, _min.y + _h * 0.24f), _white, 3.f);
		_draw->AddLine(ImVec2(_head.x + _w * 0.10f, _min.y + _h * 0.24f), ImVec2(_head.x + _w * 0.28f, _min.y + _h * 0.36f), _white, 3.f);
	}

	void DrawFileIcon(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const std::string& _extension)
	{
		const float _w = _max.x - _min.x;
		const float _h = _max.y - _min.y;
		const ImVec2 _a(_min.x + _w * 0.16f, _min.y + _h * 0.04f);
		const ImVec2 _b(_max.x - _w * 0.16f, _max.y - _h * 0.04f);
		const float _fold = _w * 0.22f;

		_draw->AddRectFilled(_a, _b, IM_COL32(205, 208, 215, 255), 4.f);
		_draw->AddTriangleFilled(ImVec2(_b.x - _fold, _a.y), ImVec2(_b.x, _a.y + _fold), ImVec2(_b.x - _fold, _a.y + _fold), IM_COL32(150, 154, 165, 255));
		DrawCenteredText(_draw, ImVec2(_a.x, _min.y + _h * 0.40f), ImVec2(_b.x, _b.y), _extension.empty() ? "?" : _extension.c_str() + 1, _w * 0.17f, IM_COL32(60, 62, 70, 255));
	}

	void DrawTextureIcon(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const std::filesystem::path& _path)
	{
		_draw->AddRectFilled(_min, _max, IM_COL32(38, 38, 44, 255), 5.f);

		if (sf::Texture* _texture = ThumbnailOf(_path))
		{
			// Fit inside the square, keep the proportions.
			const sf::Vector2f _size(_texture->getSize());
			if (_size.x > 0.f && _size.y > 0.f)
			{
				const float _scale = std::min((_max.x - _min.x) / _size.x, (_max.y - _min.y) / _size.y);
				const ImVec2 _half(_size.x * _scale * 0.5f, _size.y * _scale * 0.5f);
				const ImVec2 _center((_min.x + _max.x) * 0.5f, (_min.y + _max.y) * 0.5f);
				_draw->AddImage(TextureId(*_texture), ImVec2(_center.x - _half.x, _center.y - _half.y), ImVec2(_center.x + _half.x, _center.y + _half.y));
			}
		}
		else
		{
			DrawCenteredText(_draw, _min, _max, "IMG", (_max.y - _min.y) * 0.28f, IM_COL32(160, 160, 170, 255));
		}
		_draw->AddRect(_min, _max, IM_COL32(255, 255, 255, 40), 5.f);
	}

	void DrawIcon(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const std::filesystem::directory_entry& _entry, const Kind& _kind, const bool& _hovered)
	{
		switch (_kind)
		{
		case Kind::Folder:	DrawFolderIcon(_draw, _min, _max, _hovered); break;
		case Kind::Scene:	DrawBadge(_draw, _min, _max, IM_COL32(70, 140, 235, 255), IM_COL32(35, 70, 150, 255), "KA7"); break;
		case Kind::Script:	DrawBadge(_draw, _min, _max, IM_COL32(70, 185, 110, 255), IM_COL32(30, 105, 60, 255), LowerExtension(_entry.path()) == ".cpp" ? "C++" : ".h"); break;
		case Kind::Texture:	DrawTextureIcon(_draw, _min, _max, _entry.path()); break;
		case Kind::Audio:	DrawAudioIcon(_draw, _min, _max); break;
		case Kind::Font:	DrawBadge(_draw, _min, _max, IM_COL32(235, 140, 70, 255), IM_COL32(150, 70, 30, 255), "Aa"); break;
		case Kind::Shape:	DrawBadge(_draw, _min, _max, IM_COL32(90, 150, 255, 255), IM_COL32(40, 80, 170, 255), "SHP"); break;
		case Kind::Shader:	DrawBadge(_draw, _min, _max, IM_COL32(190, 110, 240, 255), IM_COL32(100, 50, 150, 255), "GLSL"); break;
		default:			DrawFileIcon(_draw, _min, _max, LowerExtension(_entry.path())); break;
		}
	}

	std::string FileSizeText(const std::filesystem::directory_entry& _entry)
	{
		std::error_code _error;
		const double _bytes = static_cast<double>(_entry.file_size(_error));
		char _buffer[32];
		if (_error)					return "";
		if (_bytes < 1024.0)		std::snprintf(_buffer, sizeof(_buffer), "%.0f B", _bytes);
		else if (_bytes < 1048576.0)	std::snprintf(_buffer, sizeof(_buffer), "%.1f KB", _bytes / 1024.0);
		else						std::snprintf(_buffer, sizeof(_buffer), "%.1f MB", _bytes / 1048576.0);
		return _buffer;
	}

}
