/*
  Asset API Basics Example Plugin
  Copyright (C) 2021 MAXON Computer GmbH

  Author: Ferdinand Hoppe
  Date: 12/08/2021

  Provides the database and repository related example functions.
*/
#ifndef ASSET_API_BASICS_DATABASES_H__
#define ASSET_API_BASICS_DATABASES_H__

maxon::Result<void> AccessDatabases();
maxon::Result<void> FindRepositories();
maxon::Result<void> CreateRepositories();
maxon::Result<void> AccessImportantRepositories();
maxon::Result<void> AddDatabase();
maxon::Result<void> RemoveDatabase();
maxon::Result<void> ActivateDatabase();
maxon::Result<void> DeactivateDatabase();
// maxon::Result<void> ReactToDatabaseEvents();

#endif // ASSET_API_BASICS_DATABASES_H__
