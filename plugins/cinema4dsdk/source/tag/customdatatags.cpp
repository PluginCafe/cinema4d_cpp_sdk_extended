#include "customdatatags.h"
#include "customgui_gradient.h"
#include "c4d_basedraw.h"
#include "c4d_basedocument.h"
#include "c4d_tooldata.h"
#include "c4d_commanddata.h"
#include "c4d_resource.h"

namespace maxon
{

MAXON_DATATYPE_REGISTER(VERTEXCOLOR_MESHATTRIBUTE);
MAXON_DATATYPE_REGISTER(FLOATTYPE_MESHATTRIBUTE);

class VertexColorDisplayImpl : public Component<VertexColorDisplayImpl, CustomDataTagDisplayInterface>
{
	MAXON_COMPONENT();

public:

	VertexColorDisplayImpl() {	}
	~VertexColorDisplayImpl() { Reset();	}

	MAXON_METHOD Result<void> Init(BaseTag* tag)
	{
		return OK;
	}

	MAXON_METHOD Result<void> Draw(BaseTag* tag, BaseObject* op, BaseDraw* bd, BaseDrawHelp* bh)
	{
		if (!tag || !op || !bd || !bh)
			return IllegalArgumentError(MAXON_SOURCE_LOCATION);

		bd->SetMatrix_Screen();
		bd->SetPen(Vector(1.0, 0.0, 0.0));
		bd->DrawLine(Vector(100.0, 100.0, 0.0), Vector(200.0, 200.0, 0.0), 0);
		bd->SetMatrix_Matrix(nullptr, Matrix());

		return OK;
	}

	MAXON_METHOD Result<void> DisplayControl(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseDraw* bd, BaseDrawHelp* bh, ControlDisplayStruct& cds)
	{
		iferr_scope;

		if (!tag || !bd || !bh || !op || !op->IsInstanceOf(Opoint))
			return OK;

		CustomDataTag* customtag = static_cast<CustomDataTag*>(tag);


		CUSTOMDATATAG_MODE mode = customtag->GetMode();
		cds.perPolygonVertexColor = mode == CUSTOMDATATAG_MODE::POLYVERTEX;
		switch (mode)
		{
			case CUSTOMDATATAG_MODE::VERTEX:
			{
				Int32 pointCount = ToPoint(op)->GetPointCount();
				cds.vertex_color = NewMem(Vector32, pointCount) iferr_return;
				for (Int32 vertexIndex = 0; vertexIndex < pointCount; ++vertexIndex)
				{
					const ColorA32 col = customtag->GetVertexData<ColorA32>(vertexIndex);
					cds.vertex_color[vertexIndex] = col.GetVector().GetVector3();
				}
				break;
			}
			case CUSTOMDATATAG_MODE::POLYVERTEX:
			{
				Int32 polygonCount = ToPoly(op)->GetPolygonCount();
				cds.vertex_color = NewMem(Vector32, polygonCount * 4) iferr_return;
				for (Int32 polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
				{
					for (Int32 i = 0; i < 4; ++i)
					{
						const ColorA32& color = customtag->GetPolyVertexData<ColorA32>(polygonIndex, i);
						cds.vertex_color[4 * polygonIndex + i] = color.GetVector().GetVector3();
					}
				}
				break;
			}
			default:
				break;
		}

		return OK;
	}

	MAXON_METHOD Result<void> InitDisplayControl(BaseTag* tag, BaseDocument* doc, BaseDraw* bd, const AtomArray* active)
	{
		return OK;
	}

	MAXON_METHOD void FreeDisplayControl(BaseTag* tag)
	{
	}
	
	MAXON_METHOD void Reset()
	{
		
	}

};

class VertexColorImpl : public Component<VertexColorImpl, CustomDataTagClassInterface>
{
	MAXON_COMPONENT();
	MAXON_CUSTOMDATATAG(ID_CUSTOMDATA_TAG_VC, "Vertex Color"_s, ""_s, Id("net.maxonexample.mesh_misc.customdatatagdisplay.vertexcolor"), 0, 4);

public:

	MAXON_METHOD const DataType* GetDataType() const
	{
		return maxon::GetDataType<VERTEXCOLOR_MESHATTRIBUTE>();
	}

	MAXON_METHOD void InterpolateLinear(void* data1, const void* data2, Float blendValue) const
	{
		if (!data1 || !data2)
			return;

		ColorA32* 			v1 = static_cast<ColorA32*>(data1);
		const ColorA32* v2 = static_cast<const ColorA32*>(data2);
		*v1 = Blend(*v1, *v2, (Float32)blendValue);
	}

	MAXON_METHOD void InterpolateInOutline(void* data, const Block<void*>& outline, const Block<Float>& weights) const
	{
		if (!data)
			return;

		if (outline.GetCount() != weights.GetCount())
			return;

		ColorA32* currentValue = nullptr;
		ColorA32* v = static_cast<ColorA32*>(data);
		ColorA32 	numerator = ColorA32();
		Float			denominator = 0.0;

		for (auto it = Iterable::IndexIterator(outline); it; ++it)
		{
			currentValue = static_cast<ColorA32*>(*it);
			if (!currentValue)
				continue;

			numerator += ((*currentValue) * (Float32)weights[it.GetIndex()]);
			denominator += weights[it.GetIndex()];
		}

		denominator = Inverse(denominator);
		*v = numerator * (Float32)denominator;
	}

	MAXON_METHOD void GetDefaultValue(void* data) const
	{
		if (!data)
			return;

		ColorA32* v = static_cast<ColorA32*>(data);
		if (v)
			*v = ColorA32();
	}

	MAXON_METHOD Bool AtrLessThen(const void* data1, const void* data2) const
	{
		if (!data1 || !data2)
			return false;

		const ColorA32* v1 = static_cast<const ColorA32*>(data1);
		const ColorA32* v2 = static_cast<const ColorA32*>(data2);
		return LexicographicalCompare(v1->r, v2->r, v1->g, v2->g, v1->b, v2->b, v1->a, v2->a) == COMPARERESULT::LESS;
	}

	MAXON_METHOD Bool AtrIsEqual(const void* data1, const void* data2) const
	{
		if (!data1 || !data2)
			return false;

		const ColorA32* v1 = static_cast<const ColorA32*>(data1);
		const ColorA32* v2 = static_cast<const ColorA32*>(data2);
		return LexicographicalCompare(v1->r, v2->r, v1->g, v2->g, v1->b, v2->b, v1->a, v2->a) == COMPARERESULT::EQUAL;
	}

	MAXON_METHOD void AtrAdd(void* data1, const void* data2) const
	{
		if (!data1 || !data2)
			return;

		ColorA32* 			v1 = static_cast<ColorA32*>(data1);
		const ColorA32* v2 = static_cast<const ColorA32*>(data2);
		*v1 = *v1 + *v2;
	}

	MAXON_METHOD void AtrSubstract(void* data1, const void* data2) const
	{
		if (!data1 || !data2)
			return;

		ColorA32* 			v1 = static_cast<ColorA32*>(data1);
		const ColorA32* v2 = static_cast<const ColorA32*>(data2);
		*v1 = *v1 - *v2;
	}

	MAXON_METHOD void AtrMultiply(void* data1, const void* data2) const
	{
		if (!data1 || !data2)
			return;

		ColorA32* 			v1 = static_cast<ColorA32*>(data1);
		const ColorA32* v2 = static_cast<const ColorA32*>(data2);
		*v1 = *v1 * *v2;
	}

	MAXON_METHOD void AtrMultiply(void* data, Float value) const
	{
		if (!data)
			return;

		ColorA32* 			v1 = static_cast<ColorA32*>(data);
		*v1 = ColorA32(v1->r * (Float32)value, v1->g * (Float32)value, v1->b * (Float32)value, v1->a * (Float32)value);
	}

	MAXON_METHOD void AtrDivide(void* data1, const void* data2) const
	{
		if (!data1 || !data2)
			return;

		ColorA32* 			v1 = static_cast<ColorA32*>(data1);
		const ColorA32* v2 = static_cast<const ColorA32*>(data2);
		*v1 = ColorA32(v1->r * Inverse(v2->r), v1->g * Inverse(v2->g), v1->b * Inverse(v2->b), v1->a * Inverse(v2->a));
	}

	MAXON_METHOD void AtrDivide(void* data, Float value) const
	{
		if (!data)
			return;

		ColorA32* 			v1 = static_cast<ColorA32*>(data);
		Float32 				divisor = (Float32)Inverse(value);
		*v1 = ColorA32(v1->r * divisor, v1->g * divisor, v1->b * divisor, v1->a * divisor);
	}

	MAXON_METHOD String AtrToString(const void* data, const FormatStatement* formatStatement) const
	{
		if (!data)
			return String();

		const ColorA32* v = static_cast<const ColorA32*>(data);
		return FormatString("(@,@,@)", v->r, v->g, v->b);
	}

	MAXON_METHOD Result<void> Read(void* data, HyperFile* hf, Int32 level) const
	{
		if (!hf || !data)
			return IllegalArgumentError(MAXON_SOURCE_LOCATION);

		ColorA32* dataPtr = static_cast<ColorA32*>(data);
		Vector32 	color;
		if (!hf->ReadVector32(&color))
			return IllegalStateError(MAXON_SOURCE_LOCATION);

		Float32 alpha = 0.0;
		if (!hf->ReadFloat32(&alpha))
			return IllegalStateError(MAXON_SOURCE_LOCATION);

		*dataPtr = ColorA32(color.GetColor(), alpha);
		return OK;
	}

	MAXON_METHOD Result<void> Write(const void* data, HyperFile* hf) const
	{
		if (!data || !hf)
			return IllegalArgumentError(MAXON_SOURCE_LOCATION);

		const ColorA32* dataPtr = static_cast<const ColorA32*>(data);
		if (!dataPtr)
			return OutOfMemoryError(MAXON_SOURCE_LOCATION);

		Vector32 	color = dataPtr->GetVector().GetVector3();
		Float32 	alpha = dataPtr->a;

		if (!hf->WriteVector32(color))
			return IllegalStateError(MAXON_SOURCE_LOCATION);

		if (!hf->WriteFloat32(alpha))
			return IllegalStateError(MAXON_SOURCE_LOCATION);

		return OK;
	}

	MAXON_METHOD Int32 GetIcon(Bool perPolyVertex) const
	{
		if (perPolyVertex)
			return Ocone;
		return Osky;
	}

};

class VertexFloatDisplayImpl : public Component<VertexFloatDisplayImpl, CustomDataTagDisplayInterface>
{
	MAXON_COMPONENT();

public:

	VertexFloatDisplayImpl() {	}
	~VertexFloatDisplayImpl() { Reset();	}

	MAXON_METHOD Result<void> Init(BaseTag* tag)
	{
		if (!_gradient)
		{
			_gradient = ::Gradient::Alloc();

			GradientKnot k1, k2;
			k1.col = Vector(1.0_f, 1.0_f, 1.0_f);
			k1.pos = 0.0_f;

			k2.col = Vector(0.0_f, 0.0_f, 0.0_f);
			k2.pos = 1.0_f;

			_gradient->InsertKnot(k1);
			_gradient->InsertKnot(k2);
		}

		return OK;
	}

	MAXON_METHOD Result<void> Draw(BaseTag* tag, BaseObject* op, BaseDraw* bd, BaseDrawHelp* bh)
	{
		if (!tag || !bd || !bh || !op)
			return IllegalArgumentError(MAXON_SOURCE_LOCATION);

		bd->SetMatrix_Screen();
		bd->SetPen(Vector(1.0, 1.0, 0.0));
		bd->DrawLine(Vector(100.0, 100.0, 0.0), Vector(200.0, 200.0, 0.0), 0);
		bd->SetMatrix_Matrix(nullptr, Matrix());

		return OK;
	}

	MAXON_METHOD Result<void> DisplayControl(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseDraw* bd, BaseDrawHelp* bh, ControlDisplayStruct& cds)
	{
		iferr_scope;

		if (!tag || !bd || !bh || !op || !op->IsInstanceOf(Opoint))
			return OK;

		CustomDataTag* customtag = static_cast<CustomDataTag*>(tag);

		if (_gradient)
		{
			CUSTOMDATATAG_MODE mode = customtag->GetMode();
			cds.perPolygonVertexColor = mode == CUSTOMDATATAG_MODE::POLYVERTEX;
			switch (mode)
			{
				case CUSTOMDATATAG_MODE::VERTEX:
				{
					Int32 pointCount = ToPoint(op)->GetPointCount();
					cds.vertex_color = NewMem(Vector32, pointCount) iferr_return;
					for (Int32 vertexIndex = 0; vertexIndex < pointCount; ++vertexIndex)
					{
						const Float w = customtag->GetVertexData<Float>(vertexIndex);
						cds.vertex_color[vertexIndex] = (Vector32)_gradient->CalcGradientPixel(w);
					}
					break;
				}
				case CUSTOMDATATAG_MODE::POLYVERTEX:
				{
					Int32 polygonCount = ToPoly(op)->GetPolygonCount();
					cds.vertex_color = NewMem(Vector32, polygonCount * 4) iferr_return;

					for (Int32 polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
					{
						for (Int32 i = 0; i < 4; ++i)
						{
							const Float& v = customtag->GetPolyVertexData<Float>(polygonIndex, i);
							cds.vertex_color[4 * polygonIndex + i] = (Vector32)_gradient->CalcGradientPixel(v);
						}
					}
					break;
				}
				default:
					break;
			}

		}

		return OK;
	}

	MAXON_METHOD Result<void> InitDisplayControl(BaseTag* tag, BaseDocument* doc, BaseDraw* bd, const AtomArray* active)
	{
		if (_gradient)
		{
			InitRenderStruct irs(doc);
			iferr (_gradient->InitRender(irs))
			{
				return err;
			}
		}
		return OK;
	}

	MAXON_METHOD void FreeDisplayControl(BaseTag* tag)
	{
		if (_gradient)
			_gradient->FreeRender();
	}

	MAXON_METHOD void Reset()
	{
		if (_gradient)
			::Gradient::Free(_gradient);
	}

private:
	::Gradient* _gradient = nullptr;
};

class VertexFloatImpl : public Component<VertexFloatImpl, CustomDataTagClassInterface>
{
	MAXON_COMPONENT();
	MAXON_CUSTOMDATATAG(ID_CUSTOMDATA_TAG_FL, "Float Test"_s, "tfloatc"_s, Id("net.maxonexample.mesh_misc.customdatatagdisplay.float"), 0, 1);

public:

	MAXON_METHOD const DataType* GetDataType() const
	{
		return maxon::GetDataType<FLOATTYPE_MESHATTRIBUTE>();
	}

	MAXON_METHOD void InterpolateLinear(void* data1, const void* data2, Float blendValue) const
	{
		if (!data1 || !data2)
			return;

		Float* 				v1 = static_cast<Float*>(data1);
		const Float* 	v2 = static_cast<const Float*>(data2);
		*v1 = Blend(*v1, *v2, blendValue);
	}

	MAXON_METHOD void InterpolateInOutline(void* data, const Block<void*>& outline, const Block<Float>& weights) const
	{
		if (!data)
			return;

		if (outline.GetCount() != weights.GetCount())
			return;

		Float* 			currentValue = nullptr;
		Float* 			v = static_cast<Float*>(data);
		Float 			numerator = Float();
		Float				denominator = 0.0;

		for (auto it = Iterable::IndexIterator(outline); it; ++it)
		{
			currentValue = static_cast<Float*>(*it);
			if (!currentValue)
				continue;

			numerator += ((*currentValue) * weights[it.GetIndex()]);
			denominator += weights[it.GetIndex()];
		}

		denominator = Inverse(denominator);
		*v = numerator * denominator;
	}

	MAXON_METHOD void GetDefaultValue(void* data) const
	{
		if (!data)
			return;

		Float* v = static_cast<Float*>(data);
		if (v)
			*v = 0.0;
	}

	MAXON_METHOD Bool AtrLessThen(const void* data1, const void* data2) const
	{
		if (!data1 || !data2)
			return false;

		const Float* v1 = static_cast<const Float*>(data1);
		const Float* v2 = static_cast<const Float*>(data2);
		return LexicographicalCompare(*v1, *v2) == COMPARERESULT::LESS;
	}

	MAXON_METHOD Bool AtrIsEqual(const void* data1, const void* data2) const
	{
		if (!data1 || !data2)
			return false;

		const Float* v1 = static_cast<const Float*>(data1);
		const Float* v2 = static_cast<const Float*>(data2);
		return LexicographicalCompare(*v1, *v2) == COMPARERESULT::EQUAL;
	}

	MAXON_METHOD void AtrAdd(void* data1, const void* data2) const
	{
		if (!data1 || !data2)
			return;

		Float* 				v1 = static_cast<Float*>(data1);
		const Float* 	v2 = static_cast<const Float*>(data2);
		*v1 = *v1 + *v2;
	}

	MAXON_METHOD void AtrSubstract(void* data1, const void* data2) const
	{
		if (!data1 || !data2)
			return;

		Float* 				v1 = static_cast<Float*>(data1);
		const Float* 	v2 = static_cast<const Float*>(data2);
		*v1 = *v1 - *v2;
	}

	MAXON_METHOD void AtrMultiply(void* data1, const void* data2) const
	{
		if (!data1 || !data2)
			return;

		Float* 				v1 = static_cast<Float*>(data1);
		const Float* 	v2 = static_cast<const Float*>(data2);
		*v1 = *v1 * *v2;
	}

	MAXON_METHOD void AtrMultiply(void* data, Float value) const
	{
		if (!data)
			return;

		Float* v1 = static_cast<Float*>(data);
		*v1 = *v1 * value;
	}

	MAXON_METHOD void AtrDivide(void* data1, const void* data2) const
	{
		if (!data1 || !data2)
			return;

		Float* 				v1 = static_cast<Float*>(data1);
		const Float* 	v2 = static_cast<const Float*>(data2);
		*v1 = *v1 * Inverse(*v2);
	}

	MAXON_METHOD void AtrDivide(void* data, Float value) const
	{
		if (!data)
			return;

		Float* v1 = static_cast<Float*>(data);
		*v1 = *v1 * Inverse(value);
	}

	MAXON_METHOD String AtrToString(const void* data, const FormatStatement* formatStatement) const
	{
		const Float* v = static_cast<const Float*>(data);
		return FormatString("(@)", *v);
	}

	MAXON_METHOD Result<void> Read(void* data, HyperFile* hf, Int32 level) const
	{
		if (!hf || !data)
			return IllegalArgumentError(MAXON_SOURCE_LOCATION);

		Float* dataPtr = static_cast<Float*>(data);
		if (!dataPtr)
			return OutOfMemoryError(MAXON_SOURCE_LOCATION);

		if (!hf->ReadFloat(dataPtr))
			return IllegalStateError(MAXON_SOURCE_LOCATION);

		return OK;
	}

	MAXON_METHOD Result<void> Write(const void* data, HyperFile* hf) const
	{
		if (!data || !hf)
			return IllegalArgumentError(MAXON_SOURCE_LOCATION);

		const Float* dataPtr = static_cast<const Float*>(data);
		if (!dataPtr)
			return OutOfMemoryError(MAXON_SOURCE_LOCATION);
		
		if (!hf->WriteFloat(*dataPtr))
			return IllegalStateError(MAXON_SOURCE_LOCATION);
		
		return OK;
	}
	
	MAXON_METHOD Int32 GetIcon(Bool perPolyVertex) const
	{
		if (perPolyVertex)
			return Ocube;
		return Olight;
	}
};

MAXON_COMPONENT_OBJECT_REGISTER(VertexFloatImpl, CustomDataTagClasses::FLOAT)
MAXON_COMPONENT_CLASS_REGISTER(VertexFloatDisplayImpl, CustomDataTagDisplayClasses, "net.maxonexample.mesh_misc.customdatatagdisplay.float");

MAXON_COMPONENT_OBJECT_REGISTER(VertexColorImpl, CustomDataTagClasses::VC)
MAXON_COMPONENT_CLASS_REGISTER(VertexColorDisplayImpl, CustomDataTagDisplayClasses, "net.maxonexample.mesh_misc.customdatatagdisplay.vertexcolor");
	
} // namespace maxon

class ConvertToCustomdataTagCommand : public CommandData
{
public:

	virtual Bool Execute(BaseDocument* doc)
	{
		if (!doc)
			return false;

		AutoAlloc<AtomArray> activeTags;
		if (!activeTags)
			return false;

		doc->GetActiveTags(*activeTags);

		BaseObject* 		op = nullptr;
		BaseTag* 				currentTag = nullptr;
		BaseTag* 				newDataTag = nullptr;
		CustomDataTag* 	customDataTag = nullptr;
		for (Int32 tagIndex = 0; tagIndex < activeTags->GetCount(); ++tagIndex)
		{
			currentTag = static_cast<BaseTag*>(activeTags->GetIndex(tagIndex));
			if (!currentTag)
				continue;

			op = currentTag->GetObject();
			if (!op)
				return false;

			if (currentTag->IsInstanceOf(Tvertexmap))
			{
				newDataTag = CustomDataTag::Alloc(ID_CUSTOMDATA_TAG_FL);
				if (!newDataTag)
					return false;

				op->InsertTag(newDataTag);

				customDataTag = static_cast<CustomDataTag*>(newDataTag);

				iferr (customDataTag->InitData())
				{
					err.DbgStop();
					return false;
				}

				customDataTag->SetMode(CUSTOMDATATAG_MODE::VERTEX);

				Int32 vertexCount = ToPoly(op)->GetPointCount();
				iferr (customDataTag->Resize(vertexCount))
				{
					err.DbgStop();
					return false;
				}

				const Float32* values = static_cast<VertexMapTag*>(currentTag)->GetDataAddressR();
				if (!values)
					return false;
				for (Int32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
				{
					Float32 value = values[vertexIndex];
					customDataTag->SetVertexData<Float>(vertexIndex, std::move(value));
				}
			}
			else if (currentTag->IsInstanceOf(Tvertexcolor))
			{
				newDataTag = CustomDataTag::Alloc(ID_CUSTOMDATA_TAG_VC);
				if (!newDataTag)
					return false;

				op->InsertTag(newDataTag);

				Int32 count = 0;
				VertexColorTag* vcTag = static_cast<VertexColorTag*>(currentTag);
				customDataTag = static_cast<CustomDataTag*>(newDataTag);
				iferr (customDataTag->InitData())
				{
					err.DbgStop();
					return false;
				}

				if (vcTag->IsPerPointColor())
				{
					customDataTag->SetMode(CUSTOMDATATAG_MODE::VERTEX);
					count = ToPoly(op)->GetPointCount();

					iferr (customDataTag->Resize(count))
					{
						err.DbgStop();
						return false;
					}

					ConstVertexColorHandle values = vcTag->GetDataAddressR();
					if (!values)
						return false;
					for (Int32 vertexIndex = 0; vertexIndex < count; ++vertexIndex)
					{
						maxon::ColorA32 value = VertexColorTag::Get(values, nullptr, nullptr, vertexIndex);
						customDataTag->SetVertexData<maxon::ColorA32>(vertexIndex, std::move(value));
					}
				}
				else
				{
					customDataTag->SetMode(CUSTOMDATATAG_MODE::POLYVERTEX);
					count = ToPoly(op)->GetPolygonCount();

					iferr (customDataTag->Resize(count))
					{
						err.DbgStop();
						return false;
					}

					ConstVertexColorHandle values = vcTag->GetDataAddressR();
					if (!values)
						return false;

					VertexColorStruct vcs;
					for (Int32 polygonIndex = 0; polygonIndex < count; ++polygonIndex)
					{
						VertexColorTag::Get(values, polygonIndex, vcs);
						for (Int32 polyvertexIndex = 0; polyvertexIndex < 4; ++polyvertexIndex)
						{
							maxon::ColorA32 value = vcs[polyvertexIndex];
							customDataTag->SetPolyVertexData<maxon::ColorA32>(polygonIndex, polyvertexIndex, std::move(value));
						}
					}
				}
			}
		}
		EventAdd();
		return true;
	}
};

Bool RegisterCustomDataTagDescription()
{
	return RegisterDescription(ID_CUSTOMDATA_TAG_FL, "tfloat"_s);
}

Bool RegisterCustomDataTagCommand()
{
	return RegisterCommandPlugin(ID_CUSTOMDATA_TAG_COMMAND, "Convert To CustomDataTag"_s, 0, nullptr, "Convert To CustomDataTag"_s, NewObjClear(ConvertToCustomdataTagCommand));
}


