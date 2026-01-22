#pragma once
#include "GameFramework/Level.h"
#include "Utilities/System/Singleton.h"

#define ENGINE Krampus::Engine::GetInstance()

namespace Krampus
{

	class Engine : public Singleton<Engine>
	{
		bool shouldClose = false;
	public:

		Event<> onEngineStart;
		Event<> onEngineStop;
		
	public:
		INLINE void SetLanguage(const Language& _language)
		{
			language = _language;
		}
		INLINE Language GetLanguage() const
		{
			return language;
		}

		Engine();

		void Start();
		void Update();
		void Stop();
	};

}

