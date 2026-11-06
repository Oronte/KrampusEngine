#pragma once
#include "Sample.h"

namespace Krampus
{

	/// <summary>
	/// A sound effect played from a shared, fully loaded buffer (see AudioManager::CreateSound).
	/// </summary>
	class Sound : public Sample<sf::Sound>
	{
		std::shared_ptr<sf::SoundBuffer> buffer;

	public:
		Sound() = default;
		explicit Sound(std::shared_ptr<sf::SoundBuffer> _buffer)
			: Sample(std::make_unique<sf::Sound>(*_buffer)), buffer(std::move(_buffer)) {}

		[[nodiscard]] const std::shared_ptr<sf::SoundBuffer>& GetSharedBuffer() const { return buffer; }
	};

}
