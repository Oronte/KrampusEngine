#include "CustomLevel.h"
#include "CustomActor.h"

CustomLevel::CustomLevel(Krampus::Engine* _engine)
	: Level(_engine)
{
}

void CustomLevel::InitLevel()
{
	Krampus::RectangleShapeData _playerData;
	_playerData.size = Krampus::FVector2(150.0f, 250.0f);
	_playerData.texturePath = "Player";
	CustomActor* _player = SpawnActor<CustomActor>(_playerData);
	_player->transform.position = GetWorld()->GetWindowRef().GetSize() / 2.0f;

	Krampus::RectangleShapeData _enemyData;
	_enemyData.size = Krampus::FVector2(150.0f, 250.0f);
	_enemyData.texturePath = "Enemy";
	Krampus::SpriteActor* _enemy = SpawnActor<Krampus::SpriteActor>(_enemyData);
}
