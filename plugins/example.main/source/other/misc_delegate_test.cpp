/////////////////////////////////////////////////////////////
// Cinema 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) MAXON Computer GmbH, all rights reserved            //
/////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

#include "main.h"

namespace maxon
{
static Result<void> CallDelegates();
static Result<void> CallLambdaDelegates();
static Result<void> CallByReferenceDelegates();
}

void MiscDelegateTest()
{
  iferr_scope_handler
  {
    return;
  };

	maxon::CallDelegates() iferr_return;
	maxon::CallLambdaDelegates() iferr_return;
	maxon::CallByReferenceDelegates() iferr_return;
}

namespace maxon
{

class SampleClass
{
public:
	virtual ~SampleClass()
	{
	}

	static void MethodByValue(const Char* str)
	{
		DiagnosticOutput("@", str);
	}

	static void MethodByLValueReference(const Char*& str)
	{
		DiagnosticOutput("@", str);
	}

	static void MethodByRValueReference(Char*&& str)
	{
		DiagnosticOutput("@", str);
	}

	static void TestWithoutError()
	{
		DiagnosticOutput("static Test()");
	};

	static Result<void> Test()
	{
		DiagnosticOutput("static Test()");
		return OK;
	};

	virtual Result<void> operator ()()
	{
		DiagnosticOutput("virtual operator ()()");
		return OK;
	}
	virtual const Char* GetName() const
	{
		DiagnosticOutput("virtual GetName()");
		return "SampleClass";
	}
};

static Result<void> CallDelegates()
{
	iferr_scope;
	UniqueRef<SampleClass> sample = NewObj(SampleClass) iferr_return;

	// Assign a virtual method pointer (to a const method).
	Delegate<const Char*()> fn(sample.GetPointer(), &SampleClass::GetName);
	fn();

	fn = Delegate<const Char*()>::CreateByReference<SampleClass, &SampleClass::GetName>(sample);
	fn();

	// Assign a static function pointer
	Delegate<Result<void>()> fn2(&SampleClass::Test);
	fn2() iferr_return;

	fn2 = Delegate<Result<void>()>::Create<&SampleClass::Test>() iferr_return;
	fn2() iferr_return;

	fn2 = &SampleClass::Test;
	fn2() iferr_return;

	Int x = 5;

	auto test3 = [x](Int a) -> Result<void>
	{
		if (a == 42)
			return OK;

		if (x == 0)
			return OK;

		return IllegalArgumentError(MAXON_SOURCE_LOCATION);
	};
	Delegate<Result<void>(Int)> fn3(test3);
	fn3(42) iferr_return;

	// Assign a virtual method pointer (method is not const).
	Delegate<Result<void>()> fn4 = Delegate<Result<void>()>::CreateByReference<SampleClass, &SampleClass::operator()>(sample);
	fn4() iferr_return;

	return OK;
}

static Result<void> CallLambdaDelegates()
{
	iferr_scope;

	// Parameter by value.
	auto lambdaByValue =
		[](const Char* str)
		{
			DiagnosticOutput("@", str);
		};
	Delegate<void(const Char*)> fn5(lambdaByValue);
	if (fn5.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	fn5("Lambda ByValue constructor");

	fn5 = Delegate<void(const Char*)>::Create(lambdaByValue) iferr_return;
	if (fn5.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	fn5("Lambda ByValue From");

	fn5 = lambdaByValue;
	if (fn5.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	fn5("Lambda ByValue assignment");

	// Parameter by lvalue reference.
	auto lambdaByLValueReference =
		[](const Char*& str)
		{
			DiagnosticOutput("@", str);
		};
	Delegate<void(const Char*&)> fn6(lambdaByLValueReference);
	if (fn6.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	const Char* sampleFunctionByReferenceStr = "Lambda ByLValueReference constructor";
	fn6(sampleFunctionByReferenceStr);

	fn6 = Delegate<void(const Char*&)>::Create(lambdaByLValueReference) iferr_return;
	if (fn6.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	sampleFunctionByReferenceStr = "Lambda ByLValueReference From";
	fn6(sampleFunctionByReferenceStr);

	fn6 = lambdaByLValueReference;
	if (fn6.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	sampleFunctionByReferenceStr = "Lambda ByLValueReference assignment";
	fn6(sampleFunctionByReferenceStr);

	using CharPtr = Char*;

	// Parameter by rvalue reference.
	auto lambdaByRValueReference =
		[](CharPtr&& str)
		{
			DiagnosticOutput("@", str);
		};
	Delegate<void(CharPtr&&)> fn7(lambdaByRValueReference);
	if (fn7.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	Char* sampleFunctionMoveStr = const_cast<Char*>("Lambda ByRValueReference constructor");
	fn7(std::move(sampleFunctionMoveStr));

	fn7 = Delegate<void(Char*&&)>::Create(lambdaByRValueReference) iferr_return;
	if (fn7.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	sampleFunctionMoveStr = const_cast<Char*>("Lambda ByRValueReference From");
	fn7(std::move(sampleFunctionMoveStr));

	fn7 = lambdaByRValueReference;
	if (fn7.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	sampleFunctionMoveStr = const_cast<Char*>("Lambda ByRValueReference assignment");
	fn7(std::move(sampleFunctionMoveStr));


	return OK;
}

static Result<void> CallByReferenceDelegates()
{
	iferr_scope;

	// Parameter by value.
	auto lambdaByValue =
		[](const Char* str)
		{
			DiagnosticOutput("@", str);
		};

	auto fn5 = Delegate<void(const Char*)>::CreateByReference(lambdaByValue);
	if (fn5.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	fn5("Lambda ByValue CreateByReference");

	// Parameter by lvalue reference.
	auto lambdaByLValueReference =
		[](const Char*& str)
		{
			DiagnosticOutput("@", str);
		};
	auto fn6 = Delegate<void(const Char*&)>::CreateByReference(lambdaByLValueReference);
	if (fn6.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	const Char* sampleFunctionByReferenceStr = "Lambda ByLValueReference CreateByReference";
	fn6(sampleFunctionByReferenceStr);

	using CharPtr = Char*;

	// Parameter by rvalue reference.
	auto lambdaByRValueReference =
		[](CharPtr&& str)
		{
			DiagnosticOutput("@", str);
		};
	auto fn7 = Delegate<void(CharPtr&&)>::CreateByReference(lambdaByRValueReference);
	if (fn7.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	Char* sampleFunctionMoveStr = const_cast<Char*>("Lambda ByRValueReference CreateByReference");
	fn7(std::move(sampleFunctionMoveStr));


	// Assign a virtual method pointer (to a const method).
	UniqueRef<SampleClass> sample = NewObj(SampleClass) iferr_return;
	auto fn8 = Delegate<const Char*()>::CreateByReference<SampleClass, &SampleClass::GetName>(sample);
	if (fn8.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	fn8();

	fn8 = Delegate<const Char*()>::CreateByReference(sample.GetPointer(), &SampleClass::GetName);
	if (fn8.IsStaticFunctionPointer() == true)
		return IllegalStateError(MAXON_SOURCE_LOCATION);
	fn8();

	return OK;
}


}
