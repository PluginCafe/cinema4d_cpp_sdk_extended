//
//  blw_checklicense.cpp
//  blw_simplegenerator
//
//  Created by Riccardo Gigante on 23/07/2019.
//  Copyright © 2019 MAXON Computer GmbH. All rights reserved.
//

#include "maxon/file_utilities.h"


#include "blw_crypt.h"
#include "blw_checklicense.h"

maxon::Result<maxon::String> CheckLicense::RetrieveMD5FromUrl(const maxon::Url &in_url)
{
	iferr_scope;
	
	// check passed arguments
	if (in_url.IsEmpty())
		return maxon::IllegalArgumentError(MAXON_SOURCE_LOCATION);
	
	// check if MD5 has been already computed to return it swiftly
	if (_md5Url.IsEmpty())
	{
		// allocate a BaseArray to store the read binary
		maxon::BaseArray<maxon::Char> source;
		maxon::FileUtilities::ReadToArray(in_url, source) iferr_return;
		
		// prepare target buffer
		maxon::BaseArray<maxon::UChar> hash;
		
		// MD5
		const maxon::StreamConversionRef md5 = maxon::StreamConversions::HashMD5().Create() iferr_return;
		md5.ConvertAll(source, hash) iferr_return;
		_md5Url = maxon::GetHashString(hash) iferr_return;
		hash.Reset();
	}
	return _md5Url;
}

maxon::Result<void> CheckLicense::AnalyzeLicense()
{
	iferr_scope;
	
	maxon::String prodID, sysID, userID, licID, username;
	GetGeneralLicensingInformation(prodID, sysID, userID, licID, username) iferr_return;
	
	// get the Cinema 4D Preference Folder
	const maxon::String c4dPrefFolderPath = GeGetC4DPath(C4D_PATH_PREFS).GetString();
	const maxon::Url c4dPrefsFolder (c4dPrefFolderPath);
	
	// get plugin location
	const maxon::Url pluginUrl = maxon::g_maxon.GetUrl();
	
	// get plugin folder
	const maxon::Url pluginFolder = pluginUrl.GetDirectory();
	
	// hash the systemID (note that system ID length can vary depending on the platform)
	const maxon::String hashSysID = maxon::GetPasswordHash(sysID, maxon::StreamConversions::HashCrc32c()) iferr_return;
	
	// create an URL of the plugin license assembling the C4D preference folder and the system ID
	maxon::Url pluginLicUrl = (c4dPrefsFolder + FormatString("PluginLicensingInfo@.lic"_s, hashSysID)) iferr_return;
	
	// check if license file with system ID is available in preference folder
	if (pluginLicUrl.IoDetect() != maxon::IODETECT::FILE)
	{
		pluginLicUrl = (c4dPrefsFolder + "PluginLicensingInfo.lic"_s) iferr_return;
		
		// check if license file without system ID is available in preference folder
		if (pluginLicUrl.IoDetect() != maxon::IODETECT::FILE)
		{
			pluginLicUrl = (pluginFolder + FormatString("PluginLicensingInfo@.lic"_s, hashSysID)) iferr_return;
			
			// check if license file with system ID is available in plugin binary folder
			if (pluginLicUrl.IoDetect() != maxon::IODETECT::FILE)
			{
				pluginLicUrl = (pluginFolder + "PluginLicensingInfo.lic"_s) iferr_return;
				
				// check if license file without system ID is available in plugin binary folder
				if (pluginLicUrl.IoDetect() != maxon::IODETECT::FILE)
				{
					WarningOutput("BLW >> SimpleGenerator license not found either in C4D Preferences Folder and in the plugin binary folder");
					return maxon::OK;
				}
			}
		}
	}
	
	// retrieve MD5 checksum of the encryption key
	const maxon::String hashedKey = CreateMD5FromString(KEYSTRING) iferr_return;
	
	// read the plugin license file and return the string containing the license data
	const maxon::String pluginLicString = ReadAndDecrypt(hashedKey, pluginLicUrl) iferr_return;
	
	// check for data existence on returned string
	if (pluginLicString.IsEmpty())
	{
		return maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "No data returned upon reading and decrypting the plugin license file"_s);
	}
	
	// parse the license string in a more representative DataDictionary
	const maxon::DataDictionary pluginLicDict = ImportJSONFromString(pluginLicString) iferr_return;
	
	// compare the plugin signature
	const maxon::String licSignature = pluginLicDict.Get<maxon::String>("signature"_s) iferr_return;
	if (!licSignature.IsEqual(LICSIG))
	{
		WarningOutput("BLW >> SimpleGenerator license is invalid: signature not matching");
		return maxon::OK;
	}
	
	// compare the plugin dll md5
	// retrieve the MD5 for the plugin dll
	// NOTE: the retrieved string is LOWER CASE!
	const maxon::String md5Lib = RetrieveMD5FromUrl(pluginUrl) iferr_return;
#ifdef MAXON_TARGET_MACOS
	const maxon::String licMD5Lib = pluginLicDict.Get<maxon::String>("md5libosx"_s) iferr_return;
#elif MAXON_TARGET_WINDOWS
	const maxon::String licMD5Lib = pluginLicDict.Get<maxon::String>("md5libwin"_s) iferr_return;
#elif MAXON_TARGET_LINUX
	const maxon::String licMD5Lib = pluginLicDict.Get<maxon::String>("md5liblin"_s) iferr_return;
#endif
	if (!licMD5Lib.ToLower().IsEqual(md5Lib))
	{
		WarningOutput("BLW >> SimpleGenerator license is invalid: md5 not matching");
		return maxon::OK;
	}
	
	// check for demo period by retrieving plugin license timestamp and demo period duration
	const maxon::Int64 licTimeStamp = pluginLicDict.Get<maxon::Int64>("timestamp"_s) iferr_return;
	const maxon::UniversalDateTime ts = maxon::UniversalDateTime::FromUnixTimestamp(licTimeStamp);
	const maxon::Int licDemoDays = pluginLicDict.Get<maxon::Int>("demodays"_s) iferr_return;
	maxon::TimeValue duration;
	duration.SetHours((Float64)(licDemoDays * 24));
	
	// retrieve current timestamp
	const maxon::UniversalDateTime now = maxon::UniversalDateTime::GetNow();
	
	// if demo days is set to anything greater than zero the plugin is in demo
	if (licDemoDays > 0)
	{
		_isDemo = true;
		
		// compare now with the demo period end time
		if (now > (ts + duration))
		{
			_isDemoExpired = true;
			WarningOutput("BLW >> SimpleGenerator license is invalid: demo expired");
			return maxon::OK;
		}
	}
	
	const maxon::Int runningC4DVersion = GetC4DVersion();
	const maxon::Int minC4DVer = pluginLicDict.Get<maxon::Int>("minc4dver"_s) iferr_return;
	const maxon::Int maxC4DVer = pluginLicDict.Get<maxon::Int>("maxc4dver"_s) iferr_return;
	if (runningC4DVersion < minC4DVer || runningC4DVersion > maxC4DVer)
	{
		WarningOutput("BLW >> SimpleGenerator license is invalid: Cinema 4D version range not matched");
		return maxon::OK;
	}
	
	
	
	// retrieve userID, sysID and allowed product configs from the read license
	const maxon::String licUserID = pluginLicDict.Get<maxon::String>("userID"_s) iferr_return;
	const maxon::String licSysID = pluginLicDict.Get<maxon::String>("systemID"_s) iferr_return;
	const maxon::DataDictionary allowedConfigs = pluginLicDict.Get<maxon::DataDictionary>("allowedProdConfigsDict"_s) iferr_return;
	
	// check  the current user ID is equal to user ID found on license file
	if (licUserID.IsEqual(userID) || licUserID.IsEmpty())
	{
		// check  the current system ID is equal to system ID found on license file
		if (licSysID.IsEqual(sysID) || licSysID.IsEmpty())
		{
			const maxon::Bool isKeyInAllowedConfigsDict = allowedConfigs.Contains(prodID);
			if (isKeyInAllowedConfigsDict)
			{
				const maxon::Bool isConfigAllowed = allowedConfigs.Get<maxon::Bool>(prodID) iferr_return;
				// check the current C4D configuration is among those allowed in the license file
				if (isConfigAllowed)
				{
					ApplicationOutput("BLW >> SimpleGenerator license is valid");
					_isLicensed = true;
				}
				else
				{
					WarningOutput("BLW >> SimpleGenerator license is invalid: product ID not matching");
						_isLicensed = false;
				}
			}
			else
			{
				WarningOutput("BLW >> SimpleGenerator license is invalid: product ID not found");
				_isLicensed = false;
			}
		}
		else
		{
			WarningOutput("BLW >> SimpleGenerator license is invalid: system ID not matching");
			_isLicensed = false;
		}
	}
	else
	{
		WarningOutput("BLW >> SimpleGenerator license is invalid: user ID not matching");
		_isLicensed = false;
	}
	
	return maxon::OK;
}

