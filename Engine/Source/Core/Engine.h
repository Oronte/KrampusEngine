#pragma once
#include "GameFramework/Level.h"
#include "Utilities/System/Singleton.h"
#include "Utilities/System/Language.h"
#include "Managers/TranslationManager.h"

#define ENGINE Krampus::Engine::GetInstance()

namespace Krampus
{

	class Engine : public Singleton<Engine>
	{
		bool shouldClose = false;
		Language language = Language::English;
		TranslationManager translationManager;
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

