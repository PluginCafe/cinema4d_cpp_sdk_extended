#ifndef PAINTUNDO_H__
#define PAINTUNDO_H__

#include "c4d.h"
#include "maxon/pointerarray.h"
#include "maxon/hashmap.h"

#define PAINTTILESIZE 64
#define PAINTTILEINV 1.0/64.0

class PaintUndoTile
{
public:
	PaintUndoTile();
	virtual ~PaintUndoTile();

	Bool Init(PaintLayerBmp *pBitmap, Int x, Int y);
	PaintLayerBmp *GetBitmap();
	void Apply();

	PaintUndoTile *GetCurrentStateClone();

	AutoAlloc<BaseLink> m_pDestBitmap;

	UChar *m_pData;
	Int m_dataSize;
	Int m_x;
	Int m_y;
	Int m_xTile;
	Int m_yTile;
};

class PaintUndoStroke
{
public:
	PaintUndoStroke();
	~PaintUndoStroke();

	void Init();
	void Init(PaintUndoStroke *pStroke);
	PaintUndoTile* Find(Int x, Int y);

	void AddUndoTile(PaintUndoTile *pTile);
	void Apply();

	Int GetTileCount() { return m_Tiles.GetCount(); }
	PaintUndoTile* GetUndoData(Int32 a) { return &m_Tiles[a]; }

public:
	maxon::PointerArray<PaintUndoTile> m_Tiles;

	typedef maxon::HashMap<Int, PaintUndoTile*> TileMap;
	TileMap m_tileMap;
};

class PaintUndoRedo
{
public:
	PaintUndoRedo();
	~PaintUndoRedo();

	void StartUndoStroke();
	void EndUndoStroke();

	Bool AddUndoTile(PaintLayerBmp *pBitmap, Int x, Int y);
	Bool ApplyStroke(PaintUndoStroke *pStroke);

	void Undo();
	void Redo();

	void ClearUndos();
	void ClearRedos();

	void FlushUndoBuffer();

	PaintUndoStroke *GetCurrentStroke() { return m_pCurrentStroke; }

private:
	maxon::BaseArray<PaintUndoStroke*> m_UndoStrokes;
	maxon::BaseArray<PaintUndoStroke*> m_RedoStrokes;

	PaintUndoStroke *m_pCurrentStroke;
};

class PaintUndoSystem : public SceneHookData
{
	INSTANCEOF(PaintUndoSystem , SceneHookData)

private:
	PaintUndoSystem();
	~PaintUndoSystem();

public:
	virtual Bool Init(GeListNode* node, Bool isCloneInit);
	virtual Bool Message(GeListNode *node, Int32 type, void *data);

public:
	static NodeData *Alloc();

	Bool AddUndoRedo(PaintLayerBmp *pBitmap, Int x, Int y);
	Bool Undo();
	Bool Redo();

	Bool FlushUndoBuffer();

	Bool StartStroke();
	Bool EndStroke();

	PaintUndoStroke *GetCurrentStroke();

private:
	PaintUndoRedo *m_pUndoRedo;
	Bool undoEvent;

	maxon::Spinlock m_lock; // Avoid multiple calls to Undo or Redo
};

PaintUndoSystem *GetPaintUndoSystem(BaseDocument *doc);

#endif // PAINTUNDO_H__
