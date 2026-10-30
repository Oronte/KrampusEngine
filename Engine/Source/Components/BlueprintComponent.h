#pragma once
#include "Component.h"
#include "Blueprint/Blueprint.h"

namespace Krampus
{

	/// <summary>
	/// Runs a blueprint (Content/Blueprints/Name.kbp, made in the Blueprint Editor) for its actor: its Event BeginPlay when the game
	/// starts, its Event Tick every frame, its Event Input nodes on the inputs, its Custom Events when called (CallEvent).
	/// Nothing runs while the editor edits the scene.
	/// </summary>
	class BlueprintComponent : public Component
	{
		BlueprintInstance								instance;
		std::vector<Event<>::ListenerHandle>			inputHandles;
		bool											started		= false;

	public:
		/// <summary>Name of the blueprint (folders included, no extension): "Enemies/Bat".</summary>
		std::string		blueprint;

		BlueprintComponent(Actor* _owner);

		void BeginPlay() override;
		void Tick(const Float& _deltaTime) override;
		void Deconstruct() override;

		/// <summary>Starts the Custom Events of this name.</summary>
		void CallEvent(const std::string& _name);

		/// <summary>Values of the variables of the blueprint while it runs.</summary>
		BpValue GetVariable(const std::string& _name) const { return instance.GetVariable(_name); }
		void SetVariable(const std::string& _name, const BpValue& _value) { instance.SetVariable(_name, _value); }

		/// <summary>The running blueprint (the editor lights the nodes that run).</summary>
		const BlueprintInstance& GetInstance() const { return instance; }
		bool IsRunning() const { return started && instance.IsReady(); }

		std::string ToString() const override;

	private:
		void Start();
		void ListenToInputs();
		void StopListening();
	};
}
