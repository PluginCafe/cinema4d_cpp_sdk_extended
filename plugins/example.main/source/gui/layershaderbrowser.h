// LayerShaderBrowser.h
//////////////////////////////////////////////////////////////////////

#ifndef LAYERSHADERBROWSER_H__
#define LAYERSHADERBROWSER_H__

#include "c4d_gui.h"

class LayerShaderBrowser : public GeDialog
{
public:
	LayerShaderBrowser();
	virtual ~LayerShaderBrowser();

	virtual Bool CreateLayout();
	virtual Bool InitValues();
	virtual Bool Command(Int32 id, const BaseContainer& msg);
	virtual Int32 Message(const BaseContainer& msg, BaseContainer& result);
	virtual Bool CoreMessage  (Int32 id, const BaseContainer& msg);

	void UpdateAll(Bool msg);
	void ShowInfo(LayerShaderLayer* l);

	void* lastselected;

private:
	LinkBoxGui*				 linkbox;
	TreeViewCustomGui* tree;
	Int32							 lastdirty;
};

#endif // LAYERSHADERBROWSER_H__
