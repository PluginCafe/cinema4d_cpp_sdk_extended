#ifndef PREVIEWIMAGEPROVIDER_IMPL_H__
#define PREVIEWIMAGEPROVIDER_IMPL_H__

#include "maxon/previewimageprovider.h"
#include "thumbnailgenerator_impl.h"

namespace maxonsdk
{
	
class PreviewImageProviderExample : public maxon::Component<PreviewImageProviderExample, maxon::nodes::PreviewImageProviderInterface>
{
	MAXON_COMPONENT(NORMAL);
public:
	
	PreviewImageProviderExample();
	
	void SimulateWorkload(maxon::TimeValue duration);
	
	MAXON_METHOD maxon::Result<maxon::nodes::PreviewImageProviderOutput> TakeOutput();
	
	MAXON_METHOD maxon::Result<void> Initialize(maxon::DataDictionaryObjectRef request, maxon::Int numThreads);
	
	MAXON_METHOD maxon::Result<void> ComputeIteration(const maxon::JobRef parentThread);
	
private:
	maxon::Int _numIterations = 1;
	maxon::Milliseconds _iterationWorkload = maxon::Milliseconds(100.0);
	maxon::Int _currentIteration = -1;
	maxon::IntVector2d _imageSize;
	maxon::nodes::SamplesFloat32Ref _currentResult;
	
	renderer::CircleBarTextureSampler _circleTextureSampler;
	maxon::Color32 _finishedColor;
	maxon::Color32 _unfinishedColor;
};

} // namespace maxonsdk
#endif // PREVIEWIMAGEPROVIDER_IMPL_H__
