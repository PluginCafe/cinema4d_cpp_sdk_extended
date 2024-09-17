// Maxon API header files
#include "maxon/execution.h"
#include "maxon/configuration.h"
#include "maxon/url.h"
#include "maxon/streamconversion.h"

// local header files
#include "streamconversion_declarations.h"

namespace maxonsdk
{
// ------------------------------------------------------------------------
/// A configuration variable to enable the execution of TestCaesarCipherExecution.
// ------------------------------------------------------------------------
MAXON_CONFIGURATION_BOOL(g_maxonsdk_testcaesarcipher, false, maxon::CONFIGURATION_CATEGORY::REGULAR, "Test Caesar Cipher.");

// ------------------------------------------------------------------------
/// An implementation of ExecutionInterface that will encrypt and decrypt
/// some text using the Caesar cipher.
// ------------------------------------------------------------------------
class TestCaesarCipherExecution : public maxon::ExecutionInterface<TestCaesarCipherExecution>
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
		if (!g_maxonsdk_testcaesarcipher)
			return maxon::OK;

		maxon::String			 cipherText;
		const maxon::Int32 shift = 10;

		MAXON_SCOPE
		{
			// encode

			// define settings
			maxon::DataDictionary caesarCipherSettings;
			caesarCipherSettings.Set(maxon::MAXONSDK_CAESAR_CIPHER_OPTIONS::SHIFT, shift) iferr_return;

			// create instance
			const maxon::StreamConversionRef caesarCipherEncoder = maxon::StreamConversions::MaxonSDKCaesarCipher().Create(caesarCipherSettings) iferr_return;

			// encode
			const maxon::String secretText("VENIVIDIVICI");
			const maxon::BaseArray<maxon::Char> data = secretText.GetCString() iferr_return;
			maxon::BaseArray<maxon::Char>				encodedData;
			caesarCipherEncoder.ConvertAll(data, encodedData) iferr_return;

			cipherText = maxon::String(encodedData);
		}

		DiagnosticOutput("Cipher Text: @", cipherText);

		MAXON_SCOPE
		{
			// decode

			// define settings
			maxon::DataDictionary caesarCipherSettings;
			caesarCipherSettings.Set(maxon::MAXONSDK_CAESAR_CIPHER_OPTIONS::SHIFT, -shift) iferr_return;

			// create instance
			const maxon::StreamConversionRef caesarCipherEncoder = maxon::StreamConversions::MaxonSDKCaesarCipher().Create(caesarCipherSettings) iferr_return;

			// decode
			const maxon::BaseArray<maxon::Char> data = cipherText.GetCString() iferr_return;
			maxon::BaseArray<maxon::Char>				decodedData;
			caesarCipherEncoder.ConvertAll(data, decodedData) iferr_return;

			const maxon::String plainText(decodedData);
			DiagnosticOutput("Secret Text: @", plainText);
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
MAXON_DECLARATION_REGISTER(ExecutionJobs, "net.maxonexample.execution.testcaesarciper")
{
	return NewObj(maxonsdk::TestCaesarCipherExecution);
}
}
