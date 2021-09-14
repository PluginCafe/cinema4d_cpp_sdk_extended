/*
  Asset API Basics Example Plugin
  (C) 2021 MAXON Computer GmbH

  Author: Ferdinand Hoppe
  Date: 12/08/2021

  Provides examples for accessing Asset API databases and assets contained in them, as well as
  creating new assets and modifying their metadata.

  The examples are structured as functions which can be found in their own files ordered by
  subject. The part below only contain a CommandData implementation which is required to run these
  example functions, but not for understanding the Asset API. When being run, all examples will
  print their output to the Cinema 4D console.
*/
#include "maxon/apibase.h"

#include "c4d_general.h"
#include "c4d_commandplugin.h"


#include "c4d_gui.h"
#include "maxon/asset_databases.h"
#include "maxon/assets.h"
#include "maxon/stringresource.h"

#include "asset_api_basics_databases.h"
#include "asset_api_basics_assets.h"
#include "asset_api_basics_metadata.h"
#include "asset_api_basics_plugin.h"

// The plugin id for the AsssetApiBasics plugin. Please retrieve your own unique plugin id at
// https://plugincafe.maxon.net/c4dpluginid_cp when implementing your own projects, as you otherwise 
// will cause plugin id collisions when multiple plugins try to register with the same id.
#define PID_ASSET_API_BASICS 1058220

// The dialog gadget ids for the different example functions.
#define ID_GRP_TABS 100
#define ID_GRP_DATABASES 101
#define ID_GRP_ASSETS 102
#define ID_GRP_METADATA 103

#define ID_BTN_ACCESS_DATABASES 1000
#define ID_BTN_FIND_REPOSITORIES 1001
#define ID_BTN_CREATE_REPOSITORIES 1002
#define ID_BTN_ACCESS_IMPORTANT_REPOSITORIES 1003
#define ID_BTN_ADD_DATABASE 1004
#define ID_BTN_REMOVE_DATABASE 1005
#define ID_BTN_ACTIVATE_DATABASE 1006
#define ID_BTN_DEACTIVATE_DATABASE 1007
#define ID_BTN_FIND_ASSETS 1008

#define ID_BTN_CREATE_MATERIAL_ASSETS 2001
#define ID_BTN_CREATE_OBJECT_ASSETS 2002
#define ID_BTN_CREATE_SCENE_ASSETS 2003

// --- AssetApiBasicsDialog implementation ---------------------------------------------------------

/// ------------------------------------------------------------------------------------------------
/// Adds GUI elements to an AssetApiBasicsDialog when it is being opened.
/// ------------------------------------------------------------------------------------------------
Bool AssetApiBasicsDialog::CreateLayout()
{
  SetTitle("C++ SDK - Asset API Basics"_s);
  TabGroupBegin(ID_GRP_TABS, BFH_SCALEFIT);
  {
    GroupBorder(BORDER_NONE);
    GroupBorderSpace(5, 5, 5, 5);
    GroupSpace(5, 5);

    GroupBegin(ID_GRP_DATABASES, BFH_SCALEFIT, 1, 0, "Databases"_s, 0, 0, 0);
    {
      GroupBorder(BORDER_NONE);
      GroupSpace(5, 5);

      AddButton(ID_BTN_ACCESS_DATABASES, BFH_SCALEFIT, 0, 0,
        "Access User Databases"_s);
      // AddButton(ID_BTN_FIND_REPOSITORIES, BFH_SCALEFIT, 0, 0,
      //  "Find User Repositories"_s);
      AddButton(ID_BTN_CREATE_REPOSITORIES, BFH_SCALEFIT, 0, 0,
        "Get Repositories for all User Databases"_s);
      AddButton(ID_BTN_ACCESS_IMPORTANT_REPOSITORIES, BFH_SCALEFIT, 0, 0,
        "Access Important Repositories"_s);
      AddButton(ID_BTN_ADD_DATABASE, BFH_SCALEFIT, 0, 0,
        "Add User Database"_s);
      AddButton(ID_BTN_REMOVE_DATABASE, BFH_SCALEFIT, 0, 0,
        "Remove First User Database"_s);
      AddButton(ID_BTN_ACTIVATE_DATABASE, BFH_SCALEFIT, 0, 0,
        "Activate First Inactive User Database"_s);
      AddButton(ID_BTN_DEACTIVATE_DATABASE, BFH_SCALEFIT, 0, 0,
        "Deactivate First Active User Database"_s);
      AddButton(ID_BTN_FIND_ASSETS, BFH_SCALEFIT, 0, 0,
        "Find object assets in the user repository"_s);
      GroupEnd();
    }

    GroupBegin(ID_GRP_ASSETS, BFH_SCALEFIT, 1, 0, "Assets"_s, 0, 0, 0);
    {
      GroupBorder(BORDER_NONE);
      GroupSpace(5, 5);

      AddButton(ID_BTN_CREATE_MATERIAL_ASSETS, BFH_SCALEFIT, 0, 0,
        "Create Material Asset for the Active Material"_s);
      AddButton(ID_BTN_CREATE_OBJECT_ASSETS, BFH_SCALEFIT, 0, 0,
        "Create Object Asset for the Active Object"_s);
      AddButton(ID_BTN_CREATE_SCENE_ASSETS, BFH_SCALEFIT, 0, 0,
        "Create Scene Asset for the Active Scene"_s);
      GroupEnd();
    }

    GroupBegin(ID_GRP_METADATA, BFH_SCALEFIT, 1, 0, "Metadata"_s, 0, 0, 0);
    {
      GroupBorder(BORDER_NONE);
      GroupSpace(5, 5);

      GroupEnd();
    }
    GroupEnd();
  }
  return true;
}

/// ------------------------------------------------------------------------------------------------
/// Processes user inputs for the GUI elements of an AssetApiBasicsDialog dialog. 
/// ------------------------------------------------------------------------------------------------
Bool AssetApiBasicsDialog::Command(Int32 id, const BaseContainer& msg)
{
  iferr_scope_handler
  {
    return true;
  };

  // Databases
  if (id == ID_BTN_ACCESS_DATABASES)
  {
    AccessDatabases() iferr_return;
  }
  else if (id == ID_BTN_CREATE_REPOSITORIES)
  {
    CreateRepositories() iferr_return;
  }
  else if (id == ID_BTN_ACCESS_IMPORTANT_REPOSITORIES)
  {
    AccessImportantRepositories() iferr_return;
  }
  else if (id == ID_BTN_ADD_DATABASE)
  {
    AddDatabase() iferr_return;
  }
  else if (id == ID_BTN_REMOVE_DATABASE)
  {
    RemoveDatabase() iferr_return;
  }
  else if (id == ID_BTN_ACTIVATE_DATABASE)
  {
    ActivateDatabase() iferr_return;
  }
  else if (id == ID_BTN_DEACTIVATE_DATABASE)
  {
    DeactivateDatabase() iferr_return;
  }
  // Assets
  else if (id == ID_BTN_FIND_ASSETS)
  {
    FindAssets() iferr_return;
  }
  else if (id == ID_BTN_CREATE_MATERIAL_ASSETS)
  {
    CreateMaterialAsset() iferr_return;
  }
  else if (id == ID_BTN_CREATE_OBJECT_ASSETS)
  {
    CreateObjectAsset() iferr_return;
  }
  else if (id == ID_BTN_CREATE_SCENE_ASSETS)
  {
    CreateSceneAsset() iferr_return;
  }
  // Metadata
  return true;
}

// --- AssetApiBasicsCommand implementation --------------------------------------------------------

/// ------------------------------------------------------------------------------------------------
/// Opens the AssetApiBasicsDialog attached to the AssetAccessData when the command is being 
/// invoked.
/// ------------------------------------------------------------------------------------------------ 
Bool AssetApiBasicsCommand::Execute(BaseDocument* doc, GeDialog* parentManager)
{
  if (_dialog.IsOpen() == false)
    _dialog.Open(DLG_TYPE::ASYNC, PID_ASSET_API_BASICS, -1, -1, 350, 50);
  else
    _dialog.Close();

  return true;
};

/// ------------------------------------------------------------------------------------------------
/// Called to restore the dialog after a layout change.
/// ------------------------------------------------------------------------------------------------
Bool AssetApiBasicsCommand::RestoreLayout(void* secret)
{
  return _dialog.RestoreLayout(PID_ASSET_API_BASICS, 0, secret);
};

/// ------------------------------------------------------------------------------------------------
/// Called to register the AssetApiBasics example plugin.
/// @return Success of registration.
/// ------------------------------------------------------------------------------------------------
Bool RegisterAssetApiBasics()
{
  return RegisterCommandPlugin(
    PID_ASSET_API_BASICS, "C++ SDK - Asset API Basics"_s, 0, nullptr,
    "Demonstrates basic operations with the Asset API"_s, NewObjClear(AssetApiBasicsCommand));
}