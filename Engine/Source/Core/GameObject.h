#pragma once
#include "CoreMinimal.h"
#include <typeinfo>

namespace Krampus
{
	/// <summary>
	/// Interface and base class representing an entity with a lifecycle in the game engine.
	/// </summary>
	class IGameObject : public IPrintable
	{
	protected:
		String			name		= "KrampusGameObject";
		Bool			isActive	= true;

	public:
		/// <summary>
		/// Sets the name of the game object.
		/// </summary>
		/// <param name="_name">The new name to assign.</param>
		void GetName(const String& _name)
		{
			name = _name;
		}

		/// <summary>
		/// Gets the name of the game object.
		/// </summary>
		/// <returns>The name of the game object.</returns>
		String GetName() const
		{
			return name;
		}

		void SetName(const String& _name)
		{
			name = _name;
		}

		/// <summary>
		/// Name of the reflected class (see KCLASS in Reflection/Reflection.h), or "IGameObject" if the class is not reflected.
		/// </summary>
		virtual const char* ClassName() const
		{
			return typeid(*this).name(); // compiler name; ClassNameOf() cleans it
		}

		/// <summary>
		/// Sets whether this game object is active.
		/// </summary>
		/// <param name="_active">True to activate, false to deactivate.</param>
		inline virtual void SetActive(const Bool& _active)
		{
			isActive = _active;
		}

		/// <summary>
		/// Checks whether this game object is currently active.
		/// </summary>
		/// <returns>True if active, false otherwise.</returns>
		inline bool IsActive() const
		{
			return isActive;
		}

		/// <summary>
		/// Default constructor for IGameObject.
		/// </summary>
		IGameObject() = default;

		/// <summary>
		/// Virtual destructor for IGameObject.
		/// </summary>
		virtual ~IGameObject() = default;

		/// <summary>
		/// Called when the game object is constructed.
		/// </summary>
		virtual void Construct() = 0;

		/// <summary>
		/// Called when the game object is deconstructed.
		/// </summary>
		virtual void Deconstruct() = 0;

		/// <summary>
		/// Called when gameplay begins for this game object.
		/// </summary>
		virtual void BeginPlay() = 0;

		/// <summary>
		/// Updates the game object every frame.
		/// </summary>
		/// <param name="_deltaTime">Time elapsed since the previous frame in seconds.</param>
		virtual void Tick(const Float& _deltaTime) = 0;

		/// <summary>
		/// Called just before the game object is destroyed.
		/// </summary>
		virtual void BeginDestroy() = 0;

	public:
		/// <summary>
		/// Converts the game object state to a human-readable string representation.
		/// </summary>
		/// <returns>String representation of the game object.</returns>
		virtual std::string ToString() const override
		{
			return std::format("{} -> Core : isActive = {}", name.StdString(), isActive.ToString());
		}
	};
}
