#pragma once
#include "Actors/Actor.h"
#include "UI/Widget.h"

namespace Krampus
{

	class ActorManager
	{
		std::vector<std::unique_ptr<Actor>> actors;
		std::vector<std::unique_ptr<Actor>> pendingDeleteActors;

	public:
		inline const std::vector<std::unique_ptr<Actor>>& GetActors() const noexcept
		{
			return actors;
		}
		template <typename Type, IS_BASE_OF(Actor, Type), IS_NOT_BASE_OF(Widget, Type)>
		inline std::vector<Type*> GetAllActorOfClass()
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
		inline Type* CreateActor(Args&&... _args)
		{
			std::unique_ptr<Type> _actor = std::make_unique<Type>(std::forward<Args>(_args)...);
			Type* _rawActor = _actor.get();
			actors.push_back(std::move(_actor));
			_rawActor->Construct();
			return _rawActor;
		}

		inline void MarkForDeleteActor(Actor* toMark)
		{
			auto it = std::find_if(
				actors.begin(),
				actors.end(),
				[&](const std::unique_ptr<Actor>& actor)
				{
					return actor.get() == toMark;
				});

			if (it == actors.end())
				return;

			pendingDeleteActors.push_back(std::move(*it));

			actors.erase(it);
		}

		inline void DestroyPendingDeleteActors()
		{
			pendingDeleteActors.clear();
		}

		ActorManager() = default;

	public:
		void BeginPlay();
		void Update(const float& _deltaTime);
		void BeginDestroy();
	};

}