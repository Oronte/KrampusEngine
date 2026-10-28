#include "AudioManager.h"
#include "Core/AssetPack.h"

Krampus::AudioManager::AudioManager()
{
	if (!AssetPack::IsMounted()) std::filesystem::create_directories(audioPath.StdString());	// a packed game has no Content folder
}

std::string Krampus::AudioManager::GetExtensionNameByType(const AudioExtensionType& _type) const
{
	return std::vector<std::string> { ".mp3", ".wav" } [CAST(int, _type)] ;
}

std::shared_ptr<sf::SoundBuffer> Krampus::AudioManager::LoadOrGetSoundBuffer(const String& _path)
{
	auto _iterator = cache.find(_path);
	if (_iterator != cache.end())
		return _iterator->second;

	std::shared_ptr<sf::SoundBuffer> _buffer = std::make_shared<sf::SoundBuffer>();
	const std::uint8_t* _bytes = nullptr;
	std::size_t _length = 0;
	const bool _loaded = AssetPack::Find(_path.StdString(), _bytes, _length) ? _buffer->loadFromMemory(_bytes, _length) : _buffer->loadFromFile(_path.StdString());
	if (!_loaded)
		THROW_EXCEPTION("Cannot load sound: " + _path);

	LOG(VerbosityType::Log, "Audio: loaded " + _path.StdString() + " (" + std::to_string(_buffer->getDuration().asSeconds()) + " s)");
	cache.emplace(_path, _buffer);
	return _buffer;
}

Krampus::Sound Krampus::AudioManager::CreateSound(const String& _fileName, const AudioExtensionType& _type)
{
	return Sound(LoadOrGetSoundBuffer(audioPath + _fileName + GetExtensionNameByType(_type)));
}

Krampus::Music Krampus::AudioManager::CreateMusic(const String& _fileName, const AudioExtensionType& _type)
{
	LOG(VerbosityType::Log, "Audio: opening music " + _fileName.StdString());
	Music _music;
	if (!_music.Open(audioPath + _fileName + GetExtensionNameByType(_type)))
		LOG_ERROR("Cannot load music: " + _fileName);
	return _music;
}
