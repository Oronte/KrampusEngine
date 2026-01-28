#pragma once
#include "ImageWidget.h"
#include "Components/AnimationComponent.h"

namespace Krampus
{

	class GIFWidget : public ImageWidget
	{
		std::string animationName = "InitialGIFAnimation";
	protected:
		AnimationComponent* animation = nullptr;

	public:
		inline std::string GetAnimationName() const
		{
			return animationName;
		}

		GIFWidget(Level* _level, const CircleShapeData& _shapeData, const AnimationData& _animationData);
		GIFWidget(Level* _level, const RectangleShapeData& _shapeData, const AnimationData& _animationData);
	
		void Play();
		void Stop();
		void Resume();
		void Pause();

	private:
		void Init(const AnimationData& _data);
	};


}
