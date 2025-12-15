#pragma once
#include "Utilities/Singleton.h"
#include "Sound.h"
#include "Music.h"

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
		std::string audioPath = CONTENT_PATH + "Audio/";
		std::unordered_map<std::string, std::shared_ptr<sf::SoundBuffer>> cache;

	public:
		AudioManager();

	private:
		std::string GetExtensionNameByType(const AudioExtensionType& _type) const;
		std::shared_ptr<sf::SoundBuffer> LoadOrGetSoundBuffer(const std::string& _path);
	
	public:
		Sound CreateSound(const std::string& _fileName, const AudioExtensionType& _type);
		Music CreateMusic(const std::string& _fileName, const AudioExtensionType& _type);
	};

}

