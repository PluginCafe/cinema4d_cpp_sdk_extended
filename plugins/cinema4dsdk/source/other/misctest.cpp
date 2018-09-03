/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) MAXON Computer GmbH, all rights reserved            //
/////////////////////////////////////////////////////////////

// example code for using BaseArray, BlockArray, PointerArray, SortedArray and BaseList

//////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "maxon/pointerarray.h"
#include "maxon/blockarray.h"
#include "maxon/sortedarray.h"
#include "maxon/baselist.h"

namespace maxon
{
static void	BaseArrayDemo();
static void GenericArrayDemo();
static void	PointerArraySpecificDemo();
static void	SortedArrayDemo();
static void	ListGetNextDemo();
}

void MiscTest()
{
	maxon::BaseArrayDemo();																					// demonstrates basic use of the arrays
	maxon::GenericArrayDemo();																				// shows that BaseArray, BlockArray, PointerArray etc. are interchangable
	maxon::PointerArraySpecificDemo();																// just a few methods specific to the PointerArray
	maxon::SortedArrayDemo();																				// how to use SortedArray
	maxon::ListGetNextDemo();																				// how to add old style GetNext()/GetPrev() functionality to a list element
}

namespace maxon
{

// This function demonstrates the basic methods of our arrays and lists.
// To keep it simple it does not check for errors. For real-life code
// you have to check the return value of Append() or Insert() before you
// access the memory.
static void	BaseArrayDemo()
{
  iferr_scope_handler
  {
    return;
  };

	BaseArray<Int>	test;																		// this could be BlockArray, PointerArray, SortedArray or BaseList
	Int							copyMe = 42;
	Int							i;

	test.Append() iferr_return;															// append an element with default value
	test.Append(copyMe) iferr_return;												// append a copy of copyMe
	test.Insert(1) iferr_return;														// insert an element with default value at index 1
	test.Insert(2, copyMe) iferr_return;										// insert a copy of copyMe at index 2
	test.Erase(0) iferr_return;															// erase element at index 0
	test[2] = 12345;																				// assign a value to the element at index 2

	// iterate over all elements, assign value
	for (i = 0; i < test.GetCount(); i++)
		test[i] = i;

	test.Resize(27) iferr_return;														// the array has now 27 elements
	test.Erase(10, 15) iferr_return;												// erase 15 elements from index 10 on
	test.Append(9876) iferr_return;
	test.Append(54321) iferr_return;

	// iterate over all elements, check for some value
	for (AutoIterator<BaseArray<Int> > it(test); it; ++it)
	{
		if (*it == 9876)
			break;																								// BTW: the index of this element is it - test.Begin();
	}
}

// a class for the following sample code
class DemoElement
{
public:
	explicit DemoElement(Int32 a = 1, Int32 b = 2, Int32 c = 3, Int32 d = 4) : _a(a), _b(b), _c(c), _d(d)
	{
	}
	bool operator !=(const DemoElement& x) const
	{
		return (_a != x._a) || (_b != x._b) || (_c != x._c) || (_d != x._d);
	}
	Int32	_a, _b, _c, _d;
};

// Template method that works for BaseArray, BlockArray, PointerArray or BaseList
template <typename ARRAY, typename T> static void GenericSample(ARRAY& test, const T& aValue, const T& bValue)
{
  iferr_scope_handler
  {
    return;
  };

	const maxon::Int	ARRAY_TEST_SIZE = 1024;
	maxon::Int	i;

	// append ARRAY_TEST_SIZE elements (all initialized by the default constructor)
	for (i = 0; i < ARRAY_TEST_SIZE; i++)
	{
  	test.Append() iferr_return;
  }

	// use an AutoIterator to iterate over the whole array and assign aValue to every element
	for (AutoIterator<ARRAY> it(test); it; ++it)
		*it = aValue;

	// insert an element with bValue at index 10
	test.Insert(10, bValue) iferr_return;

	// erase the element at index 11
	test.Erase(11) iferr_return;

	// just a quick check: we should still have the same number of elements
	if (test.GetCount() != ARRAY_TEST_SIZE)
		CriticalStop();

	// using an Iterator: assign bValue to all elements from test[25] to test[49]
	typename ARRAY::Iterator end = test.Begin() + 50;
	for (typename ARRAY::Iterator it = test.Begin() + 25; it != end; it++)
		*it = bValue;
}

// This function demonstrates that you can use the same code, no matter if your data
// structure is a BaseArray, BlockArray, PointerArray or BaseList
static void GenericArrayDemo()
{
	DemoElement	aValue(5, 6, 7, 8);
	DemoElement	bValue(9, 10, 11, 12);
	BaseArray<DemoElement>		baseArrayTest;
	BlockArray<DemoElement>		blockArrayTest;
	PointerArray<DemoElement>	pointerArrayTest;
	BaseList<DemoElement>			baseListTest;

	// And now for the interesting part: you can use the same code independent of the data structure
	GenericSample(baseArrayTest, aValue, bValue);
	GenericSample(blockArrayTest, aValue, bValue);
	GenericSample(pointerArrayTest, aValue, bValue);
	GenericSample(baseListTest, aValue, bValue);

	// Compare the content of the BaseArray with the BaseList: should be identical if everything works as advertised
	AutoIterator<BaseArray<DemoElement> > iteratorA(baseArrayTest);
	AutoIterator<BaseList<DemoElement> >	iteratorB(baseListTest);

	/*
	while (iteratorA && iteratorB)
	{
		if (*iteratorA++ != *iteratorB++)
			CriticalStop();
	}
	*/
}

// just a few methods specific to the PointerArray
static void	PointerArraySpecificDemo()
{
  iferr_scope_handler
  {
    return;
  };

	PointerArray<DemoElement> test;
	DemoElement* p;

	p = NewObjClear(DemoElement, 11, 22, 33, 44);
	if (p)
  {
		test.AppendPtr(p) iferr_return;
  }

	p = NewObjClear(DemoElement, 55, 66, 77, 88);
	if (p)
  {
		test.InsertPtr(0, p) iferr_return;
  }

	if (test.PopPtr(&p))
		DeleteObj(p);
}


struct MySortedIntegerArray : public SortedArray<MySortedIntegerArray, BaseArray<Int> >
{
	// your sorted array must implement a LessThan() method
	static inline maxon::Bool LessThan(Int a, Int b)	{ return a < b; }
};

// a little demonstration of a SortedArray of VLONGs
static void	SortedArrayDemo()
{
  iferr_scope_handler
  {
    return;
  };

	MySortedIntegerArray test;

	// append some values
	test.Append(25) iferr_return;
	test.Append(2) iferr_return;
	test.Append(900) iferr_return;
	test.Append(77) iferr_return;
	test.Append(78) iferr_return;
	test.Append(79) iferr_return;
	test.Append(43) iferr_return;
	test.Insert(3, 12345) iferr_return;

	// now check if they are sorted
	for (Int i = 1; i < test.GetCount(); i++)
	{
		if (test[i] < test[i - 1])
			CriticalStop();
	}
}


// If you need GetNext() or GetPrev() for your list node  (either because is
// more convenient, speed doesn't matter or it's required for compatibility)
// you derive your data from BaseListLegacyNode
class DemoListNode : public BaseListLegacyNode<DemoListNode>
{
public:
	DemoListNode() : _a(0), _b(0)
	{
	}

	Int	_a;
	Int	_b;
};


// a little demonstration how this works
static void	ListGetNextDemo()
{
  iferr_scope_handler
  {
    return;
  };

	BaseList<DemoListNode> test;

	// just append a few nodes
	test.Append() iferr_return;
	test.Append() iferr_return;
	test.Append() iferr_return;
	test.Append() iferr_return;
	test.Append() iferr_return;

	// range-based for-loop
	Int cntA = 0;
	for (DemoListNode& node : test)
	{
		node._a = 1;
		node._b = 2;
		cntA++;
	}

	// iterate over a list using Iterator - a little bit more to write
	Int cntB = 0;
	for (BaseList<DemoListNode>::Iterator it = test.Begin(); it != test.End(); it++)
	{
		it->_a = 3;
		it->_b = 4;
		cntB++;
	}

	// iterate over a list using AutoIterator
	Int cntC = 0;
	for (AutoIterator<BaseList<DemoListNode> > it(test); it; it++)
	{
		it->_a = 5;
		it->_b = 6;
		cntC++;
	}

	if ((cntA != cntB) || (cntA != cntC))
		CriticalStop();
}

}
