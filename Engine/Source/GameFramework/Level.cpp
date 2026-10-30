#include "Level.h"
#include "Managers/LevelManager.h"
#include "Managers/WorldPhysics.h"
#include "Actors/CameraActor.h"
#include "Actors/EmptyActor.h"
#include "Components/ParticleComponent.h"

using namespace Krampus;

Krampus::Level::Level(Engine* _engine)
	: KrampusObject(_engine), cameraManager(_engine), timerManager(_engine), worldPhysics(_engine)
{
}

void Level::InitLevel()
{
}

// A level with no camera at all would draw nothing: give it one, once, when it starts. Not before, so that the cameras
// of a scene or of the level's own InitLevel are used instead of an extra one.
static bool HasCamera(Level& _level)
{
	for (const std::unique_ptr<Actor>& _actor : _level.GetActorManager().GetActors())
		if (_actor->GetComponent<CameraComponent>()) return true;
	return false;
}

void Level::Load()
{
	if (isLoaded) return;

	GetWorld()->GetSettings().ApplyToLevel(*this);	// physics world and time of the project

	InitLevel();
	if (!HasCamera(*this))
	{
		CameraActor* _camera = SpawnActor<CameraActor>();
		_camera->SetPosition(GetWindow().GetSize() / 2.0f);
		LOG(VerbosityType::Log, "Level: no camera, created one");
	}
	isLoaded = true;

	// In the editor the level is paused: BeginPlay waits for Play (see StartPlay).
	if (!GetWorld()->IsLevelPaused()) StartPlay();
	LOG(GetWorld()->GetLevelManager()->IsChangeSilent() ? VerbosityType::Log : VerbosityType::Display, "Level: loaded " + std::string(typeid(*this).name()) + " (" + std::to_string(actorManager.GetActors().size()) + " actors, " + std::to_string(hud.GetWidgets().size()) + " widgets)");
}

void Level::StartPlay()
{
	if (!isLoaded || hasBegunPlay) return;
	hasBegunPlay = true;

	actorManager.BeginPlay();
	hud.BeginPlay();
}

void Krampus::Level::Unload()
{
	if (!isLoaded) return;
	isLoaded = false;
	hasBegunPlay = false;
	LOG(GetWorld()->GetLevelManager()->IsChangeSilent() ? VerbosityType::Log : VerbosityType::Display, "Level: unloading " + std::string(typeid(*this).name()));
	GetWorld()->GetWindowRef().Clear(backgroundColor);
	actorManager.Deconstruct();
	actorManager.BeginDestroy();
	hud.Deconstruct();
	hud.BeginDestroy();
}

void Level::Update(const Float& _deltaTime)
{
	actorManager.Update(_deltaTime);
	worldPhysics.Update();
	cameraManager.UpdateView();
	hud.Update(_deltaTime);
	actorManager.DestroyPendingDeleteActors();
	hud.DestroyPendingDeleteWidgets();
}

void Level::Render()
{
	GetWorld()->GetWindowRef().Clear(backgroundColor);
	cameraManager.Render(false); // TODO SplitScreen
}

Float Krampus::Level::UpdateTime()
{
	return timerManager.Update();
}

CameraComponent* Level::GetDefaultCamera() const
{
	if (CameraComponent* _current = cameraManager.GetCurrent())
		return _current;
	for (const std::unique_ptr<Actor>& _actor : actorManager.GetActors())
		if (CameraComponent* _camera = _actor->GetComponent<CameraComponent>())
			return _camera;
	return nullptr;
}

Krampus::ParticleComponent* Krampus::Level::SpawnParticles(const ParticlePreset& _preset, const FVector2& _position)
{
	EmptyActor* _actor = SpawnActor<EmptyActor>();
	_actor->SetName("Particles");
	_actor->SetPosition(_position);
	ParticleComponent* _particles = _actor->AddComponent<ParticleComponent>(_preset);
	_particles->destroyOwnerWhenDone = !_particles->Settings().looping;
	_particles->Restart();	// from where the actor is now
	return _particles;
}
