//
//  blw_simplegenerator.cpp
//  blw_simplegenerator
//
//  Created by Riccardo Gigante on 23/07/2019.
//  Copyright © 2019 MAXON Computer GmbH. All rights reserved.
//

#include "c4d_general.h"
#include "c4d_gui.h"
#include "c4d_baseobject.h"
#include "c4d_objectdata.h"

#include "blw_crypt.h"
#include "blw_checklicense.h"
#include "blw_simplegenerator.h"

static const Int32 ID_SDKEXAMPLE_BLW_SIMPLEGEN 	= 1040651;

extern CheckLicense* g_checkLic;

Bool BLW_SimpleGenerator::Init(GeListNode* node, Bool isCloneInit)
{
	// OPTIONAL CHECK required when certain features might be tested during the plugin execution
	iferr (g_checkLic->AnalyzeLicense())
	{
		return false;
	}
	
	DiagnosticOutput("BLW >> SimpleGenerator has valid license [@]", g_checkLic->IsLicensed());
	if (!g_checkLic->IsLicensed())
		return false;
	
	return true;
}

void BLW_SimpleGenerator::GetDimension(const BaseObject* op, Vector* mp, Vector* rad) const
{
	// OPTIONAL CHECK required when certain features might be tested during the plugin execution
	iferr (g_checkLic->AnalyzeLicense())
	{
		return;
	}
	
	DiagnosticOutput("BLW >> SimpleGenerator has valid license [@]", g_checkLic->IsLicensed());
	
	if (!g_checkLic->IsLicensed())
		return;
	
	mp->SetZero();
	rad->SetZero();
	
	if (nullptr == op)
		return;
	
	const BaseContainer* opBC = op->GetDataInstance();
	if (nullptr == opBC)
		return;
	
	rad->x = 200;
	rad->z = 200;
}

BaseObject* BLW_SimpleGenerator::GetVirtualObjects(BaseObject* op, const HierarchyHelp* hh)
{
	// OPTIONAL CHECK required when certain features might be tested during the plugin execution
	iferr (g_checkLic->AnalyzeLicense())
	{
		return nullptr;
	}
	
	// OPTIONAL CHECK required when certain features might be tested during the plugin execution
	DiagnosticOutput("BLW >> SimpleGenerator has valid license [@]", g_checkLic->IsLicensed());

	if (nullptr == op || !g_checkLic->IsLicensed())
		return BaseObject::Alloc(Onull);
	
	Bool isDirty = op->CheckCache(hh) || op->IsDirty(DIRTYFLAGS::DATA);
	if (!isDirty)
		return op->GetCache();
	
	// Retrieve the value for the object parameters
	const Float fWidth	 = 200;
	const Float fHeight	 = 200;
	
	if (fWidth == 0 || fHeight == 0)
		return BaseObject::Alloc(Onull);
	
	// Compute the number of vertices based on the width and height subdivisions
	const Int32 iVtxCnt = 4;
	
	// Compute the number of polys based on the width and height subdivisions and type of output
	Int32 iPolyCnt = 1;
	
	// Create the a PolygonObject object specifying vertices and
	// polygons count
	PolygonObject* polyObj = PolygonObject::Alloc(iVtxCnt, iPolyCnt);
	if (nullptr == polyObj)
		return BaseObject::Alloc(Onull);
	
	// Fill the PolygonObject object with the vertices and polygons
	//	indices data
	CPolygon* polygonsW = polyObj->GetPolygonW();
	if (nullptr == polygonsW)
	{
		PolygonObject::Free(polyObj);
		return BaseObject::Alloc(Onull);
	}
	
	Vector* verticesW = polyObj->GetPointW();
	if (nullptr == verticesW)
	{
		PolygonObject::Free(polyObj);
		return BaseObject::Alloc(Onull);
	}
	
	verticesW[0] = Vector(100, 0, 100);
	verticesW[1] = Vector(100, 0, -100);
	verticesW[2] = Vector(-100, 0, -100);
	verticesW[3] = Vector(-100, 0, 100);
	
	polygonsW[0] = CPolygon(0, 1, 2, 3);
	
	polyObj->Message(MSG_UPDATE);
	polyObj->MakeTag(Tphong);
	return polyObj;
}

Bool RegisterBLWSimpleGenerator();
Bool RegisterBLWSimpleGenerator()
{
	return RegisterObjectPlugin(ID_SDKEXAMPLE_BLW_SIMPLEGEN, "BLW_SimpleGenerator"_s, OBJECT_GENERATOR, BLW_SimpleGenerator::Alloc, ""_s, nullptr, 0);
}

