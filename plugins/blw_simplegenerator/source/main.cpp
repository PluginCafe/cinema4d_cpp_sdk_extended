//
//  main.cpp
//  blw_simplegenerator
//
//  Created by Riccardo Gigante on 23/07/2019.
//  Copyright © 2019 MAXON Computer GmbH. All rights reserved.
//

#include "c4d_resource.h"
#include "c4d_general.h"
#include "c4d_gui.h"
#include "c4d_objectdata.h"

#include "../blw_common/blw_crypt.h"
#include "blw_checklicense.h"
#include "main.h"

CheckLicense *g_checkLic = nullptr;

Bool PluginStart()
{
	// check for validity of CheckLicense global instance 
	if (nullptr == g_checkLic)
	{
		CriticalOutput("BLW >> Failed to allocate CheckLicense instance");
		return false;
	}
	
	// check for license validity
	iferr (g_checkLic->AnalyzeLicense())
	{
		return false;
	}
	
	// if license is valid than register the plugin
	if (g_checkLic->IsLicensed())
	{
		if (!RegisterBLWSimpleGenerator())
			return false;
	}
	
	return true;
}

void PluginEnd()
{
	DeleteObj(g_checkLic);
	g_checkLic = nullptr;
}

Bool PluginMessage(Int32 id, void* data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
			if (!g_resource.Init())
				return false;
			
			// allocate the global instance to check license validity
			iferr (g_checkLic = NewObj(CheckLicense))
			{
				CriticalOutput("BLW >> Failed to allocate CheckLicense instance [@]", err);
				return false;
			}
			
			return true;

		case C4DPL_PROGRAM_STARTED:
			// check for validity of CheckLicense global instance
			if (nullptr == g_checkLic)
			{
				CriticalOutput("BLW >> Failed to allocate CheckLicense instance");
				return false;
			}
			
			if (!g_checkLic->IsLicensed())
			{
				WarningOutput("BLW >> SimpleGenerator license is invalid or not found");
				maxon::Bool generateC4DLicReport = QuestionDialog("No valid license has been found for SimpleGenerator.\n\nExport your Cinema 4D License Information on disk to share with plugin vendor and order a valid license?"_s);
				if (generateC4DLicReport)
				{
					iferr (StoreC4DLicenseReportOnDisk())
					{
						WarningOutput("BLW >> Failed to save Cinema 4D license report on disk");
						return false;
					}
				}
			}
			
			// check for demo mode
			if (g_checkLic->IsDemo())
				MessageDialog("SimpleGenerator is running in demo mode."_s);
			
			// check for demo period
			if (g_checkLic->IsExpired())
				MessageDialog("SimpleGenerator trial period has ended."_s);
			
			return true;
	}

	return false;
}
