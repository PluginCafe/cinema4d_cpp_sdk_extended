#include "thumbnailgenerator_impl.h"

namespace maxonsdk
{

namespace renderer
{
maxon::Color32 CircleBarNode::Sample(const maxon::Vector2d32& uv) const
{
	const maxon::Vector2d32 center(maxon::Float32(0.5));
	const maxon::Float32 distanceToCenter = (uv - center).GetSquaredLength();
	const maxon::Float32 distanceToBar = (maxon::Vector2d32(0.5, uv.y) - center).GetSquaredLength();
	
	const maxon::Bool isInsideCircle = (distanceToCenter < _radiusSquared);
	const maxon::Bool isOutsideBar = distanceToBar > _radiusSquared * 0.01;
	
	const maxon::Color32 color = (isInsideCircle == true && isOutsideBar == true) ? _foreground : _background;
	return color;
}

	
maxon::Color32 CircleBarTextureSampler::Sample(maxon::Float32 u, maxon::Float32 v) const
{
	return _circleBar.Sample(maxon::Vector2d32(u, v));
}

maxon::Float32 CircleBarTextureSampler::GetU(maxon::Int pixelX) const
{
	const maxon::Float32 u = _uRange.GetMin() + _halfSample.x + _uStep * (maxon::Float32)pixelX;
	return u;
}

maxon::Float32 CircleBarTextureSampler::GetV(maxon::Int pixelY) const
{
	
	const maxon::Float32 v = _vRange.GetMin() + _halfSample.y + _vStep * (maxon::Float32)pixelY;
	return v;
}

maxon::Result<maxon::nodes::SamplesFloat32ConstRef> CircleBarTextureSampler::BakeColoredTexture(const maxon::Color32& color, const maxon::IntVector2d& textureSize, const maxon::JobRef& parentThread)
{
	iferr_scope;
	
	maxon::nodes::SamplesFloat32Ref writeImageRef = NewObj(maxon::nodes::SamplesFloat32) iferr_return;
	maxon::nodes::SamplesFloat32& writeImage = *writeImageRef;
	writeImage.Resize(textureSize.x * textureSize.y) iferr_return;
	
	maxon::PseudoRandom<maxon::Float32> random;
	random.Init((maxon::UInt32)maxon::TimeValue::GetTime().GetMicroseconds());
	
	const renderer::CircleBarTextureSampler nodeSampler(color, maxon::IntVector2d(textureSize));
	
	for (maxon::Int y = 0; y < textureSize.y; ++y)
	{
		const maxon::Float32 v = nodeSampler.GetV(y);
		for (maxon::Int x = 0; x < textureSize.x; ++x)
		{
			if ((x & 15) != 0 && parentThread.IsCancelled() == true)
			{
				return maxon::nodes::SamplesFloat32ConstRef();
			}
			
			const maxon::Float32 u = nodeSampler.GetU(x);
			
			maxon::nodes::SampleFloat32& writeSample = writeImage[x + textureSize.x * y];
			
			maxon::Color32 colorSample = nodeSampler.Sample(u, v);
			writeSample = maxon::nodes::SampleFloat32(colorSample.GetVector(), 1.0);
		}
	}
	
	maxon::nodes::SamplesFloat32ConstRef image = writeImageRef;
	return image;
}
	
} // namespace renderer
	
} // namespace maxonsdk
