#pragma once
#include "Core/Object.h"

namespace Krampus
{
	class Transform;

	class Component : public KObject
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

