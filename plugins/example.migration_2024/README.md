# example.migration_2024

Provides an overview of the fundamental API changes introduced with Cinema 4D 2024.0 API.

The examples can be run from the menu entry `Extensions/example.migration_2024` inside Cinema 4D 
once the SDK has been compiled and loaded.

## Files
| File | Description |
| :- | :- |
| oboundingbox.cpp | Demonstrates the common changes of a #NodeData derived plugin that has been migrated to the 2024 API. |
| oboundingbox_legacy.cpp | Exemplifies a #NodeData derived plugin that is in a pre-2024.0 API state which should be converted. |
| change_examples.cpp | Contains examples for the not directly `NodeData` related 2024.0 API changes. |
| _migration_example_plugin.cpp | Contains boilerplate code to register and run the 2024 migration examples. |

## Examples in `change_examples.cpp`

| Function | Description |
| :- | :- |
| InstantiateDescID | Demonstrates how to construct compile-time and runtime constant `DescID` identifiers. |
| AccessNodeDataContainer | Demonstrates how to access the data container of scene elements. |
| AccessNodeBranches | Demonstrates the slightly changed syntax of accessing node branches. |
| CopyOnWriteSceneData | Demonstrates the copy-on-write changes made to `VariableTag` and `BaseSelect`. |
| AvoidingDictionaries | Demonstrates the new and performant type `maxon::AttributeTuple` to store and index data with `MAXON_ATTRIBUTE` keys which are known at compile-time. |
| CustomDataTypeAccess | Demonstrates how to access custom data type fields in `BaseContainer` and `GeData` instances. |
| GradientSampling | Demonstrates the changes made to `Gradient` sampling. |
| FieldSampling | Demonstrates the changes made to `FieldObject` sampling. |
| CastingStyles | Outlines the slightly changed style recommendations for casting between data types. |