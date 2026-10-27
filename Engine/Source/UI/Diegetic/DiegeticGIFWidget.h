#pragma once
#include "DiegeticImageWidget.h"
#include "Components/AnimationComponent.h"

namespace Krampus
{

	/// <summary>
	/// Animated in-world diegetic image widget playing sprite frame sequences.
	/// </summary>
	class DiegeticGIFWidget : public DiegeticImageWidget
	{
		String					animationName	= "InitialGIFAnimation";

	protected:
		AnimationComponent*		animation		= nullptr;

	public:
		/// <summary>Gets the name of the active GIF animation sequence.</summary>
		/// <returns>Animation name string.</returns>
		inline String GetAnimationName() const
		{
			return animationName;
		}

		/// <summary>Constructs a circular diegetic GIF widget.</summary>
		/// <param name="_level">Owning Level.</param>
		/// <param name="_shapeData">Circle shape parameters.</param>
		/// <param name="_animationData">Animation frame configuration.</param>
		DiegeticGIFWidget(Level* _level, const CircleShapeData& _shapeData, const AnimationData& _animationData);

		/// <summary>Constructs a rectangular diegetic GIF widget.</summary>
		/// <param name="_level">Owning Level.</param>
		/// <param name="_shapeData">Rectangle shape parameters.</param>
		/// <param name="_animationData">Animation frame configuration.</param>
		DiegeticGIFWidget(Level* _level, const RectangleShapeData& _shapeData, const AnimationData& _animationData);
	
		/// <summary>Starts playing the animation.</summary>
		void Play();
		/// <summary>Stops the animation and resets to first frame.</summary>
		void Stop();
		/// <summary>Resumes paused animation.</summary>
		void Resume();
		/// <summary>Pauses the animation.</summary>
		void Pause();

	private:
		void Init(const AnimationData& _data);

	public:
		/// <summary>Returns a string representation of the diegetic GIF widget.</summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;
	};

}
