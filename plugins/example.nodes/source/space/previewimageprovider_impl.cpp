#include "previewimageprovider_impl.h"

#include "maxon/conditionvariable.h"

namespace maxonsdk
{
	
PreviewImageProviderExample::PreviewImageProviderExample()
{
	maxon::LinearCongruentialRandom<maxon::Float32> random;
	random.Init((maxon::UInt32)maxon::TimeValue::GetTime().GetMicroseconds());
	
	_finishedColor.r = random.Get01();
	_finishedColor.g = random.Get01();
	_finishedColor.b = random.Get01();
	
	_unfinishedColor = maxon::Color32((maxon::Float32)0.1);// dark gray
}

void PreviewImageProviderExample::SimulateWorkload(maxon::TimeValue duration)
{
	iferr_scope_handler
	{
		return;
	};

	maxon::ConditionVariableRef waitCondition = maxon::ConditionVariableRef::Create() iferr_return;
	waitCondition.Wait(duration, maxon::WAITMODE::RETURN_ON_CANCEL);
}
		
MAXON_METHOD maxon::Result<maxon::nodes::PreviewImageProviderOutput> PreviewImageProviderExample::TakeOutput()
{
	iferr_scope;
	
	const maxon::Bool isFinal = (_numIterations - _currentIteration) <= 1;
	maxon::nodes::PreviewImageProviderOutput output;
	output._iterationCount = _numIterations;
	output._currentIterationIndex = _currentIteration;

	maxon::nodes::PreviewImageProviderOutputImage image;
	image._imageSize = _imageSize;
	image._samplesFloat32Linear = _currentResult;
	image._isFinal = isFinal;
	output.SetResult(image) iferr_return;

	return output;
}

MAXON_METHOD maxon::Result<void> PreviewImageProviderExample::Initialize(maxon::DataDictionaryObjectRef request, maxon::Int numThreads)
{
	_currentIteration = -1;
	_imageSize = request.Get(maxon::nodes::PREVIEWIMAGEREQUEST::TEXTURESIZE, maxon::IntVector2d());
	
	const maxon::Bool renderProgressively = request.Get<maxon::Int>(maxon::nodes::PREVIEWIMAGEREQUEST::PROGRESSIONTHRESHOLD, maxon::Int(1)) != maxon::Int(0);
	if (renderProgressively == true)
	{
		_numIterations = 20;
		_iterationWorkload = maxon::Milliseconds(20.0);
	}
	else
	{
		// We should be really fast here. Probably the material manager expects a result and is blocking events while we render. Let's be quick!
		_numIterations = 1;
		_iterationWorkload = maxon::Milliseconds(0.0);
	}
	
	return maxon::OK;
}
		
MAXON_METHOD maxon::Result<void> PreviewImageProviderExample::ComputeIteration(const maxon::JobRef parentThread)
{
	iferr_scope;
	
	SimulateWorkload(_iterationWorkload);
	
	if (parentThread.IsCancelled())
		return maxon::OK;
	
	_currentResult = NewObj(maxon::nodes::SamplesFloat32) iferr_return;
	maxon::nodes::SamplesFloat32& result = *_currentResult;
	
	result.Resize(_imageSize.x * _imageSize.y) iferr_return;
	
	renderer::CircleBarTextureSampler nodeTextureSampler(_finishedColor, maxon::IntVector2d(_imageSize));
	
	++_currentIteration;
	const maxon::Float splitRatio = _numIterations > 1 ? ((maxon::Float)_currentIteration / maxon::Float(_numIterations - 1)) : 1.0;
	const maxon::Int splitIndex = (maxon::Int)((maxon::Float)_imageSize.x * splitRatio);
	
	for (maxon::Int y = 0; y < _imageSize.y; ++y)
	{
		const maxon::Float32 v = nodeTextureSampler.GetV(y);
		for (maxon::Int x = 0; x < _imageSize.x; ++x)
		{
			const maxon::Float32 u = nodeTextureSampler.GetU(x);
			
			maxon::nodes::SampleFloat32& sample = result[x + y * _imageSize.x];
			
			if (x < splitIndex)
			{
				const maxon::Color32 sampleColor = nodeTextureSampler.Sample(u, v);
				sample = maxon::nodes::SampleFloat32(sampleColor.GetVector(), 1);
			}
			else
			{
				sample = maxon::nodes::SampleFloat32(_unfinishedColor.GetVector(), 1);
			}
		}
	}
	
	return maxon::OK;
}

} // namespace maxonsdk
