// MAXON API header files
#include "maxon/configuration.h"
#include "maxon/execution.h"
#include "maxon/gfx_image.h"
#include "maxon/gfx_image_pixelformats.h"
#include "maxon/gfx_image_storage.h"
#include "maxon/lib_math.h"
#include "maxon/mediasession_input.h"
#include "maxon/url.h"

// local header files
#include "mediaoutput_declarations.h"

namespace maxonsdk
{
// ------------------------------------------------------------------------
/// A configuration variable to enable the execution of SaveAsIMAGEExecution.
// ------------------------------------------------------------------------
MAXON_CONFIGURATION_STRING(g_maxonsdk_saveasimage, "", maxon::CONFIGURATION_CATEGORY::REGULAR, "Load the given image file and save it as *.image.");

// ------------------------------------------------------------------------
/// An implementation of ExecutionInterface that will load a given file
/// and save it as *.image.
// ------------------------------------------------------------------------
class SaveAsIMAGEExecution : public maxon::ExecutionInterface<SaveAsIMAGEExecution>
{
public:
	maxon::Result<void> operator ()()
	{
		iferr_scope_handler
		{
			err.DiagOutput();
			err.DbgStop();
			return err;
		};

		// check configuration variable
		if (g_maxonsdk_saveasimage.IsEmpty())
			return maxon::OK;

		// check file
		const maxon::Url file(g_maxonsdk_saveasimage);
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
			// save as MaxonSDK image
			maxon::Url targetFile = file;
			targetFile.SetSuffix("image"_s) iferr_return;

			DiagnosticOutput("Save file: @", targetFile);

			const maxon::MediaOutputUrlRef destination = maxon::ImageSaverClasses::MaxonSDKImage().Create() iferr_return;

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
MAXON_DECLARATION_REGISTER(ExecutionJobs, "net.maxonexample.execution.saveasimage")
{
	return NewObj(maxonsdk::SaveAsIMAGEExecution);
}
}
