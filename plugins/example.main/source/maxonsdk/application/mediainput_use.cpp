// Maxon API header files
#include "maxon/configuration.h"
#include "maxon/execution.h"
#include "maxon/gfx_image.h"
#include "maxon/gfx_image_pixelformats.h"
#include "maxon/gfx_image_storage.h"
#include "maxon/lib_math.h"
#include "maxon/mediasession_image_export_png.h"
#include "maxon/mediasession_input.h"
#include "maxon/url.h"

namespace maxonsdk
{
// ------------------------------------------------------------------------
/// A configuration variable to enable the execution of SaveAsPNGExecution.
// ------------------------------------------------------------------------
MAXON_CONFIGURATION_STRING(g_maxonsdk_saveaspng, "", maxon::CONFIGURATION_CATEGORY::REGULAR, "Load the given image file and save it as PNG.");

// ------------------------------------------------------------------------
/// SaveAsPNGExecution loads a given image file and saves it as PNG.
// ------------------------------------------------------------------------
class SaveAsPNGExecution : public maxon::ExecutionInterface<SaveAsPNGExecution>
{
public:
	maxon::Result<void> operator ()()
	{
		iferr_scope_handler
		{
			// print error
			err.DiagOutput();
			err.DbgStop();
			return err;
		};

		// check configuration variable
		if (g_maxonsdk_saveaspng.IsEmpty())
			return maxon::OK;

		// check file
		const maxon::Url file(g_maxonsdk_saveaspng);
		if (file.IoDetect() != maxon::IODETECT::FILE)
			return maxon::IllegalArgumentError(MAXON_SOURCE_LOCATION);

		// image data
		const maxon::ImageTextureRef textureRef = maxon::ImageTextureClasses::TEXTURE().Create() iferr_return;

		MAXON_SCOPE
		{
			// load image file
			DiagnosticOutput("Load file: @", file);

			const maxon::FileFormatHandler importFileFormat = maxon::FileFormatDetectionInterface::Detect<maxon::MediaInputRef>(file) iferr_return;
			const maxon::MediaInputRef		 source = importFileFormat.CreateHandler<maxon::MediaInputRef>(file) iferr_return;

			const maxon::MediaOutputTextureRef destination = maxon::MediaOutputTextureClass().Create() iferr_return;
			destination.SetOutputTexture(textureRef, maxon::ImagePixelStorageClasses::Normal()) iferr_return;

			const maxon::MediaSessionRef session = maxon::MediaSessionObject().Create() iferr_return;
			session.ConnectMediaConverter(source, destination) iferr_return;
			session.Convert(maxon::TimeValue(), maxon::MEDIASESSIONFLAGS::NONE) iferr_return;
			session.Close() iferr_return;
		}

		MAXON_SCOPE
		{
			// save as PNG
			maxon::Url targetFile = file;
			targetFile.SetSuffix("png"_s) iferr_return;

			DiagnosticOutput("Save file: @", targetFile);

			const maxon::MediaOutputUrlRef destination = maxon::ImageSaverClasses::Png().Create() iferr_return;

			maxon::MediaSessionRef session = maxon::MediaSessionObject().Create() iferr_return;
			textureRef.Save(targetFile, destination, maxon::MEDIASESSIONFLAGS::RUNONLYANALYZE, &session) iferr_return;
			session.Convert(maxon::TimeValue(), maxon::MEDIASESSIONFLAGS::NONE) iferr_return;
			session.Close() iferr_return;
		}

		return maxon::OK;
	}
};
}

namespace maxon
{
// ------------------------------------------------------------------------
/// Registers the implementation at ExecutionJobs.
// ------------------------------------------------------------------------
MAXON_DECLARATION_REGISTER(ExecutionJobs, "net.maxonexample.execution.saveaspng")
{
	return NewObj(maxonsdk::SaveAsPNGExecution);
}
}
