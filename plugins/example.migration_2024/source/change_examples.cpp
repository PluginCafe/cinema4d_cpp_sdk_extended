/*
	example.2024_migration
	(C) MAXON Computer GmbH, 2023

	Author: Ferdinand Hoppe
	Date: 22/05/2023

	Contains examples for the not directly `NodeData` related 2024.0 API changes.
*/
#include "c4d_basedocument.h"
#include "c4d_baseobject.h"
#include "c4d_baseselect.h"
#include "c4d_basetag.h"
#include "c4d_fielddata.h"
#include "customgui_field.h"
#include "customgui_gradient.h"
#include "customgui_inexclude.h"
#include "lib_description.h"

#include "maxon/apibase.h"
#include "maxon/attributetuple.h"
#include "maxon/datadescription_string.h"

#include "ofalloff_panel.h"
#include "obaselist.h"
#include "olight.h"

#include "change_examples.h"


maxon::Result<void>InstantiateDescID(BaseDocument* doc)
{
  //! [InstantiateDescID]
	iferr_scope;

	AutoAlloc<BaseObject> op(Ocube);
	CheckArgument(op);
	
	// Construct an identifier where all levels are compile-time constants, as for example here for
	// the node name. This should replace most old DescID() constructor calls, as IDs are most of the 
	// time known at compile-time.
	const DescID compileTimeId = ConstDescID(DescLevel(ID_BASELIST_NAME));

	// We can also use the macro as an rvalue in a throwaway fashion.
	GeData data;
	op->GetParameter(ConstDescID(DescLevel(ID_BASELIST_NAME)), data, DESCFLAGS_GET::NONE);
	ApplicationOutput("Name: @", data.GetString());

	// In the rare cases where the levels of a DescID are only known at runtime, we must use 
	// CreateDescID instead. We here for example construct the first five user data identifiers from
	// an array.
	for (const auto item : { 1, 2, 3, 4, 5 })
		const DescID runTimeID = CreateDescID(DescLevel(ID_USERDATA), DescLevel(item));

  //! [InstantiateDescID]
	return maxon::OK;
}

maxon::Result<void>AccessNodeDataContainer(BaseDocument* doc)
{
  //! [AccessNodeDataContainer]
	iferr_scope;

	AutoAlloc<BaseObject> op(Ocube);
	CheckArgument(op);

	// Access the data container of the scene element #op for pure read access.
	const BaseContainer readOnly = op->GetDataInstanceRef();
	const String name = readOnly.GetString(ID_BASELIST_NAME, "default"_s);

	// Access the data container of the scene element #op for read and write access.
	BaseContainer readWrite = op->GetDataInstanceRef();
	if (readWrite.GetString(ID_BASELIST_NAME, "default"_s) == "Hello World"_s)
		readWrite.SetString(ID_BASELIST_NAME, "42 is the best number"_s);

	// A data container should not be copied anymore with BaseList2D::GetData as doing this makes it
	// hard to spot cases where this is done unintentionally. Use the copy constructor or methods
	// instead.
	BaseContainer copy = BaseContainer(op->GetDataInstanceRef());
	BaseContainer alsoCopy = *op->GetDataInstanceRef().GetClone(COPYFLAGS::NONE, nullptr);
	BaseContainer yetAnotherCopy;
	op->GetDataInstanceRef().CopyTo(&yetAnotherCopy, COPYFLAGS::NONE, nullptr);
  //! [AccessNodeDataContainer]

	return maxon::OK;
}

maxon::Result<void>AccessNodeBranches(BaseDocument* doc)
{
	//! [AccessNodeBranches]
	iferr_scope;

	AutoAlloc<BaseObject> op(Ocube);
	CheckArgument(op);
	CheckArgument(op->MakeTag(Tphong));

	BaseTag* tag;

	// Branching information is now dealt with a ValueReciever which frees us from the burden of
	// having to set an upper limit of branches we want to access at most. The method now also returns
	// a Result<Bool> as all methods do which have a ValueReciever argument.

	// As always, the most convenient form of providing a value receiver is a lambda. Because it 
	// allows us to filter while searching and then break the search early once we found our data.

	// Find the first phong tag in the branches of #op, but only got into branches which actually
	// holds nodes (Cinema 4D often creates branches which are empty for later usage).
	Bool result = op->GetBranchInfo(
		[&op, &tag](const BranchInfo& info) ->maxon::Result<maxon::Bool>
		{
			ApplicationOutput("Branch of @ with name: @", op->GetName(), info.name);
			ApplicationOutput("Branch of @ with id: @", op->GetName(), info.id);

			if (info.head->GetFirst() && info.head->GetFirst()->GetType() == Tphong)
			{
				tag = static_cast<BaseTag*>(info.head->GetFirst());
				return true; // Break the search early since we found a phong tag.
			}

			return false; // Continue searching/iterating.
		},
		GETBRANCHINFO::ONLYWITHCHILDREN) iferr_return;

	ApplicationOutput("Brach traversal has been terminated early: @", result); // Will be true
	ApplicationOutput("Tag: @", tag ? tag->GetName() : nullptr);

	// Alternatively, we can also use a collection type as the ValueReciever when we are interested
	// in receiving all values.
	maxon::BaseArray<BranchInfo> branches;
	op->GetBranchInfo(branches, GETBRANCHINFO::ONLYWITHCHILDREN) iferr_return;

	// In cases where we want to replace legacy code and do not want to use a lambda with a counter
	// variable but do want to retain an upper limit of retained branches, we can use a 
	// #BufferedBaseArray. Also remember that for dealing with error results in methods without error
	// handling, we can use #iferr_scope_handler.

	auto myFunc = [&op]() -> bool
	{
		// Error handler which could be for example inside a Message() -> bool function. When an error
		// occurs, we print it to the diagnostics output and terminate.
		iferr_scope_handler
		{
			DebugOutput(maxon::OUTPUT::DIAGNOSTIC, "Error: @", err);
			return false;
		};

		// Only get the first 8 branches of #op.
		maxon::BufferedBaseArray<BranchInfo, 8> info;
		op->GetBranchInfo(info, GETBRANCHINFO::NONE) iferr_return;
		
		return true;
	};
	//! [AccessNodeBranches]
	
	UseVariable(myFunc);
	return maxon::OK;
}

maxon::Result<void>AvoidingDictionaries(BaseDocument* doc)
{
  //! [AvoidingDictionaries]
	iferr_scope;

	// An AttributeTuple can be used to replace DataDictionary instances carrying MAXON_ATTRIBUTE
	// keys. The type is much more performant than a dictionary and should be used when all required
	// fields are already known at compile-time. The type is equal in performance to native 
	// c-structures as access to its members is carried out directly without lookup or indirection.

	// Define a data structure that carries the name and tags of an object ...
	using NameTagContainer = maxon::AttributeTuple<true,
		decltype(maxon::OBJECT::BASE::NAME),
		decltype(maxon::OBJECT::BASE::TAGS)>;

	// ... and use it in a read and write fashion. The MAXON_ATTRIBUTE entities ::NAME and ::TAG 
	// are the field access keys for our #NameTagContainer data structure.
	NameTagContainer myStuff;
	myStuff[maxon::OBJECT::BASE::NAME] = "Hello world!"_s;
	myStuff[maxon::OBJECT::BASE::TAGS] = "Bob; is; your; uncle; !;"_s;

	ApplicationOutput("myStuff[maxon::OBJECT::BASE::NAME] = @", myStuff[maxon::OBJECT::BASE::NAME]);
	ApplicationOutput("myStuff[maxon::OBJECT::BASE::TAGS] = @", myStuff[maxon::OBJECT::BASE::TAGS]);
 //! [AvoidingDictionaries]
	
	return maxon::OK;
}

maxon::Result<void>CustomDataTypeAccess(BaseDocument* doc)
{
 //! [CustomDataTypeAccess]
	iferr_scope;

	AutoAlloc<BaseObject> op(Olight);
	CheckArgument(op);

	GeData data;
	op->GetParameter(ConstDescID(DescLevel(LIGHT_DETAILS_GRADIENT)), data, DESCFLAGS_GET::NONE);

	// Read only access to the falloff gradient of the light object.
	const Gradient* const a = data.GetCustomDataType<const Gradient>();

	// The legacy interface for the same operation. Should not be used anymore, but can serve as a 
	// temporary fix by adding the "I" to otherwise already modern legacy code.
	const Gradient* b = static_cast<const Gradient*>(data.GetCustomDataTypeI(CUSTOMDATATYPE_GRADIENT));

	// Read and write access to the falloff gradient of the light object, in both the new templated,
	// as well as the legacy variant.
	Gradient* const c = data.GetCustomDataTypeWritable<Gradient>();
	Gradient* d = static_cast<Gradient*>(data.GetCustomDataTypeWritableI(CUSTOMDATATYPE_GRADIENT));

	// Similar changes have also been made to BaseContainer.

	// Get the read and write container reference of the node so that we can show both read and
	// write access for custom data types. 
	BaseContainer& bc = op->GetDataInstanceRef();

	// Get read and read-write access to the object in-exclusion list of the light. For BaseContainer
	// exist I-postfix legacy methods analogously to GeData.
	const InExcludeData* const e = bc.GetCustomDataType<InExcludeData>(LIGHT_EXCLUSION_LIST);
	InExcludeData* const f = bc.GetCustomDataTypeWritableObsolete<InExcludeData>(LIGHT_EXCLUSION_LIST);
	//! [CustomDataTypeAccess]

	UseVariable(a);
	UseVariable(b);
	UseVariable(c);
	UseVariable(d);
	UseVariable(e);
	UseVariable(f);

	return maxon::OK;
}

maxon::Result<void>GradientSampling(BaseDocument* doc)
{
	//! [GradientSampling]
	iferr_scope;

	AutoAlloc<Gradient> gradient;
	CheckArgument(gradient);

	gradient->SetData(GRADIENT_MODE, GRADIENTMODE_COLORALPHA);
	Gradient* alphaGradient = gradient->GetAlphaGradient();
	CheckArgument(alphaGradient);

	// Define the gradient knots abstractly as ColorA, position tuples so that we do not have to set
	// each alpha value manually.
	using Knot = maxon::Tuple<maxon::ColorA, maxon::Float>;
	maxon::BaseArray<Knot> knotData;
	// Add a knot at position 0% of solid red, a green knot with an alpha of 50% at position 25%, and
	// finally a blue knot with an alpha of 100% at position 100%.
	knotData.Append(Knot(maxon::ColorA(1., 0., 0., 0.), 0.)) iferr_return;
	knotData.Append(Knot(maxon::ColorA(0., 1., 0., .5), .25)) iferr_return; 
	knotData.Append(Knot(maxon::ColorA(0., 0., 1., 1.), 1.)) iferr_return; 

	// Write the knot data into the gradient.
	for (const auto& item : knotData)
	{
		// Write the color into the color gradient.
		maxon::GradientKnot color;
		color.col = maxon::Color(item.first.r, item.first.g, item.first.b);
		color.pos = item.second;
		gradient->InsertKnot(color);

		// Write the alpha into the color gradient.
		maxon::GradientKnot alpha;
		alpha.col = maxon::Color(item.first.a);
		alpha.pos = item.second;
		alphaGradient->InsertKnot(alpha);
	}

	// Sample the color portion of a gradient alone using ::PrepareRenderData.
	InitRenderStruct irs;
	irs.doc = GetActiveDocument();

	
	maxon::GradientRenderData renderData = gradient->PrepareRenderData(irs) iferr_return;
	maxon::Color color0 = renderData.CalcGradientPixel(0.);
	maxon::Color color1 = renderData.CalcGradientPixel(1.);

	ApplicationOutput("Sampling color information only: color0 = @, color1 = @", color0, color1);

	// Sample the color and the alpha portion of a gradient alone using ::PrepareRenderDataWithAlpha.
	// A GradientRenderDataTuple is just a Tuple<GradientRenderData, GradientRenderData> where the
	// first element for evaluating the color gradient and second for evaluating the alpha gradient.
	GradientRenderDataTuple renderDataTuple = gradient->PrepareRenderDataWithAlpha(irs) iferr_return;
	color0 = renderDataTuple.first.CalcGradientPixel(0.);
	color1 = renderDataTuple.first.CalcGradientPixel(1.);
	maxon::Color alpha0 = renderDataTuple.second.CalcGradientPixel(0.);
	maxon::Color alpha1 = renderDataTuple.second.CalcGradientPixel(1.);

	ApplicationOutput(
		"Sampling color and alpha information: color0 = @, color1 = @, alpha0 = @, alpha1 = @", 
		color0, color1, alpha0, alpha1);
	//! [GradientSampling]

	return maxon::OK;
}

maxon::Result<void>FieldSampling(BaseDocument* doc)
{
	iferr_scope;

	// Allocate the effector and field objects and insert them into the document.
	BaseObject* const effector = BaseObject::Alloc(Omgplain);
	CheckArgument(effector);
	FieldObject* const field = FieldObject::Alloc(Fspherical);
	CheckArgument(field);

	doc->InsertObject(effector, nullptr, nullptr);
	doc->InsertObject(field, nullptr, nullptr);

	// Get the field list for the effector.
	GeData data;
	if (!effector->GetParameter(ConstDescID(DescLevel(FIELDS)), data, DESCFLAGS_GET::NONE))
		return maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "Could not access field list."_s);

	FieldList* fieldList = data.GetCustomDataTypeWritable<FieldList>();
	if (!fieldList)
		return maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "Could not access field list."_s);

	// Create a new #FieldLayer for field objects in the list and link our #field to it.
	FieldLayer* const layer = FieldLayer::Alloc(FLfield);
	CheckArgument(layer);

	fieldList->InsertLayer(layer, nullptr, nullptr) iferr_return;
	layer->SetLinkedObject(field);

	{
	//! [FieldSampling]
	iferr_scope;

	// #field is a #FieldObject sampled by the #BaseObject #effector. See the full example for details.

	// The input location we want to sample and the output data. We are only interested in sampling
	// FIELDSAMPLE_FLAG::VALUE, i.e., the field influence value of the field at point x.
	FieldInput inputs(Vector(0, 50, 0));
	FieldOutput outputs;
	outputs.Resize(inputs.GetCount(), FIELDSAMPLE_FLAG::VALUE) iferr_return;
	FieldOutputBlock outputBlock = outputs.GetBlock();

	// Create the field info for sampling the sample data #inputs for the caller #effector.
	const FieldInfo info = FieldInfo::Create(effector, inputs, FIELDSAMPLE_FLAG::VALUE) iferr_return;

	// Sample the field. In 2024.0 we now must pass on the extra data generated by the sampling 
	// initialization so that #field can remain const for this operation.
	maxon::GenericData extraData = field->InitSampling(info) iferr_return;
	field->Sample(inputs, outputBlock, info, extraData, FIELDOBJECTSAMPLE_FLAG::NONE) iferr_return;

	// Iterate over the output values.
	for (const maxon::Float value : outputBlock._value)
		ApplicationOutput("Sampled value: @", value);

	field->FreeSampling(info, extraData);
	//! [FieldSampling]
	}

	EventAdd();
	return maxon::OK;
}

maxon::Result<void>CopyOnWriteSceneData(BaseDocument* doc)
{
	iferr_scope;

	// Allocate a cube object and build and access its generator cache.
	BaseObject* const cube = BaseObject::Alloc(Ocube);
	CheckArgument(cube);

	doc->InsertObject(cube, nullptr, nullptr);
	doc->ExecutePasses(nullptr, false, false, true, BUILDFLAGS::NONE);
	if (!cube->GetCache())
		return maxon::UnexpectedError(
			MAXON_SOURCE_LOCATION, "Could not build cube object generator cache."_s);

	// Copy the cache of #cube.
	PolygonObject* const polyCube = static_cast<PolygonObject*>(
		cube->GetCache()->GetClone(COPYFLAGS::NONE, nullptr));
	CheckArgument(polyCube);

	// Add a point selection tag to #polyCube and select the 2nd vertex in it.
	SelectionTag* const sTag = static_cast<SelectionTag*>(polyCube->MakeTag(Tpointselection));
	CheckArgument(sTag);
	BaseSelect* const selection = sTag->GetWritableBaseSelect();
	CheckArgument(selection);
	selection->Select(1);

	{
	//! [CopyOnWriteSceneData]
	iferr_scope;

	// #polyCube is a PolygonObject, see the full example for details. We get the point data of the
	// object stored in its #Tpoint tag.
	VariableTag* const pointTag = static_cast<VariableTag*>(polyCube->GetTag(Tpoint));
	CheckArgument(pointTag);

	// Requesting access to the writable data of a VariableTag with ::GetLowlevelDataAddressW will
	// always trigger a copy being made when data is being shared between multiple entities; no matter
	// if write operations actually occur or not. We should therefore be very conservative with calling
	// this method.
	
	// Will copy the point data of #pointTag when the reference count is higher than one, although
	// we actually do not write any data.
	Vector* points = reinterpret_cast<Vector*>(pointTag->GetLowlevelDataAddressW());
	CheckArgument(points);
	ApplicationOutput("points[0] = @", points[0]);

	// Similarly, selection states as represented by the type BaseSelect are manged in a COW-manner 
	// too. This applies to implicitly stored selection states such as the selected and hidden element
	// states on #Point-, #Spline-, and #PolygonObject instances.

	// Access the point selection state of #polyCube for read-only purposes.
	const BaseSelect* const readPointSelection = polyCube->GetPointS();
	// Access the point selection state of #polyCube for read-write purposes.
	BaseSelect* const readWritePointSelection = polyCube->GetWritablePointS();

	// And it applies to explicitly stored selection states in form of Selection tags. Data being 
	// shared among multiple entities is more likely here due to the long-term storage nature of 
	// selection tags.

	// Attempt to get the first point selection tag on #polyCube.
	SelectionTag* const selectionTag = static_cast<SelectionTag*>(polyCube->GetTag(Tpointselection));
	CheckArgument(selectionTag);

	// Access read-only data by using #GetBaseSelect, shared data references cannot be severed, as
	// it is not possible to accidentally call one of the methods causing that.
	const BaseSelect* const readStoredPointSelection = selectionTag->GetBaseSelect();
	for (Int32 i = 0; i < readStoredPointSelection->GetCount(); i++)
		ApplicationOutput("Point @ is selected: @", i, readStoredPointSelection->IsSelected(i));

	// Opposed to #VariableTag read-write access, accessing a writable #BaseSelect instance with 
	// methods like #::GetWritablePointS or #::GetWritableBaseSelect will not automatically cause
	// the internal data to be copied. Only when invoking one of the non-const methods of #BaseSelect
	// that modify the state of the selection, such as Select(All), Deselect(All), Toggle(All), 
	// CopyTo, etc., will the COW-mechanism trigger and copy the data when it is also referenced by 
	// others.

	// Reading data on a writable BaseSelect will never cause COW copies to trigger.
	BaseSelect* const readWriteStoredPointSelection = selectionTag->GetWritableBaseSelect();
	for (Int32 i = 0; i < readWriteStoredPointSelection->GetCount(); i++)
		ApplicationOutput("Point @ is selected: @", i, readWriteStoredPointSelection->IsSelected(i));

	// But calling for example #Select on a shared BaseSelect will.
	readWriteStoredPointSelection->Select(0);

	//! [CopyOnWriteSceneData]
	UseVariable(readPointSelection);
	UseVariable(readWritePointSelection);
	}

	EventAdd();
	return maxon::OK;
}

maxon::Result<void>CastingStyles(BaseDocument* doc)
{
	//! [CastingStyles]
	iferr_scope;

	BaseObject* op = BaseObject::Alloc(Ocube);
	CheckArgument(op);

	// C-style casts should be avoided in favor of explicit C++ casts.

	const BaseObject* constOp = const_cast<BaseObject*>(op);
	void* data = reinterpret_cast<void*>(op);

	BaseList2D* a = (BaseList2D*)op;                             // No
	BaseList2D* b = static_cast<BaseList2D*>(op);                // Yes

	BaseList2D* c = (BaseList2D*)data;                           // No
	BaseList2D* d = reinterpret_cast<BaseList2D*>(data);         // Yes

	BaseObject* e = (BaseObject*)constOp;                        // No
	BaseObject* f = const_cast<BaseObject*>(constOp);            // Yes

	// C-style casts between fundamental types should be avoided in favor of conversion constructors.

	Float32 floatValue = 3.14F;
	maxon::CString cString = "Hello World!"_cs;

	Int32 g = (Int32)floatValue;                                 // No
	Int32 h = Int32(floatValue);                                 // Yes

	String i = (String)cString;                                  // No
	String j = String(cString);                                  // Yes
	//! [CastingStyles]
	
	UseVariable(a);
	UseVariable(b);
	UseVariable(c);
	UseVariable(d);
	UseVariable(e);
	UseVariable(f);
	UseVariable(g);
	UseVariable(h);
	UseVariable(i);
	UseVariable(j);

	finally
	{
		BaseObject::Free(op);
	};

	return maxon::OK;
}
