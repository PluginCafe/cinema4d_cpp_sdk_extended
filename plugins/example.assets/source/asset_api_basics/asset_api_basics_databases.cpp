/*
  Asset API Basics Example Plugin
  (C) 2021 MAXON Computer GmbH

  Author: Ferdinand Hoppe
  Date: 12/08/2021

  Provides the database and repository related example functions.
*/
#include "maxon/apibase.h"

#include "c4d_general.h"
#include "c4d_basedocument.h"

#include "asset_api_basics_databases.h"
#include "maxon/datadescription_string.h"
#include "maxon/asset_databases.h"
#include "maxon/asset_metaproperties.h"
#include "maxon/subtype_asset.h"
#include "maxon/assets.h"
#include "maxon/stringresource.h"

/// ------------------------------------------------------------------------------------------------
/// Access the data structures that represent the user asset databases attached to the running
/// Cinema 4D instance.
///
/// The accessed AssetDatabaseStruct instances only contain meta information about the asset 
/// databases, not the content of these databases. Accessed will be the user databases, not the
/// application databases with which Cinema does deliver its default assets. See 
/// AccessImportantRepositories() for how to access the repositories of the application databases.
/// ------------------------------------------------------------------------------------------------
maxon::Result<void> AccessDatabases()
{
  // An error scope handling which is more verbose than a simple "iferr_scope;". If one of the
  // calls below returns an error, the function will be exited through this block.
  iferr_scope_handler
  {
    ApplicationOutput("Stopped AccessDatabases() execution with the error: @", err);
    return err;
  };

  // Databases have to be loaded and we should wait for that as we otherwise will not find
  // any databases or not all databases.
  maxon::Bool loaded = maxon::AssetDataBasesInterface::WaitForDatabaseLoading();
  ApplicationOutput("Databases have been loaded: @", loaded);
  if (!loaded)
  {
    ApplicationOutput("Could not load databases.");
    return maxon::OK;
  }

  // Get the user databases which are represented by instances of AssetDatabaseStruct.
  maxon::BaseArray<maxon::AssetDatabaseStruct> databaseCollection;
  maxon::AssetDataBasesInterface::GetDatabases(databaseCollection) iferr_return;

  // Report how many databases were found.
  ApplicationOutput("Found @ asset database(s)", databaseCollection.GetCount());

  // And iterate over them to print out the their fields. 
  for (maxon::AssetDatabaseStruct& database : databaseCollection)
  {
    ApplicationOutput("Found asset database with the properties - url: @, active: @, builtin: @", 
      database._dbUrl, database._active, database._isBuiltin);
  }

  return maxon::OK;
}


/// ------------------------------------------------------------------------------------------------
/// Create repositories and their underlying user databases from scratch or wrap with repositories 
/// around existing user databases.
///
/// The example will create repositories for the mounted and active user asset databases and print 
/// out basic information for each of them. This example will not access the application 
/// repositories Cinema 4D does come with, see AccessImportantRepositories() for how to access the 
/// application repositories.
/// 
/// ------------------------------------------------------------------------------------------------
maxon::Result<void> CreateRepositories()
{
  // An error scope handling which is more verbose than a simple "iferr_scope;". If one of the
  // calls below returns an error, the function will be exited through this block.
  iferr_scope_handler
  {
    ApplicationOutput("Stopped CreateRepositories() execution with the error: @", err);
    return err;
  };

  // Databases have to be loaded and we should wait for that as we otherwise will not find
  // any databases or not all databases.
  maxon::Bool loaded = maxon::AssetDataBasesInterface::WaitForDatabaseLoading();
  ApplicationOutput("Databases have been loaded: @", loaded);
  if (!loaded)
  {
    ApplicationOutput("Could not load databases.");
    return maxon::OK;
  }

  // Get the user databases which are represented by instances of AssetDatabaseStruct.
  maxon::BaseArray<maxon::AssetDatabaseStruct> databaseCollection;
  maxon::AssetDataBasesInterface::GetDatabases(databaseCollection) iferr_return;

  // Iterate over the databases in order to get an AssetRepositoryRef for them with their Url.
  for (maxon::AssetDatabaseStruct database : databaseCollection)
  {
    ApplicationOutput("Found asset database with the properties - url: @, active: @, builtin: @", 
      database._dbUrl, database._active, database._isBuiltin);

    // Get the repository for the Url.

    // Create an unique identifier for the database, we have to make sure this identifier is not
    // being used by any bases of the repository. In this case we could get just any id, since our
    // repository will not have any bases. But we are going to construct an id with the database
    // Url as a prefix for demonstration purposes. To make this even nicer 
    const maxon::BaseArray<Char> path = database._dbUrl.GetPath().GetCString() iferr_return;
    const maxon::Char* prefix = path.GetFirst();
    maxon::Id uuid = maxon::AssetInterface::MakeUuid(prefix, false) iferr_return;

    // Create or get the bases for the repository. In this case we do not want any bases, i.e., we
    // want the repository to only contain what is contained in this one database.
    const maxon::Block<const maxon::AssetRepositoryRef> bases{};

    // Create the repository reference for that database. If the third argument, database._dbUrl,
    // would point to a directory where no database is being located, Cinema would create the
    // necessary database structure.
    maxon::UpdatableAssetRepositoryRef repository = maxon::AssetInterface::CreateRepositoryFromUrl(
      uuid, bases, database._dbUrl, true, true, true) iferr_return;

    // Print some information for the new created repository reference.
    maxon::Id id = repository.GetId();
    maxon::String name = repository.GetRepositoryName(
      maxon::Resource::GetDefaultLanguage()) iferr_return;
    maxon::Bool isWriteable = repository.IsWritable();

    ApplicationOutput("Build repository with the properties - name: @, id: @, isWriteable: @", 
      name, id, isWriteable);
  }
  return maxon::OK;
}

/// ------------------------------------------------------------------------------------------------
/// Accesses the built-in, app, user and active document repositories.
///
/// The here described methods are the most common forms of accessing asset databases.
/// ------------------------------------------------------------------------------------------------
maxon::Result<void> AccessImportantRepositories()
{
  // An error scope handling which is more verbose than a simple "iferr_scope;". If one of the
  // calls below returns an error, the function will be exited through this block.
  iferr_scope_handler
  {
    ApplicationOutput("Stopped AccessImportantRepositories() execution with the error: @", err);
    return err;
  };

  // We will append the these four common repositories to this BaseArray.
  maxon::BaseArray<maxon::AssetRepositoryRef> importantRepositories;

  // The non-writable built-in repository of Cinema 4D, this will contain almost nothing.
  importantRepositories.Append(maxon::AssetInterface::GetBuiltinRepository()) iferr_return;
  // The non-writable application repository based on the built-in repository of Cinema 4D, it
  // will contain assets like nodes.
  importantRepositories.Append(maxon::AssetInterface::GetApplicationRepository()) iferr_return;
  // The writable user preferences repository, it will contain the shipped asset database and
  // the user databases attached to the running instance.
  importantRepositories.Append(maxon::AssetInterface::GetUserPrefsRepository()) iferr_return;
  // The repository that is associated with the active document, passing true to 
  // GetSceneRepository() means that it will be created if there is none.
  BaseDocument* doc = GetActiveDocument();
  maxon::AssetRepositoryRef documentRepository = doc->GetSceneRepository(true) iferr_return;
  importantRepositories.Append(documentRepository) iferr_return;

  // Print some information for all three repositories.
  for (maxon::AssetRepositoryRef repository : importantRepositories)
  {
    maxon::Id id = repository.GetId();
    maxon::String name = repository.GetRepositoryName(
      maxon::Resource::GetDefaultLanguage()) iferr_return;
    maxon::Bool isWriteable = repository.IsWritable();

    ApplicationOutput("Repository with the properties - name: @, id: @, isWriteable: @", 
      name, id, isWriteable);
  }

  return maxon::OK;
}

/// ------------------------------------------------------------------------------------------------
/// Add user databases to the running Cinema 4D instance.
/// 
/// This example will open a directory selection dialog to let the user select a database location. 
/// If the selected directory already does contain an asset database, Cinema will wrap around that 
/// data. If it does not, Cinema will create the necessary data representing a new database within 
/// this directory. This example aligns closely with what is being done in the Asset Browser section 
/// of the Preferences dialog of Cinema 4D. 
/// ------------------------------------------------------------------------------------------------
maxon::Result<void> AddDatabase()
{
  // An error scope handling which is more verbose than a simple "iferr_scope;". If one of the
  // calls below returns an error, the function will be exited through this block.
  iferr_scope_handler
  {
    ApplicationOutput("Stopped AddDatabase() execution with the error: @", err);
    return err;
  };

  // Open a directory selection dialog to pick the new database location.
  Filename filename;
  Bool res = filename.FileSelect(FILESELECTTYPE::ANYTHING, FILESELECT::DIRECTORY, 
    "Select database directory"_s);
  if (!res)
    return maxon::OK;

  // Create a new unique id for the database.
  maxon::Id  uuid = maxon::AssetInterface::MakeUuid("database", false) iferr_return;
  // Convert the classic API file location into a maxon::Url.
  maxon::Url databaseUrl = MaxonConvert(filename, MAXONCONVERTMODE::WRITE);

  // Databases have to be loaded and we should wait for that as we otherwise will not find
  // any databases or not all databases.
  maxon::Bool loaded = maxon::AssetDataBasesInterface::WaitForDatabaseLoading();
  ApplicationOutput("Databases have been loaded: @", loaded);
  if (!loaded)
  {
    ApplicationOutput("Could not load databases.");
    return maxon::OK;
  }

  // Get the user asset databases.
  maxon::BaseArray<maxon::AssetDatabaseStruct> databaseCollcetion;
  maxon::AssetDataBasesInterface::GetDatabases(databaseCollcetion) iferr_return;

  // Iterate over the existing databases to check if a database with that Url has already been 
  // added.
  Bool shouldAddDatabase = true;
  for (maxon::AssetDatabaseStruct& database : databaseCollcetion)
  {
    // We found a database with the same Url as the one specified by the folder selection dialog.
    if (database._dbUrl == databaseUrl)
    {
      // The existing database is also already active.
      if (database._active)
      {
        ApplicationOutput("The database with the url '@' is already active.", databaseUrl);
        return maxon::OK;
      }
      // Activate an already added but inactive database.
      database._active = true;
      shouldAddDatabase = false;
    }
  }

  // The database should be added and we append a AssetDatabaseStruct.
  if (shouldAddDatabase)
  {
    // Pack the database information into a AssetDatabaseStruct, passing the field values  _dbUrl, 
    // _active and _exportOnSaveProject. Could also be shortened to:
    //
    //    databaseCollcetion.Append({ databaseUrl, true, true }) iferr_return;

    maxon::AssetDatabaseStruct newDatabase { databaseUrl, true, true };
    databaseCollcetion.Append(newDatabase) iferr_return;
    ApplicationOutput("Added new database with the url: @", databaseUrl);
  }

  // Write the AssetDatabaseStruct BaseArray back into Cinema 4D. It is important that the new 
  // database is being added to the BaseArray of AssetDatabaseStruct retrieved from GetDatabases().
  // If just a BaseArray of the to be added databases would be passed, all other user databases 
  // would be unmounted.
  maxon::AssetDataBasesInterface::SetDatabases(databaseCollcetion) iferr_return;

  // An event has to be invoked, so that the GUI of Cinema can catch up. 
  EventAdd();

  return maxon::OK;
}

/// ------------------------------------------------------------------------------------------------
/// Remove user databases from the running Cinema 4D instance.
/// 
/// This example will remove the first user database from the running Cinema 4D instance. This will
/// cause all assets contained in the database to become inaccessible. This example aligns closely 
/// with what is being done in the Asset Browser section of the Preferences dialog of Cinema 4D. 
/// ------------------------------------------------------------------------------------------------
maxon::Result<void> RemoveDatabase()
{
  // An error scope handling which is more verbose than a simple "iferr_scope;". If one of the
  // calls below returns an error, the function will be exited through this block.
  iferr_scope_handler
  {
    ApplicationOutput("Stopped RemoveDatabase() execution with the error: @", err);
    return err;
  };

  // Databases have to be loaded and we should wait for that as we otherwise will not find
  // any databases or not all databases.
  maxon::Bool loaded = maxon::AssetDataBasesInterface::WaitForDatabaseLoading();
  ApplicationOutput("Databases have been loaded: @", loaded);
  if (!loaded)
  {
    ApplicationOutput("Could not load databases.");
    return maxon::OK;
  }

  // Get the user databases which are represented by instances of AssetDatabaseStruct.
  maxon::BaseArray<maxon::AssetDatabaseStruct> databaseCollection;
  maxon::AssetDataBasesInterface::GetDatabases(databaseCollection) iferr_return;

  // There are no databases which could be removed.
  if (databaseCollection.GetCount() == 0)
  {
    ApplicationOutput("Did not find any user databases which could be removed.");
  }
  // Remove the first database.
  else
  {
    // Report which database we are going to remove.
    maxon::AssetDatabaseStruct removeMe = databaseCollection[0];
    ApplicationOutput("Removing database with the url: @", removeMe._dbUrl);
    // Remove the first AssetDatabaseStruct representing a database. This will not erase the 
    // physical database, but only remove the item representing it.
    databaseCollection.Erase(0) iferr_return;
    // Write the modified database collection back.
    maxon::AssetDataBasesInterface::SetDatabases(databaseCollection) iferr_return;
    // An event has to be invoked, so that the GUI of Cinema can catch up. 
    EventAdd();
  }

  return maxon::OK;
}

/// Activate user databases to make their assets accessible both within the Asset Browser and 
/// the Asset API.
///
/// The example will activate the first user database that is mounted an inactive. Activated 
/// databases will appear as checked in the Asset Browser section of the Cinema 4D Preferences 
/// dialog.
maxon::Result<void> ActivateDatabase()
{
  // An error scope handling which is more verbose than a simple "iferr_scope;". If one of the
  // calls below returns an error, the function will be exited through this block.
  iferr_scope_handler
  {
    ApplicationOutput("Stopped ActivateDatabase() execution with the error: @", err);
    return err;
  };

  // Databases have to be loaded and we should wait for that as we otherwise will not find
  // any databases or not all databases.
  maxon::Bool loaded = maxon::AssetDataBasesInterface::WaitForDatabaseLoading();
  ApplicationOutput("Databases have been loaded: @", loaded);
  if (!loaded)
  {
    ApplicationOutput("Could not load databases.");
    return maxon::OK;
  }

  // Get the user databases which are represented by instances of AssetDatabaseStruct.
  maxon::BaseArray<maxon::AssetDatabaseStruct> databaseCollection;
  maxon::AssetDataBasesInterface::GetDatabases(databaseCollection) iferr_return;

  // And iterate over them to find the first database that is inactive.
  for (maxon::AssetDatabaseStruct& database : databaseCollection)
  {
    // The current database is inactive.
    if (!database._active)
    {
      // Activate the database.
      database._active = true;

      // Write the modified database collection back.
      maxon::AssetDataBasesInterface::SetDatabases(databaseCollection) iferr_return;
      // An event has to be invoked, so that the GUI of Cinema can catch up. 
      EventAdd();

      ApplicationOutput("Activated databse with the url: @", database._dbUrl);
      return maxon::OK;
    }
  }
  ApplicationOutput("No inactive database found.");
  return maxon::OK;
}

/// Deactivate user databases to make their assets inaccessible both within the Asset Browser and 
/// the Asset API.
///
/// The example will deactivate the first user database that is mounted an active. Deactivated 
/// databases will appear as unchecked in the Asset Browser section of the Cinema 4D Preferences 
/// dialog.
maxon::Result<void> DeactivateDatabase()
{
  // An error scope handling which is more verbose than a simple "iferr_scope;". If one of the
  // calls below returns an error, the function will be exited through this block.
  iferr_scope_handler
  {
    ApplicationOutput("Stopped DeactivateDatabase() execution with the error: @", err);
    return err;
  };

  // Databases have to be loaded and we should wait for that as we otherwise will not find
  // any databases or not all databases.
  maxon::Bool loaded = maxon::AssetDataBasesInterface::WaitForDatabaseLoading();
  ApplicationOutput("Databases have been loaded: @", loaded);
  if (!loaded)
  {
    ApplicationOutput("Could not load databases.");
    return maxon::OK;
  }

  // Get the user databases which are represented by instances of AssetDatabaseStruct.
  maxon::BaseArray<maxon::AssetDatabaseStruct> databaseCollection;
  maxon::AssetDataBasesInterface::GetDatabases(databaseCollection) iferr_return;

  // Report how many databases were found.
  ApplicationOutput("Found @ asset database(s).", databaseCollection.GetCount());

  // And iterate over them to find the first database that is active.
  for (maxon::AssetDatabaseStruct& database : databaseCollection)
  {
    // The current database is active.
    if (database._active)
    {
      // Deactivate the database.
      database._active = false;

      // Write the modified database collection back.
      maxon::AssetDataBasesInterface::SetDatabases(databaseCollection) iferr_return;
      // An event has to be invoked, so that the GUI of Cinema can catch up. 
      EventAdd();

      ApplicationOutput("Deactivated databse with the url: @", database._dbUrl);
      return maxon::OK;
    }
  }
  ApplicationOutput("No active database found.");
  return maxon::OK;
}

static Int32 g_max_object_to_find = 10; 
/// ------------------------------------------------------------------------------------------------
/// Search for assets in repositories and filter the results by their type, subtype or other 
/// properties.
///
/// The example iterates over the first ten object assets in the user preferences repository and 
/// prints out their name, id and the id of the repository they are contained in. 
/// ------------------------------------------------------------------------------------------------
maxon::Result<void> FindAssets()
{
  // An error scope handling which is more verbose than a simple "iferr_scope;". If one of the
  // calls below returns an error, the function will be exited through this block.
  iferr_scope_handler
  {
    ApplicationOutput("Stopped FindAssets() execution with the error: @", err);
    return err;
  };

  // Get the user preferences repository. For retrieving specific user database repositories, see 
  // CreateRepositories() in asset_api_basics_databases.cpp.
  const maxon::AssetRepositoryRef& repository = maxon::AssetInterface::GetUserPrefsRepository();

  // Now we are preparing some arguments for the call to find assets.

  // The asset type we are looking for. Asset types are declared in the maxxon::AssetTypes 
  // namespace.
  const maxon::AssetType assetType = maxon::AssetTypes::File();
  // The id of the asset we are looking for, passing an empty id will return assets with any id.
  const maxon::Id assetFindId = {};
  // The version of the asset we are looking for, passing an empty id will return assets with any 
  // version.
  const maxon::Id assetVersion = {};
  // A maxon::BaseArray where we will store all found asset descriptions. 
  maxon::BaseArray<maxon::AssetDescription> results;

  // There are multiple methods attached to AssetRepositoryInterface to find assets in an 
  // repository. We use here FindAsset() which allows us to define the verion types we want
  // to retrieve. In this case we pass ASSET_FIND_MODE::LATEST to only find the latest version of
  // each asset. But we could for example also pass ASSET_FIND_MODE::ALL to find all version of an
  // asset.
  repository.FindAssets(assetType, assetFindId, assetVersion, maxon::ASSET_FIND_MODE::LATEST, 
    results) iferr_return;
  ApplicationOutput("Total number of file-type assets found: @", results.GetCount());

  maxon::Int counter = 0;
  // Iterate over the results and print out some data for the first ten object assets.
  for (maxon::AssetDescription assetDescription : results)
  {
    // Exit when we encountered ten object assets.
    if (counter == g_max_object_to_find)
      break;

    // We retrieve the subtype of the subtype asset. See the topic Asset API - Metadata for more 
    // details on asset metadata.
    maxon::AssetMetaData metadata = assetDescription.GetMetaData();
    maxon::Id subTypeId = metadata.Get(maxon::ASSETMETADATA::SubType) iferr_return;

    // This asset is of subtype object. 
    if (subTypeId == maxon::ASSETMETADATA::SubType_ENUM_Object)
    {
      // Now we collect some data to print out.

      // The id of the asset itself.
      maxon::Id assetId = assetDescription.GetId();

      // Rather pointless here, because we know the repository in this case already, but an 
      // AssetDescription has a the id of the repository attached it is contained in.
      maxon::Id repositoryId = assetDescription.GetRepositoryId();

      // The storage Url of the asset.
      const maxon::Url& assetUrl = assetDescription.GetUrl() iferr_return;

      // The name of the asset.
      maxon::String assetName = assetDescription.GetMetaString(maxon::OBJECT::BASE::NAME,
        maxon::LanguageRef()) iferr_return;

      ApplicationOutput("Found object asset with the id: @", assetId);
      ApplicationOutput("\tRepository: @", repositoryId);
      ApplicationOutput("\tLocation: @", assetUrl);
      ApplicationOutput("\tName: @", assetName);

      counter++;
    }
  }

  return maxon::OK;
}
