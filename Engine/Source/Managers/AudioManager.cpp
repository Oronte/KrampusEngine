#include "AudioManager.h"

Krampus::AudioManager::AudioManager()
{
	std::filesystem::create_directories(audioPath);
}

std::string Krampus::AudioManager::GetExtensionNameByType(const AudioExtensionType& _type) const
{
	return std::vector<std::string> { ".mp3", ".wav" } [CAST(int, _type)] ;
}

std::shared_ptr<sf::SoundBuffer> Krampus::AudioManager::LoadOrGetSoundBuffer(const std::string& _path)
{
	auto _iterator = cache.find(_path);
	if (_iterator != cache.end())
		return _iterator->second;

	std::shared_ptr<sf::SoundBuffer> _buffer = std::make_shared<sf::SoundBuffer>();
	if (!_buffer->loadFromFile(_path))
		THROW_EXCEPTION("Cannot load sound: " + _path);

	cache.emplace(_path, _buffer);
	return _buffer;
}

Krampus::Sound Krampus::AudioManager::CreateSound(const std::string& _fileName, const AudioExtensionType& _type)
{
	return Sound(LoadOrGetSoundBuffer(audioPath + _fileName + GetExtensionNameByType(_type)));
}

Krampus::Music Krampus::AudioManager::CreateMusic(const std::string& _fileName, const AudioExtensionType& _type)
{
	Music _music;
	if (_music.Open(audioPath + _fileName + GetExtensionNameByType(_type)))
		LOG_ERROR("Cannot load music: " + _fileName);
	return _music;
}
