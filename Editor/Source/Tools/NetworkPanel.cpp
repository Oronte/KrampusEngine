#include "EditorContext.h"
#include "Network/NetworkManager.h"
#include <imgui.h>
#include <deque>

using namespace Krampus;

// Tools > Network: host or join a game from the editor, see who is connected and what messages go by.
// To test with two copies, host here and start KrampusGame.exe with --join (see the tutorial), or the opposite.

namespace
{
	bool									g_open		= false;
	NetworkManager*							g_watching	= nullptr;
	Event<const NetMessage&>::ListenerHandle	g_handle;
	std::deque<std::string>					g_log;

	std::string Describe(const PropertyValue& _value)
	{
		return std::visit([](const auto& _v) -> std::string
		{
			using V = std::decay_t<decltype(_v)>;
			if constexpr (std::is_same_v<V, bool>)				return _v ? "true" : "false";
			else if constexpr (std::is_same_v<V, int>)			return std::to_string(_v);
			else if constexpr (std::is_same_v<V, float>)		return std::to_string(_v);
			else if constexpr (std::is_same_v<V, std::string>)	return "\"" + _v + "\"";
			else if constexpr (std::is_same_v<V, FVector2>)		return "(" + std::to_string(_v.x) + ", " + std::to_string(_v.y) + ")";
			else												return "color";
		}, _value);
	}
}

namespace Krampus
{
	void OpenNetworkWindow()
	{
		g_open = true;
		ImGui::SetWindowFocus("Network");
	}

	void DrawNetworkWindow(EditorContext& _context)
	{
		if (!g_open) return;
		NetworkManager* _net = _context.engine->GetNetwork();

		// The log of messages (the internal ones, "__...", never reach it).
		if (g_watching != _net)
		{
			g_watching = _net;
			g_log.clear();
			g_handle = _net->onMessage.AddListener([](const NetMessage& _message)
			{
				std::string _line = (_message.reliable ? "TCP " : "UDP ") + std::string("from ") + std::to_string(_message.sender) + ": " + _message.name;
				for (const PropertyValue& _value : _message.args) _line += " " + Describe(_value);
				g_log.push_back(_line);
				if (g_log.size() > 60) g_log.pop_front();
			});
		}

		ImGui::SetNextWindowSize(ImVec2(420.f, 460.f), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Network", &g_open))
		{
			ImGui::End();
			return;
		}

		NetworkSettings& _settings = _context.engine->GetSettings().network;
		ImGui::Text("%s", _net->GetStatusText().c_str());
		if (_net->IsOnline())
			ImGui::TextDisabled("Player %d, %d in the game%s", _net->GetLocalId(), _net->GetPlayerCount(), _net->IsUdpReady() ? "" : "  (UDP not ready: using TCP)");
		ImGui::Separator();

		const bool _offline = _net->GetRole() == NetRole::Offline;
		ImGui::BeginDisabled(!_offline);
		int _port = _settings.port;
		ImGui::SetNextItemWidth(100.f);
		if (ImGui::InputInt("Port", &_port, 0)) _settings.port = std::clamp(_port, 1, 65535);
		if (ImGui::Button("Host")) _net->Host();
		ImGui::SameLine();

		char _address[128];
		std::snprintf(_address, sizeof(_address), "%s", _settings.address.c_str());
		ImGui::SetNextItemWidth(160.f);
		if (ImGui::InputText("##address", _address, sizeof(_address))) _settings.address = _address;
		ImGui::SameLine();
		if (ImGui::Button("Join")) _net->Join();
		ImGui::EndDisabled();

		ImGui::BeginDisabled(_offline);
		if (ImGui::Button("Disconnect")) _net->Disconnect();
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::TextDisabled("Saved defaults: Settings > Network");

		ImGui::Separator();
		ImGui::TextUnformatted("Players");
		if (_net->IsOnline())
		{
			ImGui::BulletText("%d  (you)%s", _net->GetLocalId(), _net->IsHost() ? "  host" : "");
			for (const int _id : _net->GetOtherPlayers())
				ImGui::BulletText("%d%s", _id, _id == 0 ? "  host" : "");
			if (_net->IsClient()) ImGui::TextDisabled("Ping %.0f ms", _net->GetPingMs());
		}
		else
			ImGui::TextDisabled("Not in a game");

		ImGui::Separator();
		ImGui::TextUnformatted("Messages");
		ImGui::SameLine();
		if (ImGui::SmallButton("Clear")) g_log.clear();
		ImGui::BeginChild("NetworkLog", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders);
		for (const std::string& _line : g_log)
			ImGui::TextUnformatted(_line.c_str());
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.f);
		ImGui::EndChild();

		ImGui::End();
	}
}
