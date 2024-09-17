// Maxon API header files
#include "maxon/streamconversion.h"
#include "maxon/streamconversion_impl_helper.h"

// local header files
#include "streamconversion_declarations.h"

namespace maxon
{

// ------------------------------------------------------------------------
/// An implementation of StreamConversionInterface implementing a Caesar cipher.
// ------------------------------------------------------------------------
class MaxonSDKCaesarCipherImpl : public Component<MaxonSDKCaesarCipherImpl, StreamConversionInterface>
{
	MAXON_COMPONENT(NORMAL, StreamConversionBaseClass);

public:
	Result<void> FactoryInit(FactoryInterface::ConstPtr factory, const DataDictionary& settings)
	{
		// this function is called by the factory

		_shift = settings.Get(maxon::MAXONSDK_CAESAR_CIPHER_OPTIONS::SHIFT, Int32(0));
		return OK;
	}

	MAXON_METHOD const DataType& GetSourceType() const
	{
		// Caesar chiper supports only Char
		return GetDataType<Char>();
	}

	MAXON_METHOD const DataType& GetDestinationType() const
	{
		// Caesar chiper supports only Char
		return GetDataType<Char>();
	}

	MAXON_METHOD Id GetCounterpart() const
	{
		// Caesar cipher is its own counterpart
		return Id();
	}

	MAXON_METHOD Result<Int> ConvertImpl(const Block<const Generic>& src, WritableArrayInterface<Generic>& xdst, Int dstLimitHint, Bool inputFinished, Bool& outputFinished)
	{
		// implement this function to define the encryption algorithm

		iferr_scope;

		const Char asciiA = 65;
		const Char asciiZ = 90;

		// get source data
		const Block<const Char>& block = reinterpret_cast<const Block<const Char>&>(src);
		// prepare destination data
		ArrayAppendCache<Char> destination(xdst);

		for (const Char character : block)
		{
			// check if valid upper case ASCII character
			if ((character >= asciiA && character <= asciiZ))
			{
				// shift
				const Int32 number = (Int32)(character - asciiA);
				const Int32 newNumber	 = Mod((number + _shift), 26);
				const Char	resultChar = (Char)(asciiA + newNumber);

				destination.Append(resultChar) iferr_return;
			}
			else
			{
				return IllegalArgumentError(MAXON_SOURCE_LOCATION, "Caesar Cipher only accepts upper case ASCII."_s);
			}
		}

		outputFinished = true;
		destination.Finalize() iferr_return;

		return destination.GetCount();
	}

private:
	Int32 _shift = 0;	///< shift value
};

// ------------------------------------------------------------------------
/// Register the implementation.
// ------------------------------------------------------------------------
MAXON_COMPONENT_CLASS_REGISTER(MaxonSDKCaesarCipherImpl, "net.maxonexample.class.caesarcipher");

// ------------------------------------------------------------------------
/// Register the factory for that implementation.
// ------------------------------------------------------------------------
MAXON_DECLARATION_REGISTER(StreamConversions::MaxonSDKCaesarCipher)
{
	iferr_scope;

	auto factoryFunction = &MaxonSDKCaesarCipherImpl::FactoryInit;
	auto factory = StreamConversionFactory::CreateObjectFactory(factoryFunction) iferr_return;

	// is encoder
	factory.Set(STREAMCONVERSIONFACTORYFLAGS::ISENCODER, true) iferr_return;

	return factory;
}
}


