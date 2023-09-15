/*
	example.2024_migration
	(C) MAXON Computer GmbH, 2023

	Author: Ferdinand Hoppe
	Date: 22/05/2023

	Contains examples for the not directly `NodeData` related 2024.0 API changes. 
*/
#ifndef CHANGE_EXAMPLES_H__
#define CHANGE_EXAMPLES_H__

#include "c4d_general.h"

#include "maxon/apibase.h"

/// @brief Demonstrates how to construct compile-time and runtime constant `DescID` identifiers.
maxon::Result<void>InstantiateDescID(BaseDocument* doc);

/// @brief Demonstrates how to access the data container of scene elements.
maxon::Result<void>AccessNodeDataContainer(BaseDocument* doc);

/// @brief Demonstrates the slightly changed syntax of accessing node branches.
maxon::Result<void>AccessNodeBranches(BaseDocument* doc);

/// @brief Demonstrates the copy-on-write changes made to `VariableTag` and `BaseSelect`.
maxon::Result<void>CopyOnWriteSceneData(BaseDocument* doc);

/// @brief Demonstrates the new and performant type `maxon::AttributeTuple` to store and index data with `MAXON_ATTRIBUTE` keys which are known at compile-time.
maxon::Result<void>AvoidingDictionaries(BaseDocument* doc);

/// @brief Demonstrates how to access custom data type fields in `BaseContainer` and `GeData` instances.
maxon::Result<void>CustomDataTypeAccess(BaseDocument* doc);

/// @brief Demonstrates the changes made to `Gradient` sampling.
maxon::Result<void>GradientSampling(BaseDocument* doc);

/// @brief Demonstrates the changes made to `FieldObject` sampling.
maxon::Result<void>FieldSampling(BaseDocument* doc);

/// @brief Outlines the slightly changed style recommendations for casting between data types.
maxon::Result<void>CastingStyles(BaseDocument* doc);


#endif // CHANGE_EXAMPLES_H__