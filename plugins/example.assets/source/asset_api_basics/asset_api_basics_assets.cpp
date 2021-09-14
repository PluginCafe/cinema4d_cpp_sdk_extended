/*
  Asset API Basics Example Plugin
  (C) 2021 MAXON Computer GmbH

  Author: Ferdinand Hoppe
  Date: 12/08/2021

  Provides the asset related example functions.
*/
#include "maxon/apibase.h"

#include "c4d_general.h"
#include "c4d_basedocument.h"
#include "c4d_basematerial.h"

#include "asset_api_basics_assets.h"
#include "maxon/datadescription_string.h"
#include "maxon/asset_creation.h"
#include "maxon/asset_databases.h"
#include "maxon/asset_metaproperties.h"
#include "maxon/subtype_asset.h"
#include "maxon/assets.h"
#include "maxon/stringresource.h"


/// Store materials as assets in an asset repository to be exposed by the Asset Browser.
/// 
/// The example uses one of the convenience functions attached to AssetCreationInterface to store
/// the active material, i.e., the first selected material in the Material Manager, as an asset. 
/// The repository used in this example is user preferences repository. See Asset API - Databases
/// for information on how to build and retrieve custom repositories.

maxon::Result<void> CreateMaterialAsset()
{
  // An error scope handling which is more verbose than a simple "iferr_scope;". If one of the
  // calls below returns an error, the function will be exited through this block.
  iferr_scope_handler
  {
    ApplicationOutput("Stopped CreateMaterialAsset() execution with the error: @", err);
    return err;
  };

  // Get the active document and the active material in it.
  BaseDocument* doc = GetActiveDocument();
  if (doc == nullptr)
    return maxon::NullptrError(MAXON_SOURCE_LOCATION);

  BaseMaterial* material = doc->GetActiveMaterial();
  if (material == nullptr)
    return maxon::NullptrError(MAXON_SOURCE_LOCATION);

  // Get the user preferences repository to store our asset in, we could also use our own database
  // and repository here. For details see Asset API - Databases.
  const maxon::AssetRepositoryRef& repository = maxon::AssetInterface::GetUserPrefsRepository();

  // Create an id for the new asset.
  maxon::Id assetId = maxon::AssetInterface::MakeUuid("material", false) iferr_return;
  // The name of the asset, we can pick here anything we would like, the name does not have to be
  // unique.
  maxon::String assetName = FormatString("C++ SDK - Material Asset (@)", material->GetName());
  // Create the metadata for the asset, we will leave it empty here. See Asset API - Metadata for
  // details on handling asset metadata.
  maxon::AssetMetaData metadata;
  // The version of the asset.
  maxon::String assetVersion = "1.0"_s;
  // And finally the StoreAssetStruct which bundles up a category id, the lookup and the storage
  // repository.
  maxon::StoreAssetStruct storeAsset{ maxon::Id(), repository, repository };

  // There are more atomic methods attached to AssetRepositoryInterface we could use to store 
  // assets, but here we are going to use one of the convenience functions attached to 
  // AssetCreationInterface to store the material as an asset.
  maxon::AssetDescription description = maxon::AssetCreationInterface::CreateMaterialAsset(
    doc, material, storeAsset, assetId, assetName, assetVersion, metadata, true) iferr_return;

  // Push an update event to Cinema 4D, so that the Asset Browser can catch up.
  EventAdd();
  ApplicationOutput("Created material asset with the id '@'.", description);

  return maxon::OK;
}

/// Store objects as assets in an asset repository to be exposed by the Asset Browser.
/// 
/// The example uses one of the convenience functions attached to AssetCreationInterface to store
/// the active object, i.e., the first selected object in the Object Manager, as an asset. The
/// object is being stored with all 'connected' scene elements like child objects, tags and 
/// materials that are being used by the object or one of its children. The repository used in 
/// this example is user preferences repository. See Asset API - Databases
/// for information on how to build and retrieve custom repositories.
maxon::Result<void> CreateObjectAsset()
{
  // An error scope handling which is more verbose than a simple "iferr_scope;". If one of the
  // calls below returns an error, the function will be exited through this block.
  iferr_scope_handler
  {
    ApplicationOutput("Stopped CreateObjectAsset() execution with the error: @", err);
    return err;
  };

  // Get the active document and the active object in it.
  BaseDocument* doc = GetActiveDocument();
  if (doc == nullptr)
    return maxon::NullptrError(MAXON_SOURCE_LOCATION);

  BaseObject* op = doc->GetActiveObject();
  if (op == nullptr)
    return maxon::NullptrError(MAXON_SOURCE_LOCATION);

  // Get the user preferences repository to store our asset in, we could also use our own database
  // and repository here. For details see Asset API - Databases.
  const maxon::AssetRepositoryRef& repository = maxon::AssetInterface::GetUserPrefsRepository();

  // Create an id for the new asset.
  maxon::Id assetId = maxon::AssetInterface::MakeUuid("object", false) iferr_return;
  // The name of the asset, we can pick here anything we would like, the name does not have to be
  // unique.
  maxon::String assetName = FormatString("C++ SDK - Object Asset (@)", op->GetName());
  // Create the metadata for the asset, we will leave it empty here. See Asset API - Metadata for
  // details on handling asset metadata.
  maxon::AssetMetaData metadata;
  // The version of the asset.
  maxon::String assetVersion = "1.0"_s;
  // And finally the StoreAssetStruct which bundles up a category id, the lookup and the storage
  // repository.
  maxon::StoreAssetStruct storeAsset{ maxon::Id(), repository, repository };

  // There are more atomic methods attached to AssetRepositoryInterface we could use to store 
  // assets, but here we are going to use one of the convenience functions attached to 
  // AssetCreationInterface to store the material as an asset.
  maxon::AssetDescription description = maxon::AssetCreationInterface::CreateObjectAsset(
    op, doc, storeAsset, assetId, assetName, assetVersion, metadata, true) iferr_return;

  // Push an update event to Cinema 4D, so that the Asset Browser can catch up.
  EventAdd();
  ApplicationOutput("Created object asset with the id '@'.", description);

  return maxon::OK;
}

/// Store whole scenes as assets in an asset repository to be exposed by the Asset Browser.
/// 
/// The example uses one of the convenience functions attached to AssetCreationInterface to store
/// the active scene, i.e., the scene that is currently shown by Cinema 4D, as an asset. The 
/// repository used in this example is user preferences repository. See Asset API - Databases
/// for information on how to build and retrieve custom repositories.
maxon::Result<void> CreateSceneAsset()
{
  // An error scope handling which is more verbose than a simple "iferr_scope;". If one of the
  // calls below returns an error, the function will be exited through this block.
  iferr_scope_handler
  {
    ApplicationOutput("Stopped CreateSceneAsset() execution with the error: @", err);
    return err;
  };

  // Get the active document.
  BaseDocument* doc = GetActiveDocument();
  if (doc == nullptr)
    return maxon::NullptrError(MAXON_SOURCE_LOCATION);

  // Get the user preferences repository to store our asset in, we could also use our own database
  // and repository here. For details see Asset API - Databases.
  const maxon::AssetRepositoryRef& repository = maxon::AssetInterface::GetUserPrefsRepository();

  // Create an id for the new asset.
  maxon::Id assetId = maxon::AssetInterface::MakeUuid("scene", false) iferr_return;
  // The name of the asset, we can pick here anything we would like, the name does not have to be
  // unique.
  maxon::String assetName = FormatString("C++ SDK - Scene Asset (@)", doc->GetDocumentName());
  // Create the metadata for the asset, we will leave it empty here. See Asset API - Metadata for
  // details on handling asset metadata.
  maxon::AssetMetaData metadata;
  // The version of the asset.
  maxon::String assetVersion = "1.0"_s;
  // And finally the StoreAssetStruct which bundles up a category id, the lookup and the storage
  // repository.
  maxon::StoreAssetStruct storeAsset{ maxon::Id(), repository, repository };

  // There are more atomic methods attached to AssetRepositoryInterface we could use to store 
  // assets, but here we are going to use one of the convenience functions attached to 
  // AssetCreationInterface to store the material as an asset.
  maxon::AssetDescription description = maxon::AssetCreationInterface::CreateSceneAsset(
    doc, storeAsset, assetId, assetName, assetVersion, metadata, true) iferr_return;

  // Push an update event to Cinema 4D, so that the Asset Browser can catch up.
  EventAdd();
  ApplicationOutput("Created scene asset with the id '@'.", description);

  return maxon::OK;
}
