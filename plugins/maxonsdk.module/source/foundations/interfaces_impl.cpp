// ------------------------------------------------------------------------
/// This file contains implementations of various custom interfaces.
///
/// "SimpleNumberImpl" is a simple implementation of SimpleNumberInterface. It also shows how to implement a factory.
/// "AdvancedNumberImpl" is an implementation of EvenOddNumberInterface. It shows how to re-implement and use base functions.
/// "SequenceOperationBase" is a component base. It is re-used in "SummationImp" and "MultiplicationImp".
/// "DirectoryElementImpl" is an implementation of DirectoryElementInterface. It uses functions of the base class.
// ------------------------------------------------------------------------

// local header files
#include "interfaces_declarations.h"

namespace maxonsdk
{

// ------------------------------------------------------------------------
/// A simple implementation of SimpleNumberInterface.
// ------------------------------------------------------------------------
class SimpleNumberImpl : public maxon::Component<SimpleNumberImpl, SimpleNumberInterface>
{
	MAXON_COMPONENT();

public:
	MAXON_METHOD void SetNumber(maxon::Int number)
	{
		_value = number;
	}

	MAXON_METHOD maxon::Int GetNumber() const
	{
		return _value;
	}

	// ------------------------------------------------------------------------
	/// factory method to be used with Factory::CreateObjectFactory()
	// ------------------------------------------------------------------------
	maxon::Result<void> FactoryInit(maxon::FactoryInterface::ConstPtr, maxon::Int value)
	{
		_value = value;
		return maxon::OK;
	}

private:
	maxon::Int _value = 0;	///< internally stored value
};

// ------------------------------------------------------------------------
/// Registers the implementation at the given published object.
// ------------------------------------------------------------------------
MAXON_COMPONENT_CLASS_REGISTER(SimpleNumberImpl, SimpleNumber);

// ------------------------------------------------------------------------
/// Implements the given published object by creating a factory.
// ------------------------------------------------------------------------
MAXON_DECLARATION_REGISTER(SimpleNumberFactory)
{
	return SimpleNumberFactoryType::CreateObjectFactory(&SimpleNumberImpl::FactoryInit);
}

// ------------------------------------------------------------------------
/// An implementation of EvenOddNumberInterface.
// ------------------------------------------------------------------------
class EvenOddNumberImpl : public maxon::Component<EvenOddNumberImpl, EvenOddNumberInterface>
{
	// add the component "SimpleNumber" as an implementation of the inherited interface "SimpleNumberInterface".
	MAXON_COMPONENT(NORMAL, SimpleNumber);

public:
	// ------------------------------------------------------------------------
	/// re-implemented function of SimpleNumberInterface
	// ------------------------------------------------------------------------
	MAXON_METHOD void SetNumber(maxon::Int number)
	{
		// check if number is even
		_even = (number % 2 == 0);

		// call "SetNumber()" of the implementation defined with MAXON_COMPONENT()
		super.SetNumber(number);
	}

	MAXON_METHOD maxon::Bool IsEven() const
	{
		return _even;
	}

private:
	// ------------------------------------------------------------------------
	/// "true" if the number is even.
	/// Note: Default value of "true" assumes that a new SimpleNumberInterface
	/// instance is initialized with 0.
	// ------------------------------------------------------------------------
	maxon::Bool _even = true;
};

// ------------------------------------------------------------------------
/// Registers the implementation at the given published object.
// ------------------------------------------------------------------------
MAXON_COMPONENT_CLASS_REGISTER(EvenOddNumberImpl, EvenOddNumber);


// ------------------------------------------------------------------------
/// A base component implementing functions of SequenceOperationInterface.
// ------------------------------------------------------------------------
class SequenceOperationBase : public maxon::ComponentRoot
{
public:
	maxon::Result<void> AddNumber(maxon::Float v)
	{
		iferr_scope;
		_values.Append(v) iferr_return;
		return maxon::OK;
	}

	void Reset()
	{
		_values.Reset();
	}

protected:
	maxon::BaseArray<maxon::Float> _values;	///< array of float values
};

// ------------------------------------------------------------------------
/// An implementation of SequenceOperationInterface. Re-uses SequenceOperationBase.
// ------------------------------------------------------------------------
class SummationImpl : public maxon::ComponentWithBase<SummationImpl, SequenceOperationBase, SequenceOperationInterface>
{
	MAXON_COMPONENT();

public:
	MAXON_METHOD maxon::Float GetResult()
	{
		// calculate the sum of all elements

		maxon::Float sum = 0;

		for (maxon::Float& value : _values)
		{
			sum += value;
		}

		return sum;
	}
};

// ------------------------------------------------------------------------
/// An implementation of SequenceOperationInterface. Re-uses SequenceOperationBase.
// ------------------------------------------------------------------------
class MultiplicationImpl : public maxon::ComponentWithBase<MultiplicationImpl, SequenceOperationBase, SequenceOperationInterface>
{
	MAXON_COMPONENT();

public:
	MAXON_METHOD maxon::Float GetResult()
	{
		// calculate the product of all elements

		if (_values.GetCount() == 0)
			return 0.0;

		maxon::Float product = 1;

		for (maxon::Float& value : _values)
		{
			product *= value;
		}

		return product;
	}
};

// ------------------------------------------------------------------------
/// Registers the implementation at the given published object.
// ------------------------------------------------------------------------
MAXON_COMPONENT_CLASS_REGISTER(SummationImpl, Summation);
// ------------------------------------------------------------------------
/// Registers the implementation at the given published object.
// ------------------------------------------------------------------------
MAXON_COMPONENT_CLASS_REGISTER(MultiplicationImpl, Multiplication);


// ------------------------------------------------------------------------
/// An implementation of DirectoryElementInterface.
// ------------------------------------------------------------------------
class DirectoryElementImpl : public maxon::Component<DirectoryElementImpl, DirectoryElementInterface>
{
	// add the component "HierarchyObjectClass" as an implementation of the inherited interface "HierarchyObjectInterface".
	MAXON_COMPONENT(NORMAL, maxon::HierarchyObjectClass);

public:
	maxon::Result<void> InitComponent()
	{
		_folder = ""_s;
		return maxon::OK;
	}

	MAXON_METHOD void SetFolder(const maxon::String& folder)
	{
		_folder = folder;
	}

	MAXON_METHOD maxon::String GetFolder() const
	{
		return _folder;
	}

	MAXON_METHOD maxon::Result<maxon::Url> GetFullUrl() const
	{
		iferr_scope;

		// check folder
		if (_folder.IsEmpty())
			return maxon::IllegalStateError(MAXON_SOURCE_LOCATION, "Empty Folder"_s);

		maxon::Url url;

		// get parent element
		const auto parentObject = super.GetParent();
		if (parentObject != nullptr)
		{
			// check type of parent element
			if (!parentObject.IsInstanceOf<DirectoryElementRef>())
				return maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "Parent object is no DirectoryElementRef."_s);

			// get Url of parent element
			const DirectoryElementRef parentElement = maxon::Cast<DirectoryElementRef>(parentObject);
			url = parentElement.GetFullUrl() iferr_return;
		}
		else
		{
			// if no parent element, define Url scheme
			url.SetScheme(maxon::URLSCHEME_FILESYSTEM) iferr_return;
		}

		// append local folder
		url.Append(_folder) iferr_return;

		return url;
	}

private:
	maxon::String _folder;	///< the folder component
};

// ------------------------------------------------------------------------
/// Registers the implementation at the given published object.
// ------------------------------------------------------------------------
MAXON_COMPONENT_CLASS_REGISTER(DirectoryElementImpl, DirectoryElement);
}


