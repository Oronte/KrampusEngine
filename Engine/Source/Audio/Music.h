#pragma once
#include "Sample.h"

namespace Krampus
{

	/// <summary>
	/// A music streamed from its file (or from the asset pack of a shipped game) while it plays.
	/// </summary>
	class Music : public Sample<sf::Music>
	{
	public:
		Music() : Sample(std::make_unique<sf::Music>()) {}

		/// <summary>Opens a music file; the pack is asked first, then the disk.</summary>
		/// <returns>False when the file cannot be read.</returns>
		bool Open(const String& _path);

		/// <summary>Length of the opened music, in seconds.</summary>
		[[nodiscard]] float GetDuration() const { return source->getDuration().asSeconds(); }
	};

}
