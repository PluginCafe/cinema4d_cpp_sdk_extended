//
//  blw_simplegenerator.h
//  blw_simplegenerator
//
//  Created by Riccardo Gigante on 23/07/2019.
//  Copyright © 2019 MAXON Computer GmbH. All rights reserved.
//

#ifndef BLW_SIMPLEGENERATOR_H__
#define BLW_SIMPLEGENERATOR_H__

//------------------------------------------------------------------------------------------------
/// Basic ObjectData implementation responsible for generating a simple plane centered in the
/// origin and sized 200 x 200.
//------------------------------------------------------------------------------------------------
class BLW_SimpleGenerator : public ObjectData
{
	INSTANCEOF(BLW_SimpleGenerator, ObjectData)
	
public:
	static NodeData* Alloc()
	{
		iferr (NodeData * nodeData = NewObj(BLW_SimpleGenerator))
		{
			err.DiagOutput();
			err.DbgStop();
			return nullptr;
		}
		
		return nodeData;
	};
	virtual Bool Init(GeListNode* node);
	virtual void GetDimension(BaseObject* op, Vector* mp, Vector* rad);
	virtual BaseObject* GetVirtualObjects(BaseObject* op, HierarchyHelp* hh);
};

#endif /* BLW_SIMPLEGENERATOR_H__ */
