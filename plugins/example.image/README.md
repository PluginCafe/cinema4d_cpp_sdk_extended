# examples.image

Contains the color management and OpenColorIO (OCIO) examples for the Image API.

The examples can be run from the menu entry `Extensions/examples.image` inside Cinema 4D once the 
extended SDK has been compiled and loaded.

## Files
| File | Description |
| :- | :- |
| plugin_image_api_examples.h | Contains the plugin layer used to invoke the provided examples. |
| examples_color_management.h | Contains the color management examples for the Image API. |
| examples_ocio.h | Contains the OCIO examples for the Image API. |

## Examples in `examples_color_management.h`

| Function | Description |
| :- | :- |
| GetColorProfilesFromColorSpaces | Instantiate color profiles from the builtin color spaces and pixel formats. |
| GetColorProfilesFromFile | Instantiate color profiles from ICC color profile files and the builtin pixel formats. |
| GetColorProfileMetadata | Read color profile metadata such as description strings and supported pixel formats. |
| WriteColorProfileToFile | Write a color profile object to an ICC color profile file on disk. |
| GetPixelFormats | Access builtin pixel formats and their associated metadata, such as formatting groups, number of channels, and memory layout per channel. |
| ConvertSinglePixelWithColorProfile | Convert color data pixel by pixel with color profiles and/or pixel formats. |
| ConvertManyPixelWithColorProfile | Convert color data in chunks of pixels with color profiles and/or pixel formats. |
| ConvertTextureWithColorProfile | Read color data from a bitmap to a buffer and convert this buffer with color profiles and/or pixel formats. |
| ConvertColorWithUtils | Convert colors between common color representations such as RGB, HSL, and CMYK. |

## Examples in `examples_ocio.h`

| Function | Description |
| :- | :- |
| ConvertSceneOrElements | Convert all colors contained in a document or contained in a set of elements in a document to a new OCIO Render space. |
| CopyColorManagementSettings | Copy all color management settings from one document to another. |
| GetSetColorManagementSettings | Read and write the OCIO color management settings stored in a document. |
| ConvertOcioColors | Convert colors along OCIO conversion paths defined by the OCIO color spaces associated with a document. |
| GetSetColorValuesInOcioDocuments | Read and write color parameters in an OCIO document either in the native %Render space of the document or other spaces. |
| GetSetBitmapOcioProfiles | Read and write the OCIO color spaces associated with #BaseBitmap instances to change or nullify parts of an OCIO conversion path for them. |