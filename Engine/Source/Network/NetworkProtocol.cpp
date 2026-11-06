#include "NetworkProtocol.h"

namespace Krampus::NetProtocol
{
	std::string CleanName(std::string _name, const int _id)
	{
		if (_name.size() > MaxNameLength) _name.resize(MaxNameLength);
		_name.erase(std::remove_if(_name.begin(), _name.end(), [](const unsigned char _c) { return _c < 32; }), _name.end());
		return _name.empty() ? "Player " + std::to_string(_id) : _name;
	}

	void WriteValue(sf::Packet& _packet, const PropertyValue& _value)
	{
		_packet << static_cast<std::uint8_t>(_value.index());
		switch (_value.index())
		{
		case 0: _packet << std::get<bool>(_value); break;
		case 1: _packet << static_cast<std::int32_t>(std::get<int>(_value)); break;
		case 2: _packet << std::get<float>(_value); break;
		case 3: _packet << std::get<std::string>(_value); break;
		case 4: _packet << std::get<FVector2>(_value).x << std::get<FVector2>(_value).y; break;
		default:
		{
			const Color& _color = std::get<Color>(_value);
			_packet << static_cast<std::uint8_t>(_color.r) << static_cast<std::uint8_t>(_color.g) << static_cast<std::uint8_t>(_color.b) << static_cast<std::uint8_t>(_color.a);
			break;
		}
		}
	}

	bool ReadValue(sf::Packet& _packet, PropertyValue& _value)
	{
		std::uint8_t _type = 0;
		if (!(_packet >> _type)) return false;

		switch (_type)
		{
		case 0: { bool _v = false; if (!(_packet >> _v)) return false; _value = _v; return true; }
		case 1: { std::int32_t _v = 0; if (!(_packet >> _v)) return false; _value = static_cast<int>(_v); return true; }
		case 2: { float _v = 0.f; if (!(_packet >> _v)) return false; _value = _v; return true; }
		case 3: { std::string _v; if (!(_packet >> _v)) return false; _value = _v; return true; }
		case 4: { float _x = 0.f, _y = 0.f; if (!(_packet >> _x >> _y)) return false; _value = FVector2(_x, _y); return true; }
		case 5:
		{
			std::uint8_t _r = 0, _g = 0, _b = 0, _a = 0;
			if (!(_packet >> _r >> _g >> _b >> _a)) return false;
			_value = Color(_r, _g, _b, _a);
			return true;
		}
		default: return false;
		}
	}

	// After the type byte: sender, target, name, values.
	void WriteMessage(sf::Packet& _packet, const NetMessage& _message, const int _target)
	{
		_packet << static_cast<std::uint8_t>(PMessage) << static_cast<std::int32_t>(_message.sender) << static_cast<std::int32_t>(_target) << _message.name
			<< static_cast<std::uint8_t>(std::min<size_t>(_message.args.size(), 255));
		for (size_t _i = 0; _i < _message.args.size() && _i < 255; ++_i)
			WriteValue(_packet, _message.args[_i]);
	}

	bool ReadMessage(sf::Packet& _packet, NetMessage& _message, int& _target)
	{
		std::int32_t _sender = 0, _to = 0;
		std::uint8_t _count = 0;
		if (!(_packet >> _sender >> _to >> _message.name >> _count)) return false;

		_message.sender = _sender;
		_target = _to;
		_message.args.clear();
		for (std::uint8_t _i = 0; _i < _count; ++_i)
		{
			PropertyValue _value;
			if (!ReadValue(_packet, _value)) return false;
			_message.args.push_back(std::move(_value));
		}
		return true;
	}
}
