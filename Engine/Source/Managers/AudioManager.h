#pragma once
#include "Utilities/System/Singleton.h"
#include "Audio/Sound.h"
#include "Audio/Music.h"

#define M_AUDIO Krampus::AudioManager::GetInstance()

namespace Krampus
{

	/// <summary>
	/// Supported audio sound file format extensions.
	/// </summary>
	enum class AudioExtensionType
	{
		MP3,
		WAV
	};

	/// <summary>
	/// Singleton manager that loads, caches, and instantiates sound buffers and music streams.
	/// </summary>
	class AudioManager : public Singleton<AudioManager>
	{
#ifdef SHIP
		String															audioPath	= "../../Content/Audio/";
#else
		String															audioPath	= CONTENT_PATH + "Audio/";
#endif
		std::unordered_map<String, std::shared_ptr<sf::SoundBuffer>>	cache;

	public:
		/// <summary>
		/// Default constructor initializing AudioManager.
		/// </summary>
		AudioManager();

	private:
		std::string GetExtensionNameByType(const AudioExtensionType& _type) const;
		std::shared_ptr<sf::SoundBuffer> LoadOrGetSoundBuffer(const String& _path);
	
	public:
		/// <summary>
		/// Creates a Sound effect from an audio resource, caching the loaded buffer.
		/// </summary>
		/// <param name="_fileName">Audio file name without extension.</param>
		/// <param name="_type">Audio file format extension.</param>
		/// <returns>Configured Sound instance.</returns>
		Sound CreateSound(const String& _fileName, const AudioExtensionType& _type);

		/// <summary>
		/// Creates a streaming Music instance from an audio file.
		/// </summary>
		/// <param name="_fileName">Music file name without extension.</param>
		/// <param name="_type">Audio file format extension.</param>
		/// <returns>Configured Music instance.</returns>
		Music CreateMusic(const String& _fileName, const AudioExtensionType& _type);
	};

}
