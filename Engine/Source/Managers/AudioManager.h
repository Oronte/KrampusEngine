#pragma once
#include "Utilities/System/Singleton.h"
#include "Audio/Sound.h"
#include "Audio/Music.h"

#define M_AUDIO Krampus::AudioManager::GetInstance()

namespace Krampus
{

	enum class AudioExtensionType
	{
		MP3,
		WAV
	};

	class AudioManager : public Singleton<AudioManager>
	{
#ifdef SHIP
		String															audioPath	= "../../Content/Audio/";
#else
		String															audioPath	= "Content/Audio/";
#endif
		std::unordered_map<String, std::shared_ptr<sf::SoundBuffer>>	cache;

	public:
		AudioManager();

	private:
		std::string GetExtensionNameByType(const AudioExtensionType& _type) const;
		std::shared_ptr<sf::SoundBuffer> LoadOrGetSoundBuffer(const String& _path);
	
	public:
		Sound CreateSound(const String& _fileName, const AudioExtensionType& _type);
		Music CreateMusic(const String& _fileName, const AudioExtensionType& _type);
	};

}

