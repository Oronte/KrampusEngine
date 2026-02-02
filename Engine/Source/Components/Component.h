#pragma once
#include "Core/GameObject.h"

namespace Krampus
{
	class Transform;
	class Engine;

	class Component : public IGameObject
	{
	protected:
		class Actor* owner;
		class Level* level = nullptr;
		Transform& transform;

	public:
		inline Actor* GetOwner() const
		{
			return owner;
		}
		Engine* GetWorld() const;

	protected:
		Component(Actor* _owner);

	public:
		virtual ~Component() = default;

	public:
		virtual void Construct() override;
		virtual void Deconstruct() override;
		virtual void BeginPlay() override {};
		virtual void Tick(const float& _deltaTime) override {};
		virtual void BeginDestroy() override {};

		virtual std::string ToString() const override;
	};


}

