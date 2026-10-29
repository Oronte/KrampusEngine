#pragma once
#include "Actors/Actor.h"
#include "../UI/Diegetic/DiegeticWidget.h"

namespace Krampus
{

	/// <summary>
	/// Manages the lifecycle, spawning, updating, and deferred destruction of all actors within a level.
	/// </summary>
	class ActorManager
	{
		std::vector<std::unique_ptr<Actor>>		actors;
		std::vector<std::unique_ptr<Actor>>		pendingDeleteActors;
		Bool									isPlaying				= false;

	public:
		/// <summary>
		/// Gets a const reference to the list of managed actors.
		/// </summary>
		/// <returns>Vector of unique pointers to Actor.</returns>
		inline const std::vector<std::unique_ptr<Actor>>& GetActors() const noexcept
		{
			return actors;
		}

		/// <summary>
		/// Retrieves all active actors that inherit from or match a specific Actor derived class.
		/// </summary>
		/// <typeparam name="Type">Actor-derived type.</typeparam>
		/// <returns>Vector of raw pointers to matching actors.</returns>
		template <typename Type, IS_BASE_OF(Actor, Type)>
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

		/// <summary>
		/// Constructs and registers a new actor in the level.
		/// </summary>
		/// <typeparam name="Type">Actor class to instantiate.</typeparam>
		/// <param name="_args">Arguments forwarded to the actor constructor.</param>
		/// <returns>Pointer to the newly spawned actor.</returns>
		template <typename Type, typename ...Args, IS_BASE_OF(Actor, Type)>
		inline Type* CreateActor(Args&&... _args)
		{
			std::unique_ptr<Type> _actor = std::make_unique<Type>(std::forward<Args>(_args)...);
			Type* _rawActor = _actor.get();
			actors.push_back(std::move(_actor));
			_rawActor->Construct();
			if (isPlaying) _rawActor->BeginPlay();
			LOG(VerbosityType::Verbose, "Actor: spawned " + _rawActor->GetName().StdString() + " (" + std::string(typeid(Type).name()) + ")");
			return _rawActor;
		}

		/// <summary>
		/// Constructs and registers a new actor placed at a specific initial transform.
		/// </summary>
		/// <typeparam name="Type">Actor class to instantiate.</typeparam>
		/// <param name="_transform">Initial position, rotation, and scale.</param>
		/// <param name="_args">Arguments forwarded to constructor.</param>
		/// <returns>Pointer to the newly spawned actor.</returns>
		template <typename Type, typename ...Args, IS_BASE_OF(Actor, Type)>
		inline Type* CreateActorAt(const Transform& _transform, Args&&... _args)
		{
			std::unique_ptr<Type> _actor = std::make_unique<Type>(std::forward<Args>(_args)...);
			Type* _rawActor = _actor.get();
			actors.push_back(std::move(_actor));
			_rawActor->SetTransform(_transform);
			_rawActor->Construct();
			if (isPlaying) _rawActor->BeginPlay();
			LOG(VerbosityType::Verbose, "Actor: spawned " + _rawActor->GetName().StdString() + " (" + std::string(typeid(Type).name()) + ")");
			return _rawActor;
		}

		/// <summary>
		/// Schedules an actor for deferred destruction at the end of the frame.
		/// </summary>
		/// <param name="toMark">Pointer to actor to destroy.</param>
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

			LOG(VerbosityType::Verbose, "Actor: destroyed " + (*it)->GetName().StdString());
			pendingDeleteActors.push_back(std::move(*it));

			actors.erase(it);
		}

		/// <summary>
		/// Executes destruction on all actors marked for deletion.
		/// </summary>
		inline void DestroyPendingDeleteActors()
		{
			// BeginDestroy may destroy other actors: they are handled by the next turn of the loop.
			while (!pendingDeleteActors.empty())
			{
				std::vector<std::unique_ptr<Actor>> _dying = std::move(pendingDeleteActors);
				pendingDeleteActors.clear();
				for (const std::unique_ptr<Actor>& _actor : _dying)
					_actor->BeginDestroy();
			}
		}

		/// <summary>
		/// Default constructor for ActorManager.
		/// </summary>
		ActorManager() = default;

	private:
		/// <summary>
		/// The actors as they are now. A callback may spawn or destroy actors while the list is walked: the copy keeps the walk valid
		/// (a destroyed actor stays alive until the end of the frame, in pendingDeleteActors).
		/// </summary>
		inline std::vector<Actor*> Snapshot() const
		{
			std::vector<Actor*> _snapshot;
			_snapshot.reserve(actors.size());
			for (const std::unique_ptr<Actor>& _actor : actors)
				_snapshot.push_back(_actor.get());
			return _snapshot;
		}

	public:
		/// <summary>
		/// Starts gameplay for all managed actors, invoking their BeginPlay hooks.
		/// </summary>
		void BeginPlay();

		/// <summary>
		/// Updates all active actors each frame.
		/// </summary>
		/// <param name="_deltaTime">Elapsed frame time in seconds.</param>
		void Update(const Float& _deltaTime);

		/// <summary>
		/// Deconstructs all managed actors.
		/// </summary>
		void Deconstruct();

		/// <summary>
		/// Triggers destruction callbacks on all actors and clears actor collections.
		/// </summary>
		void BeginDestroy();
	};

}
