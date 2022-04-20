/*
  Asset API Examples - Metadata
  Copyright (C) 2022 MAXON Computer GmbH

  Author: Ferdinand Hoppe
  Date: 01/04/2022
  SDK: R26

  Contains the Asset API examples related to the topic of metadata.
*/
#ifndef EXAMPLES_METADATA_H__
#define EXAMPLES_METADATA_H__

#include "maxon/apibase.h"
#include "maxon/assets.h"

//! [declare_custom_metadata_attribute]
// Declares a custom metadata attribute that denotes if an asset has been "touched" by the SDK 
// examples. The files generated by the source processor for this header file must be included 
// for this to compile.
#include "examples_metadata1.hxx"
namespace maxon::ASSETMETADATA
{
  MAXON_ATTRIBUTE(maxon::Bool, SDK_TOUCHED, "net.maxonexample.asset.sdk_touched");
};
#include "examples_metadata2.hxx"
//! [declare_custom_metadata_attribute]
 

/// Accesses the data attached to an asset description.
/// 
/// Highlights the meta information provided by an asset description for its asset that is not 
/// being accessed with the asset metadata container.
/// 
/// @param[in] assetDescription    The asset description to access data for.
maxon::Result<void> AccessAssetDescriptionData(const maxon::AssetDescription& assetDescription);


/// Adds a version to a file asset of subtype object that only contains a sphere object.
/// 
/// @param[in] asset    The asset description of the asset to add a version to.
/// @return             The new version of the asset.
maxon::Result<maxon::AssetDescription> AddAssetVersion(const maxon::AssetDescription& asset);


/// Finds a all category assets with a given name and category.
/// 
/// Category names are not unique as there can be two assets named 'B' within the asset category
/// 'A'. This example returns all categories that have a given name and optionally a specific 
/// parent category. Passing in the empty id for the parent category will match categories at the 
/// root level.
/// 
/// This approach of retrieving asset categories is error prone, since there can be multiple
/// AssetCategoryInterface instances for a single location which live in different repositories.
/// It should only be used when a category identifier can neither be predicted nor hardcoded by 
/// looking it up in the Asset Browser. See Asset API - Related Topics - Development Tools for 
/// details on retrieving asset identifiers in the Asset Browser. See Asset API Metadata - 
/// GenerateAssetIdentifiers() for details on determinsitic Asset identifier generation.
/// 
/// @param[in] repository      The repository which contains the categories.
/// @param[in, out] results    The found category assets. This can be prepopulated with category ids 
///                            which should be ignored.
/// @param[in] serachName      The category name to match.
/// @param[in] category        The parent category to match.
/// @param[in] testCategory    If to test for parent category.
maxon::Result<void> FindCategoryAssetsByName(
  const maxon::AssetRepositoryRef& repository, maxon::BaseArray<maxon::AssetDescription>& results,
  const maxon::String& serachName, const maxon::Id& category = maxon::Id(),
  const maxon::Bool& testCategory = false);


/// Finds all category assets that match a given category path.
/// 
/// Categories within the path must be separated by forward slashes. The category path 'A/B/C' will
/// return all categories named 'C' with ancestors named 'A' and 'B', in the order A->B->C. 
/// 
/// This approach of retrieving asset categories is very error prone, since there can be multiple
/// AssetCategoryInterface instances for a single location which live in different repositories.
/// It should only be used when a category identifier can neither be predicted nor hardcoded by 
/// looking it up in the Asset Browser. See Asset API - Related Topics - Development Tools for 
/// details on retrieving asset identifiers in the Asset Browser. See Asset API Metadata - 
/// GenerateAssetIdentifiers() for details on determinsitic Asset identifier generation.
/// 
/// @param[in] repository      The repository which contains the categories.
/// @param[out] results        The found category assets.
/// @param[in] categoryPath    The path to the categories.
/// @param[in] relativePath    If true, the search can start anywhere in the tree. If false, the 
///                            search always does start at the root.

maxon::Result<void> FindCategoryAssetsByPath(
  const maxon::AssetRepositoryRef& repository, maxon::BaseArray<maxon::AssetDescription>& results,
  const maxon::String& categoryPath, const maxon::Bool& relativePath = false);


/// Demonstrates how to generate asset identifiers.
/// 
/// Asset identifiers must be unique within a repository without its bases. UUID-based identifiers
/// generated by the operating system assure this quality but are also hard to reproduce at a later 
/// point of time or on another system. In some cases this quality of predictable asset identifiers 
/// is required to search for assets effectively or avoid ingesting duplicate assets. The example
/// demonstrates with the cases of a category and image asset how such identifying asset hashes
/// could be constructed with the maxon API in a sufficiently collision free manner.
maxon::Result<void> GenerateAssetIdentifiers();


/// Iterates over all existing entries in an AssetMetadata instance. 
/// 
/// @param[in] assetDescription    The asset description to print out the metadata for.
maxon::Result<void> IterateAssetMetadata(const maxon::AssetDescription& assetDescription);


/// Reads the metadata of an asset that are commonly required to be read. 
/// 
/// @param[in] assetDescription    The asset description to read the metadata for.
maxon::Result<void> ReadAssetMetadata(const maxon::AssetDescription& assetDescription);


/// Writes the metadata of an asset that are commonly required to be written. 
/// 
/// @param[in] assetDescription    The asset description to write the metadata for.
maxon::Result<void> WriteAssetMetadata(const maxon::AssetDescription& assetDescription);
#endif // EXAMPLES_METADATA_H__
