#include "EditorContext.h"
#include "Network/NetworkManager.h"
#include <imgui.h>
#include <deque>

using namespace Krampus;

// Tools > Network: host, join or find a game on the LAN from the editor, see the players (name, ping), the shared values, the traffic,
// send a test message and watch the messages that go by.
// To test with two copies, host here and start KrampusGame.exe with --join (or Search LAN in the other one), or the opposite.

namespace
{
	bool									g_open		= false;
	NetworkManager*							g_watching	= nullptr;
	Event<const NetMessage&>::ListenerHandle	g_handle;
	std::deque<std::string>					g_log;
	ImGuiTextFilter							g_logFilter;
	char									g_sendName[64]		= "Hello";
	char									g_sendValue[128]	= "";
	char									g_sharedKey[64]		= "score";
	char									g_sharedValue[128]	= "0";

	std::string Describe(const PropertyValue& _value)
	{
		return std::visit([](const auto& _v) -> std::string
		{
			using V = std::decay_t<decltype(_v)>;
			if constexpr (std::is_same_v<V, bool>)				return _v ? "true" : "false";
			else if constexpr (std::is_same_v<V, int>)			return std::to_string(_v);
			else if constexpr (std::is_same_v<V, float>)		return std::format("{:.3g}", _v);
			else if constexpr (std::is_same_v<V, std::string>)	return "\"" + _v + "\"";
			else if constexpr (std::is_same_v<V, FVector2>)		return std::format("({:.3g}, {:.3g})", _v.x, _v.y);
			else												return std::format("color({}, {}, {}, {})", static_cast<int>(_v.r), static_cast<int>(_v.g), static_cast<int>(_v.b), static_cast<int>(_v.a));
		}, _value);
	}

	/// <summary>"12" is an int, "1.5" a float, "true"/"false" a bool, anything else a string: what the test fields send.</summary>
	PropertyValue Parse(const std::string& _text)
	{
		if (_text == "true") return true;
		if (_text == "false") return false;
		try
		{
			size_t _used = 0;
			const int _int = std::stoi(_text, &_used);
			if (_used == _text.size()) return _int;
			const float _float = std::stof(_text, &_used);
			if (_used == _text.size()) return _float;
		}
		catch (const std::exception&) { }
		return _text;
	}

	std::string Bytes(const double _bytes)
	{
		if (_bytes < 1024.0) return std::format("{:.0f} B", _bytes);
		if (_bytes < 1024.0 * 1024.0) return std::format("{:.1f} KB", _bytes / 1024.0);
		return std::format("{:.1f} MB", _bytes / (1024.0 * 1024.0));
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
			g_handle = _net->onMessage.AddListener([_net](const NetMessage& _message)
			{
				std::string _line = (_message.reliable ? "TCP " : "UDP ") + _net->GetPlayerName(_message.sender) + ": " + _message.name;
				for (const PropertyValue& _value : _message.args) _line += " " + Describe(_value);
				g_log.push_back(_line);
				if (g_log.size() > 200) g_log.pop_front();
			});
		}

		ImGui::SetNextWindowSize(ImVec2(460.f, 620.f), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Network", &g_open))
		{
			ImGui::End();
			return;
		}

		NetworkSettings& _settings = _context.engine->GetSettings().network;
		ImGui::Text("%s", _net->GetStatusText().c_str());
		if (_net->IsOnline())
			ImGui::TextDisabled("Player %d, %d in the game%s", _net->GetLocalId(), _net->GetPlayerCount(), _net->IsUdpReady() ? "" : "  (UDP not ready: using TCP)");

		// ---- Connection
		if (ImGui::CollapsingHeader("Connection", ImGuiTreeNodeFlags_DefaultOpen))
		{
			char _name[64];
			std::snprintf(_name, sizeof(_name), "%s", _net->playerName.c_str());
			ImGui::SetNextItemWidth(160.f);
			ImGui::InputText("Player name", _name, sizeof(_name));
			if (ImGui::IsItemDeactivatedAfterEdit())	// Enter or a click elsewhere: the others see the new name
			{
				_net->SetPlayerName(_name);
				_settings.playerName = _net->playerName;
			}

			const bool _offline = _net->GetRole() == NetRole::Offline;
			ImGui::BeginDisabled(!_offline);
			int _port = _settings.port;
			ImGui::SetNextItemWidth(100.f);
			if (ImGui::InputInt("Port", &_port, 0)) _settings.port = std::clamp(_port, 1, 65535);
			ImGui::SameLine();
			if (ImGui::Button("Host")) _net->Host();

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
		}

		// ---- LAN
		if (ImGui::CollapsingHeader("Games on the LAN", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginDisabled(_net->IsSearchingLan());
			if (ImGui::Button(_net->IsSearchingLan() ? "Searching..." : "Search LAN")) _net->SearchLan();
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::TextDisabled("discovery port %d", static_cast<int>(_net->discoveryPort));

			if (_net->GetLanGames().empty())
				ImGui::TextDisabled(_net->IsSearchingLan() ? "Looking..." : "No game found");
			for (size_t _i = 0; _i < _net->GetLanGames().size(); ++_i)
			{
				const NetLanGame& _game = _net->GetLanGames()[_i];
				ImGui::PushID(static_cast<int>(_i));
				ImGui::BeginDisabled(_net->GetRole() != NetRole::Offline);
				const bool _join = ImGui::SmallButton("Join");
				ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::Text("%s  by %s  %s:%d  (%d/%d)", _game.gameName.c_str(), _game.hostName.c_str(), _game.address.c_str(), _game.port, _game.players, _game.maxPlayers);
				ImGui::PopID();
				if (_join)
				{
					const NetLanGame _copy = _game;		// Join changes the list
					_net->Join(_copy);
					break;
				}
			}
		}

		// ---- Players
		if (ImGui::CollapsingHeader("Players", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (!_net->IsOnline())
				ImGui::TextDisabled("Not in a game");
			else if (ImGui::BeginTable("Players", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
			{
				ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthFixed, 30.f);
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Ping", ImGuiTableColumnFlags_WidthFixed, 60.f);
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 50.f);
				ImGui::TableHeadersRow();
				int _kick = -1;
				for (const NetPlayer& _player : _net->GetPlayers())
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("%d", _player.id);
					ImGui::TableNextColumn();
					ImGui::Text("%s%s%s", _player.name.c_str(), _player.isHost ? "  (host)" : "", _player.isLocal ? "  (you)" : "");
					ImGui::TableNextColumn();
					if (_player.isHost) ImGui::TextDisabled("-");
					else ImGui::Text("%.0f ms", _player.pingMs);
					ImGui::TableNextColumn();
					if (_net->IsHost() && !_player.isLocal)
					{
						ImGui::PushID(_player.id);
						if (ImGui::SmallButton("Kick")) _kick = _player.id;
						ImGui::PopID();
					}
				}
				ImGui::EndTable();
				if (_kick >= 0) _net->Kick(_kick);
			}
			if (!_net->GetDisconnectReason().empty() && !_net->IsOnline())
				ImGui::TextDisabled("Last game ended: %s", _net->GetDisconnectReason().c_str());
		}

		// ---- Shared values
		if (ImGui::CollapsingHeader("Shared values"))
		{
			ImGui::TextDisabled("The same on every machine, kept by the host (NetworkManager::SetShared)");
			if (ImGui::BeginTable("Shared", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
			{
				for (const auto& [_key, _value] : _net->GetSharedValues())
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(_key.c_str());
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(Describe(_value).c_str());
				}
				ImGui::EndTable();
			}
			ImGui::SetNextItemWidth(110.f);
			ImGui::InputText("##sharedKey", g_sharedKey, sizeof(g_sharedKey));
			ImGui::SameLine();
			ImGui::SetNextItemWidth(110.f);
			ImGui::InputText("##sharedValue", g_sharedValue, sizeof(g_sharedValue));
			ImGui::SameLine();
			if (ImGui::Button("Set") && g_sharedKey[0]) _net->SetShared(g_sharedKey, Parse(g_sharedValue));
			ImGui::SameLine();
			if (ImGui::Button("+1") && g_sharedKey[0]) _net->AddShared(g_sharedKey, 1.f);
		}

		// ---- Traffic
		if (ImGui::CollapsingHeader("Traffic"))
		{
			const NetStats& _stats = _net->GetStats();
			ImGui::Text("Sent      %s/s   (%s, %llu packets)", Bytes(_stats.sentPerSecond).c_str(), Bytes(static_cast<double>(_stats.bytesSent)).c_str(), static_cast<unsigned long long>(_stats.packetsSent));
			ImGui::Text("Received  %s/s   (%s, %llu packets)", Bytes(_stats.receivedPerSecond).c_str(), Bytes(static_cast<double>(_stats.bytesReceived)).c_str(), static_cast<unsigned long long>(_stats.packetsReceived));
		}

		// ---- Messages
		if (ImGui::CollapsingHeader("Messages", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SetNextItemWidth(110.f);
			ImGui::InputText("##sendName", g_sendName, sizeof(g_sendName));
			ImGui::SameLine();
			ImGui::SetNextItemWidth(140.f);
			ImGui::InputText("##sendValue", g_sendValue, sizeof(g_sendValue));
			ImGui::SameLine();
			if (ImGui::Button("Send to all") && g_sendName[0])
				_net->Send(g_sendName, g_sendValue[0] ? std::vector<PropertyValue>{ Parse(g_sendValue) } : std::vector<PropertyValue>{ });
			ImGui::SetItemTooltip("Sends the message (name, and the value if there is one) to every player, this one included");

			g_logFilter.Draw("Filter", 180.f);
			ImGui::SameLine();
			if (ImGui::SmallButton("Clear")) g_log.clear();
			ImGui::BeginChild("NetworkLog", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders);
			for (const std::string& _line : g_log)
				if (g_logFilter.PassFilter(_line.c_str()))
					ImGui::TextUnformatted(_line.c_str());
			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.f);
			ImGui::EndChild();
		}

		ImGui::End();
	}
}
