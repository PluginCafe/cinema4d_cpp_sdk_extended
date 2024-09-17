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

	cinema::Bool Init(cinema::PaintLayerBmp *pBitmap, cinema::Int x, cinema::Int y);
	cinema::PaintLayerBmp *GetBitmap();
	void Apply();

	PaintUndoTile *GetCurrentStateClone();

	cinema::AutoAlloc<cinema::BaseLink> m_pDestBitmap;

	cinema::UChar *m_pData;
	cinema::Int m_dataSize;
	cinema::Int m_x;
	cinema::Int m_y;
	cinema::Int m_xTile;
	cinema::Int m_yTile;
};

class PaintUndoStroke
{
public:
	PaintUndoStroke();
	~PaintUndoStroke();

	void Init();
	void Init(PaintUndoStroke *pStroke);
	PaintUndoTile* Find(cinema::Int x, cinema::Int y);

	void AddUndoTile(PaintUndoTile *pTile);
	void Apply();

	cinema::Int GetTileCount() { return m_Tiles.GetCount(); }
	PaintUndoTile* GetUndoData(cinema::Int32 a) { return &m_Tiles[a]; }

public:
	maxon::PointerArray<PaintUndoTile> m_Tiles;

	typedef maxon::HashMap<cinema::Int, PaintUndoTile*> TileMap;
	TileMap m_tileMap;
};

class PaintUndoRedo
{
public:
	PaintUndoRedo();
	~PaintUndoRedo();

	void StartUndoStroke();
	void EndUndoStroke();

	cinema::Bool AddUndoTile(cinema::PaintLayerBmp *pBitmap, cinema::Int x, cinema::Int y);
	cinema::Bool ApplyStroke(PaintUndoStroke *pStroke);

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

class PaintUndoSystem : public cinema::SceneHookData
{
	INSTANCEOF(PaintUndoSystem , cinema::SceneHookData)

private:
	PaintUndoSystem();
	~PaintUndoSystem();

public:
	virtual cinema::Bool Init(cinema::GeListNode* node, cinema::Bool isCloneInit);
	virtual cinema::Bool Message(cinema::GeListNode *node, cinema::Int32 type, void *data);

public:
	static cinema::NodeData *Alloc();

	cinema::Bool AddUndoRedo(cinema::PaintLayerBmp *pBitmap, cinema::Int x, cinema::Int y);
	cinema::Bool Undo();
	cinema::Bool Redo();

	cinema::Bool FlushUndoBuffer();

	cinema::Bool StartStroke();
	cinema::Bool EndStroke();

	PaintUndoStroke *GetCurrentStroke();

private:
	PaintUndoRedo *m_pUndoRedo;
	cinema::Bool undoEvent;

	maxon::Spinlock m_lock; // Avoid multiple calls to Undo or Redo
};

PaintUndoSystem *GetPaintUndoSystem(cinema::BaseDocument *doc);

#endif // PAINTUNDO_H__
