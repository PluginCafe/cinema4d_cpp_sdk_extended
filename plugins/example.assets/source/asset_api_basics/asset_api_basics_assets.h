/*
  Asset API Basics Example Plugin
  Copyright (C) 2021 MAXON Computer GmbH

  Author: Ferdinand Hoppe
  Date: 12/08/2021

  Provides the asset related example functions.
*/
#ifndef ASSET_API_BASICS_ASSETS_H__
#define ASSET_API_BASICS_ASSETS_H__

maxon::Result<void> FindAssets();
maxon::Result<void> CreateMaterialAsset();
maxon::Result<void> CreateObjectAsset();
maxon::Result<void> CreateSceneAsset();

#endif // ASSET_API_BASICS_ASSETS_H__

