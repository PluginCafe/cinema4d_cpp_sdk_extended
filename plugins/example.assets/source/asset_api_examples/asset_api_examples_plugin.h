/*
  Asset API Examples - Plugin
  Copyright (C) 2022 MAXON Computer GmbH

  Author: Ferdinand Hoppe
  Date: 01/04/2022
  SDK: R26

  Contains the plugin layer to invoke the provided examples.

  The code shown here is irrelevant for learning the Asset API.
*/
#ifndef ASSET_API_EXAMPLES_PLUGIN_H__
#define ASSET_API_EXAMPLES_PLUGIN_H__

#include "c4d_commandplugin.h"
#include "c4d_general.h"
#include "c4d_gui.h"
#include "c4d_listview.h"

#include "maxon/apibase.h"

// The URL of the SDK asset database.
#define SDK_DATABASE_URL "https://assets.maxon.net/assets/sdkdatabase.db"

/// Provides the GUI for the Asset API examples and also most of its interface logic.
class AssetApiExamplesDialog : public GeDialog
{
public:
  AssetApiExamplesDialog() {}
  ~AssetApiExamplesDialog() {}

  /// Populates the GUI of the dialog.
  virtual Bool CreateLayout();

  /// Processes user inputs in the dialog. 
  virtual Bool Command(Int32 id, const BaseContainer& msg);

  /// Initializes the dialog once its layout has been created.
  virtual Bool InitValues();

  /// Called when the dialog is closed.
	virtual Bool AskClose();

  /// Runs one of the Asset API example cases.
  virtual maxon::Result<void> RunExample(const Int32 eid);

private:
  // The list view used by the four example categories.
  SimpleListView contentListView = SimpleListView();

  // The example id range for the currently loaded examples.
  maxon::Int32 minExampleId = NOTOK;
  maxon::Int32 maxExampleId = NOTOK;

  /// Indicates that an error message must be shown that loading the asset database has failed.
	maxon::Bool showDatabaseMountingError = true;

  /// The composed default help string for the context help.
  maxon::String defeaultContextHelp;

  /// The toggle state of the clear console bitmap button.
  maxon::Bool clearConsole = true;

  /// The function reference storage used by the repository observer related examples.
  maxon::HashMap<maxon::String, maxon::FunctionBaseRef> observerData;

  /// Gets the index of the selected item in the content list view.
  maxon::Result<maxon::Int32> GetSelectedItem();

  /// Loads the Asset API example scene used by some of the examples.
  maxon::Result<void> LoadExampleScene();

  /// Attempts to access the SDK - Cube asset to test if the SDK database is present.
	maxon::Result<maxon::Bool> AssertSDKDatabaseAccess();

  /// Ensures the SDK asset database URL is mounted and mounts it if not.
  maxon::Result<void> MountSDKAssetDatabase();

  /// Sets the content of the list view when the category did change.

  maxon::Result<void> SetExamplesListview(maxon::Int32 selectedCategory);
};


/// Provides the plugin interface for the Asset API examples.
class AssetApiExamplesCommand : public CommandData
{
  INSTANCEOF(AssetApiExamplesCommand, CommandData)

private:
  /// The dialog of the plugin instance.
  AssetApiExamplesDialog dialog;

public:
  static AssetApiExamplesCommand* Alloc() { return NewObjClear(AssetApiExamplesCommand); }

  /// Opens the dialog of the plugin.
  virtual Bool Execute(BaseDocument* doc, GeDialog* parentManager);

  /// Called to restore the dialog of the plugin after a layout change.
  virtual Bool RestoreLayout(void* secret);
};


/// Called to register the AssetApiBasics example plugin.
Bool RegisterAssetApiBasics();

#endif // ASSET_API_EXAMPLES_PLUGIN_H__
