/////////////////////////////////////////////////////////////
// Cinema 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) MAXON Computer GmbH, all rights reserved            //
/////////////////////////////////////////////////////////////

#include "main.h"

namespace copymoveconstructorsample
{


//////////////////////////////////////////////////////////////////////////
//
// This source describes how to implement move and copy constructors in classes.
//
// In the API there are strict requirements as to how classes need to be implemented, so that they can function e.g. with sort and array templates.


//////////////////////////////////////////////////////////////////////////
//
// Type I
//
// The first distinction is whether your class is a POD (plain old data) class or not.
//
// POD classes can be copied as a whole by a simple CopyMem() and do not allocate any memory. POD classes do not require any special move/copy constructors.
//
// Example:

class Vector
{
public:
	maxon::Float x, y, z;
};

//////////////////////////////////////////////////////////////////////////
//
// Type II
//
// For all other classes the first step is to use of the MAXON_DISALLOW_COPY_AND_ASSIGN macro at the beginning of the class.
// It declares the class copy constructor and operator private (not all compilers support C++11 deleted functions yet) and
// prevents copy and assign operations which implicitly expect success (or throw an exception).
//
// Example:

class MemoryBlock
{
	MAXON_DISALLOW_COPY_AND_ASSIGN(MemoryBlock);

private:
	void* _memory;
	maxon::Int _memorySize;
};

// To use such a class in combination with (most types of) arrays we need to define a move constructor and operator.
// Move constructors and operators define how to move the class in memory efficiently, without the need to copy the class and its contents.
//
// Example:

class Example1
{
	MAXON_DISALLOW_COPY_AND_ASSIGN(Example1);
public:
	Example1() : _memory(nullptr), _memorySize(0) {}
	~Example1() { DeleteMem(_memory); _memorySize = 0; }
	Example1(Example1&& src) : _memory(std::move(src._memory)), _memorySize(std::move(src._memorySize))
	{
		src._memory = nullptr; // important to clear, otherwise class would be freed twice
	}
	MAXON_OPERATOR_MOVE_ASSIGNMENT(Example1);
private:
	maxon::Char* _memory;
	maxon::Int   _memorySize;
};

// Note: You can use the macro MAXON_MOVE_MEMBERS to make the code more readable and simplify things.
// So instead
// Example1(Example1&& src) : _memory(std::move(src._memory)), _memorySize(std::move(src._memorySize))
// you could write
// Example1b(Example1b&& src) : MAXON_MOVE_MEMBERS(_memory, _memorySize)
//
// The statement MAXON_OPERATOR_MOVE_ASSIGNMENT automatically defines the move operator based on the move constructor, so we don't have to write the same code twice.
//
// After defining the move constructor arrays or sorts can be used (otherwise you'd get a compile error in one of the header files):

static maxon::Bool UseExample1()
{
	maxon::BaseArray<Example1> test;
	iferr (test.Append())
		return false;
	return true;
}

// For derived classes the code is slightly more difficult to write:

class Example2 : public Example1
{
	MAXON_DISALLOW_COPY_AND_ASSIGN(Example2);
public:
	Example2() { _val2 = 1; }
	Example2(Example2&& src) : Example1(std::move(src)), _val2(std::move(src._val2))
	{
		src._val2 = 0;
	}
	MAXON_OPERATOR_MOVE_ASSIGNMENT(Example2);
private:
	maxon::Int _val2;
};

//////////////////////////////////////////////////////////////////////////
//
// Type III
//
// If your class does not only need to be moved in memory, but also needs to be copied or duplicated the member CopyFrom needs to be added:

class Example3
{
	MAXON_DISALLOW_COPY_AND_ASSIGN(Example3);
public:
	Example3() : _memory(nullptr), _memorySize(0) {}
	~Example3() { DeleteMem(_memory); _memorySize = 0; }
	Example3(Example3&& src) : _memory(std::move(src._memory)), _memorySize(std::move(src._memorySize))
	{
		src._memory = nullptr; // important to clear, otherwise class would be freed twice
	}
	maxon::Result<void> CopyFrom(const Example3& src)
	{
    iferr_scope;
  
		DeleteMem(_memory);
		_memorySize = 0;
		_memory = NewMem(Char, src._memorySize) iferr_return;
		_memorySize = src._memorySize;
		return maxon::OK;
	}
	MAXON_OPERATOR_MOVE_ASSIGNMENT(Example3);
private:
	maxon::Char* _memory;
	maxon::Int   _memorySize;
};

// Defining CopyFrom is the only clean way to deal with out of memory situations, which couldn't properly be tracked by using regular C++ copy constructors/operators.
//
// After defining a CopyFrom method you're now able to copy arrays:

static maxon::Bool UseExample3()
{
	maxon::BaseArray<Example3> test, test2;
	iferr (test.CopyFrom(test2))
		return false;
	return true;
}

// or append data by copying it (though this is not the most efficient way to do):

static maxon::Bool UseExample3b()
{
	maxon::BaseArray<Example3> test;
	Example3 val;
	iferr (test.Append(val))
		return false;
	return true;
}

}

void MoveCopyConstructorSample()
{
	copymoveconstructorsample::UseExample1();
	copymoveconstructorsample::UseExample3();
	copymoveconstructorsample::UseExample3b();
}

