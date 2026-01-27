#pragma once
#include "Actors/Actor.h"
#include "UI/Widget.h"

namespace Krampus
{

	class ActorManager
	{
		std::vector<std::unique_ptr<Actor>> actors;

	public:
		inlin const std::vector<std::unique_ptr<Actor>>& GetActors() const noexcept
		{
			return actors;
		}
		template <typename Type, IS_BASE_OF(Actor, Type), IS_NOT_BASE_OF(Widget, Type)>
		inlin std::vector<Type*> GetAllActorOfClass()
		{
			std::vector<Type*> _finalVector;

			for (const std::unique_ptr<Actor>& _actor : actors)
			{
				if (Type* _castedActor = Cast<Type>(_actor.get())) 
					_finalVector.push_back(_castedActor);
			}

			return _finalVector;
		}
		template <typename Type, typename ...Args, IS_BASE_OF(Actor, Type), IS_NOT_BASE_OF(Widget, Type)>
		inlin Type* CreateActor(Args&&... _args)
		{
			std::unique_ptr<Type> _actor = std::make_unique<Type>(std::forward<Args>(_args)...);
			Type* _rawActor = _actor.get();
			actors.push_back(std::move(_actor));
			_rawActor->Construct();
			return _rawActor;
		}
		
		inlin void DeleteActor(Actor* _toDelete)
		{
			std::erase_if(actors, [&](const std::unique_ptr<Actor>& _actor)
				{
					return _actor.get() == _toDelete;
				});
		}

		ActorManager() = default;

	public:
		void BeginPlay();
		void Update(const float& _deltaTime);
		void BeginDestroy();
	};

}