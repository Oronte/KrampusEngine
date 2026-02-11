#pragma once
#include "PhysicsActor.h"
#include "Components/AnimationComponent.h"
#include "Components/AudioListenerComponent.h"
#include "Components/CameraComponent.h"

namespace Krampus
{

	class Character : public PhysicsActor
	{
		std::vector<Event<>::ListenerHandle> handles;

	protected:
		AnimationComponent*		animation = nullptr;
		AudioListenerComponent* audioListener = nullptr;
		CameraComponent*		camera = nullptr;

	public:
		Character(Level* _level, const RectangleShapeData& _data);

		virtual void BeginPlay() override;

		void MoveRight();
		void MoveLeft();
	};


}
