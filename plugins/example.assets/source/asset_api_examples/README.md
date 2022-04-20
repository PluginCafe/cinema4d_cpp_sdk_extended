# Asset API Examples
Showcases basic interactions with the Asset API to modify asset databases and their content. The example is delivered as a `GeDialog` plugin with which various code snippets of the Asset API Handbook can be executed. This plugin requires the SDK asset database to function correctly, you must either provide an internet connection on the machine where the plugin is being executed on or download and mount the database manually from [developers.maxon.net](https://developers.maxon.net).

![Asset API Examples Dialog](asset_api_examples_readme.png)

*Fig I - The Asset API Examples plugin opened in the 'Asset Databases' category, containing the asset database related examples.*

| File | Description |
| :- | :- |
| asset_api_examples_plugin.h | Contains the plugin layer used to invoke the provided examples. |
| examples_databases.h | Contains the Asset API examples related to the topic of databases. |
| examples_assets.h | Contains the Asset API examples related to the topic of asset types. |
| examples_metadata.h | Contains the Asset API examples related to the topic of metadata. |
| examples_contexts.h | Contains execution contexts showcasing how to use the examples shown in the manuals. |