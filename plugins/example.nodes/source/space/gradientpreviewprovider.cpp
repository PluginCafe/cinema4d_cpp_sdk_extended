#include "gradientpreviewprovider.h"

#include "customnode-customnodespace_descriptions.h"
#include "maxon/datadescription_nodes.h"
#include "maxon/nodesgraph.h"
#include "maxon/nodesgraph_helpers.h"
#include "maxon/valuereceiver.h"

namespace maxonsdk
{

GradientPreviewProvider::GradientPreviewProvider()
{

}

maxon::Result<maxon::nodes::PreviewImageProviderOutput> GradientPreviewProvider::TakeOutput()
{
	iferr_scope;
	CheckState(_gradientBufferRef != nullptr);

	maxon::nodes::PreviewImageProviderOutput output;
	output._iterationCount = 1;
	output._currentIterationIndex = 0;

	maxon::nodes::PreviewImageProviderOutputImage image;
	image._imageSize = maxon::IntVector2d(_gradientBufferRef->GetCount(), 1);
	image._samplesFloat32Linear = _gradientBufferRef;
	image._isFinal = true;
	output.SetResult(image) iferr_return;
	return output;
}

maxon::Result<void> GradientPreviewProvider::Initialize(maxon::DataDictionaryObjectRef request, maxon::Int numThreads)
{
	iferr_scope_handler
	{
		err.CritStop();
		return err;
	};

	MAXON_SCOPE // Clone the graph now and process later asynchronously
	{
		maxon::GraphNode gradientBundle = request.Get(maxon::nodes::PREVIEWIMAGEREQUEST::SUBJECT, maxon::GraphNode());
		const maxon::GraphModelRef& graph = gradientBundle.GetGraph();
		maxon::nodes::NodesGraphModelRef nodeGraph = maxon::Cast<maxon::nodes::NodesGraphModelRef>(graph);
		maxon::nodes::NodesGraphModelRef nodeGraphClone = nodeGraph.Clone() iferr_return;
		maxon::nodes::NodesGraphHelpersInterface::SetOriginal(nodeGraphClone, nodeGraph) iferr_return;
		_clonedGradientBundle = nodeGraphClone.GetNode(gradientBundle.GetPath());
	}

	MAXON_SCOPE // We extract other settings right away
	{
		// We ignore the vRange because our knots are all constantly colored and we therefore have a 1D gradient.
		// const maxon::Range<maxon::Float> vRange = request.Get(maxon::nodes::PREVIEWIMAGEREQUEST::VRANGE, maxon::Range<maxon::Float>());
		_gradientPreviewSize = request.Get(maxon::nodes::PREVIEWIMAGEREQUEST::TEXTURESIZE, maxon::IntVector2d());
		_gradientPreviewSize.y = 1;

		_gradientURange = request.Get(maxon::nodes::PREVIEWIMAGEREQUEST::URANGE, maxon::Range<maxon::Float>());
		CheckArgument(_gradientURange.GetMin() <= _gradientURange.GetMax(), "Invalid u range");
	}

	return maxon::OK;
}

maxon::Result<void> GradientPreviewProvider::ComputeIteration(const maxon::JobRef parentThread)
{
	iferr_scope;

	const GradientBakingData gradientData = InitializeGradientData(_clonedGradientBundle) iferr_return;

	// We can now drop the graph.
	_clonedGradientBundle = maxon::GraphNode();

	// We bake with the extracted data.
	_gradientBufferRef = BakeGradient(gradientData, _gradientPreviewSize, _gradientURange) iferr_return;

	return maxon::OK;
}

maxon::Result<GradientBakingData> GradientPreviewProvider::InitializeGradientData(const maxon::GraphNode& gradientBundle)
{
	iferr_scope;

	GradientBakingData gradientData;
	gradientBundle.GetChildren(
	[&gradientData](const maxon::GraphNode& bundleKnot) -> maxon::Result<maxon::Bool>
	{
		iferr_scope;
		maxon::GraphNode colorPort = bundleKnot.FindChild(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::COLOR) iferr_return;
		maxon::GraphNode positionPort = bundleKnot.FindChild(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::POSITION) iferr_return;
		maxon::GraphNode biasPort = bundleKnot.FindChild(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::BIAS) iferr_return;
		maxon::GraphNode interpolationPort = bundleKnot.FindChild(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::INTERPOLATION) iferr_return;

		const maxon::Float position = positionPort.GetEffectivePortValue<maxon::Float>().GetValueOrDefault() iferr_return;
		gradientData._sortedKnots.InsertKnot(maxon::Float32(position)) iferr_return;

		// We only support fixed values for these ports and can store them right away as a result.
		GradientBakingData::KnotData knotData;
		const maxon::ColorA colorValue = colorPort.GetEffectivePortValue<maxon::ColorA>().GetValueOrDefault() iferr_return;
		knotData._value = maxon::ColorA32(colorValue);

		const maxon::Float biasValue = biasPort.GetEffectivePortValue<maxon::Float>().GetValueOrDefault() iferr_return;
		knotData._bias = maxon::Float32(biasValue);

		const maxon::InternedId interpolationMode = interpolationPort.GetEffectivePortValue<maxon::InternedId>().GetValueOrDefault() iferr_return;
		knotData._interpolationMode = interpolationMode.GetHashCode();
		gradientData._knotsData.Append(std::move(knotData)) iferr_return;

		return true;
	}) iferr_return;

	gradientData._sortedKnots.Sort() iferr_return;

	return gradientData;
}

maxon::Result<maxon::nodes::SamplesFloat32Ref> GradientPreviewProvider::BakeGradient(const GradientBakingData& gradientData, const maxon::IntVector2d& gradientPreviewSize, const maxon::Range<maxon::Float>& gradientURange)
{
	iferr_scope;

	using InterpolationNone = decltype(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::INTERPOLATION_ENUM_NONE);
	using InterpolationLinearKnot = decltype(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::INTERPOLATION_ENUM_LINEARKNOT);
	using InterpolationSmoothKnot = decltype(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::INTERPOLATION_ENUM_SMOOTHKNOT);
	using InterpolationBlend = decltype(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::INTERPOLATION_ENUM_BLEND);
	using InterpolationCubicKnot = decltype(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::INTERPOLATION_ENUM_CUBICKNOT);
	using InterpolationCubicBias = decltype(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::INTERPOLATION_ENUM_CUBICBIAS);
	static const maxon::GradientSampler<InterpolationNone, InterpolationLinearKnot, InterpolationSmoothKnot, InterpolationBlend, InterpolationCubicKnot, InterpolationCubicBias> gradientSampler;

	auto GetKnotValue = [&gradientData](Int32 index) -> maxon::Result<maxon::ColorA32>
	{
		return gradientData._knotsData[index]._value;
	};

	auto GetKnotInterpolationMode = [&gradientData](Int32 index) -> maxon::Result<UInt>
	{
		return gradientData._knotsData[index]._interpolationMode;
	};

	auto GetKnotBias = [&gradientData](Int32 index) -> maxon::Result<Float32>
	{
		return gradientData._knotsData[index]._bias;
	};
	maxon::nodes::SamplesFloat32Ref gradientBufferRef = NewObj(maxon::nodes::SamplesFloat32) iferr_return;
	maxon::nodes::SamplesFloat32& gradientBuffer = *gradientBufferRef;
	gradientBuffer.Resize(gradientPreviewSize.x) iferr_return;

	const Float32 uStep = Float32(gradientURange.GetMax() - gradientURange.GetMin()) / Float32(gradientPreviewSize.x);
	const Float32 uPixelCenterOffset = uStep / Float32(2.0);
	const Float32 uOffset = Float32(gradientURange.GetMin()) + uPixelCenterOffset;
	for (Int x = 0; x < gradientPreviewSize.x; ++x)
	{
		const Float32 samplePosition = uOffset + uStep * Float32(x);
		maxon::ColorA32 sample = gradientSampler.Sample<maxon::ColorA32>(samplePosition, gradientData._sortedKnots, GetKnotValue, GetKnotInterpolationMode, GetKnotBias) iferr_return;
		gradientBuffer[x] = maxon::nodes::SampleFloat32(sample);
	}

	return gradientBufferRef;
}

} // namespace maxonsdk
