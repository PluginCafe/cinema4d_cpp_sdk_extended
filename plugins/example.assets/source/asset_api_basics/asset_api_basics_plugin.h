/*
  Asset API Basics Example Plugin
  Copyright (C) 2021 MAXON Computer GmbH

  Author: Ferdinand Hoppe
  Date: 12/08/2021

  Provides examples for accessing Asset API databases and assets contained in them, as well as
  creating new assets and modifying their metadata.

  The examples are structured as functions which can be found in their own files ordered by
  subject. The part below only contain a CommandData implementation which is required to run these
  example functions, but not for understanding the Asset API. When being run, all examples will
  print their output to the Cinema 4D console.
*/
#ifndef ASSET_API_BASICS_PLUGIN_H__
#define ASSET_API_BASICS_PLUGIN_H__

/// ------------------------------------------------------------------------------------------------
/// The dialog with which the AsssetApiBasics example plugin is being controlled.
/// ------------------------------------------------------------------------------------------------
class AssetApiBasicsDialog : public GeDialog
{
public:
  /// Adds GUI elements to an AssetApiBasicsDialog when it is being opened.
  virtual Bool CreateLayout();
  /// Processes user inputs for the GUI elements of an AssetApiBasicsDialog dialog. 
  virtual Bool Command(Int32 id, const BaseContainer& msg);
};

/// ------------------------------------------------------------------------------------------------
/// The CommandData plugin interface for the AsssetApiBasics example.
/// ------------------------------------------------------------------------------------------------
class AssetApiBasicsCommand : public CommandData
{
  INSTANCEOF(AssetApiBasicsCommand, CommandData)

private:
  AssetApiBasicsDialog _dialog;

public:
  static AssetApiBasicsCommand* Alloc() { return NewObjClear(AssetApiBasicsCommand); }
  /// Opens the AssetApiBasicsDialog attached to the AssetAccessData when the command is being 
  /// invoked.
  virtual Bool Execute(BaseDocument* doc, GeDialog* parentManager);
  /// Called to restore the dialog after a layout change.
  virtual Bool RestoreLayout(void* secret);
};

/// ------------------------------------------------------------------------------------------------
/// Called to register the AssetApiBasics example plugin.
/// @return Success of registration.
/// ------------------------------------------------------------------------------------------------
Bool RegisterAssetApiBasics();

#endif // ASSET_API_BASICS_PLUGIN_H__
