#pragma once
#include "DiegeticImageWidget.h"
#include "Components/AnimationComponent.h"

namespace Krampus
{

	class DiegeticGIFWidget : public DiegeticImageWidget
	{
		String					animationName	= "InitialGIFAnimation";

	protected:
		AnimationComponent*		animation		= nullptr;

	public:
		inline String GetAnimationName() const
		{
			return animationName;
		}

		DiegeticGIFWidget(Level* _level, const CircleShapeData& _shapeData, const AnimationData& _animationData);
		DiegeticGIFWidget(Level* _level, const RectangleShapeData& _shapeData, const AnimationData& _animationData);
	
		void Play();
		void Stop();
		void Resume();
		void Pause();

	private:
		void Init(const AnimationData& _data);

	public:
		virtual std::string ToString() const override;
	};


}
