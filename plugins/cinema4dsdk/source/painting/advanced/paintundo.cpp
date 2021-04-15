#include "paintundo.h"
#include "paintchannels.h"
#include "registeradvancedpaint.h"

#define SCENEHOOK_VERSION 1
#define ID_PAINT_UNDOREDO_SCENEHOOK 1031368

//=============================================================================================================

#define SCULPTPAINTUNDOSTRING "SculptPaintUndo"

static Bool CreateSculptUndo(BaseDocument *pDoc)
{
	if (pDoc)
	{
		BaseSceneHook *pSceneHook = pDoc->FindSceneHook(ID_PAINT_UNDOREDO_SCENEHOOK);
		if (pSceneHook)
		{
			BaseContainer *bc = pSceneHook->GetDataInstance();
			bc->SetString((Int32)UNDOTYPE::PRIVATE_STRING, String(SCULPTPAINTUNDOSTRING));
			pDoc->StartUndo();
			pDoc->AddUndo(UNDOTYPE::PRIVATE_STRING, pSceneHook);
			pDoc->EndUndo();
			return true;
		}
	}
	return false;
}

PaintUndoTile::PaintUndoTile()
: m_pData(nullptr)
, m_dataSize(0)
, m_x(0)
, m_y(0)
, m_xTile(0)
, m_yTile(0)
{
}

PaintUndoTile::~PaintUndoTile()
{
	if (m_pData)
	{
		DeleteMem(m_pData);
	}
}

Bool PaintUndoTile::Init(PaintLayerBmp *pBitmap, Int x, Int y)
{
	m_pDestBitmap->SetLink(pBitmap);
	if (!pBitmap)
	{
		return false;
	}

	m_xTile = (Int)(x * Int(PAINTTILEINV));
	m_yTile = (Int)(y * Int(PAINTTILEINV));

	m_x = m_xTile * PAINTTILESIZE;
	m_y = m_yTile * PAINTTILESIZE;

	int bitdepth, numChannels;
	if (!GetChannelInfo(pBitmap, bitdepth, numChannels))
		return false;

	if (m_pData)
	{
		DeleteMem(m_pData);
		m_dataSize = 0;
	}

	COLORMODE colorMode = (COLORMODE)pBitmap->GetColorMode();
	Int32 bitsPerPixel = (bitdepth / 8) * numChannels;
	m_dataSize = PAINTTILESIZE * PAINTTILESIZE * bitsPerPixel;
	iferr (m_pData = NewMem(UChar, m_dataSize))
		return false;

	// Copy the data across
	UChar *pos = m_pData;
	for (Int32 yy = 0; yy < PAINTTILESIZE; yy++)
	{
		pBitmap->GetPixelCnt((Int32)m_x, (Int32)m_y + yy, (Int32)PAINTTILESIZE, &pos[yy*PAINTTILESIZE*bitsPerPixel], colorMode, PIXELCNT::NONE);
	}

	return true;
}

PaintLayerBmp *PaintUndoTile::GetBitmap()
{
	return  (PaintLayerBmp*)m_pDestBitmap->ForceGetLink();
}

void PaintUndoTile::Apply()
{
	PaintLayerBmp *pBitmap = GetBitmap();
	if (!pBitmap)
	{
		return;
	}

	int bitdepth, numChannels;
	if (!GetChannelInfo(pBitmap, bitdepth, numChannels))
		return;

	COLORMODE colorMode = (COLORMODE)pBitmap->GetColorMode();
	Int32 bitsPerPixel = (bitdepth / 8) * numChannels;

	// Copy the data across
	UChar *pos = m_pData;
	for (Int32 y = 0; y < PAINTTILESIZE; y++)
	{
		pBitmap->SetPixelCnt((Int32)m_x, (Int32)m_y + y, PAINTTILESIZE, &pos[y*PAINTTILESIZE*bitsPerPixel], bitsPerPixel, colorMode , PIXELCNT::NONE);
	}

	pBitmap->UpdateRefresh((Int32)m_x, (Int32)m_y, (Int32)m_x+PAINTTILESIZE, (Int32)m_y+PAINTTILESIZE, UPDATE_STD);
}

PaintUndoTile *PaintUndoTile::GetCurrentStateClone()
{
	iferr (PaintUndoTile *pTile = NewObj(PaintUndoTile))
		return nullptr;
	pTile->Init(GetBitmap(), m_x, m_y);
	return pTile;
}

//=============================================================================================================

PaintUndoStroke::PaintUndoStroke()
{
}

PaintUndoStroke::~PaintUndoStroke()
{
}

void PaintUndoStroke::Init()
{
}

void PaintUndoStroke::Init(PaintUndoStroke *pStroke)
{
	for (auto& a : pStroke->m_Tiles)
	{
		AddUndoTile(a.GetCurrentStateClone());
	}
}

inline Int GetTileHash(Int x, Int y)
{
	Int xx = (Int)(x * Int(PAINTTILEINV));
	Int yy = (Int)(y * Int(PAINTTILEINV));
	return xx + PAINTTILESIZE * yy;
}

void PaintUndoStroke::AddUndoTile(PaintUndoTile *pTile)
{
  iferr_scope_handler
  {
    return;
  };

	if (pTile)
	{
		m_Tiles.AppendPtr(pTile) iferr_return;
		m_tileMap.Insert(GetTileHash(pTile->m_x, pTile->m_y), pTile) iferr_return;
	}
}

void PaintUndoStroke::Apply()
{
	for (auto& a : m_Tiles)
	{
		a.Apply();
	}
	DrawViews(DRAWFLAGS::PRIVATE_NO_WAIT_GL_FINISHED|DRAWFLAGS::ONLY_ACTIVE_VIEW|DRAWFLAGS::NO_THREAD|DRAWFLAGS::NO_ANIMATION);
}

PaintUndoTile* PaintUndoStroke::Find(Int x, Int y)
{
	TileMap::Entry* entry = m_tileMap.Find(GetTileHash(x, y));
	return entry ? const_cast<PaintUndoTile*>(entry->GetValue()) : nullptr;
}


//=============================================================================================================

PaintUndoRedo::PaintUndoRedo()
: m_pCurrentStroke(nullptr)
{
}

PaintUndoRedo::~PaintUndoRedo()
{
	ClearRedos();
	ClearUndos();
}

void PaintUndoRedo::ClearRedos()
{
	for (auto& stroke : m_RedoStrokes)
	{
		DeleteObj(stroke);
	}
	m_RedoStrokes.Reset();
}

void PaintUndoRedo::ClearUndos()
{
	for (auto& stroke : m_UndoStrokes)
	{
		DeleteObj(stroke);
	}
	m_UndoStrokes.Reset();
}

void PaintUndoRedo::Undo()
{
	if (m_UndoStrokes.GetCount() > 0)
	{
		PaintUndoStroke *pLastStroke = nullptr;
		if (m_UndoStrokes.Pop(&pLastStroke))
		{
			iferr (PaintUndoStroke *pRedoStroke = NewObj(PaintUndoStroke))
				return;
			pRedoStroke->Init(pLastStroke);
			iferr (m_RedoStrokes.Append(pRedoStroke))
        return;
			pLastStroke->Apply();
			DeleteObj(pLastStroke);
			pLastStroke = nullptr;
		}
	}
}

void PaintUndoRedo::Redo()
{
	if (m_RedoStrokes.GetCount() > 0)
	{
		PaintUndoStroke *pLastStroke = nullptr;
		if (m_RedoStrokes.Pop(&pLastStroke))
		{
			iferr (PaintUndoStroke *pUndoStroke = NewObj(PaintUndoStroke))
				return;
			pUndoStroke->Init(pLastStroke);
			iferr (m_UndoStrokes.Append(pUndoStroke))
        return;
			pLastStroke->Apply();
			DeleteObj(pLastStroke);
			pLastStroke = nullptr;
		}
	}
}

void PaintUndoRedo::FlushUndoBuffer()
{
	ClearRedos();
	ClearUndos();
}


void PaintUndoRedo::StartUndoStroke()
{
	ClearRedos();
	if (!m_pCurrentStroke)
	{
		m_pCurrentStroke = NewObjClear(PaintUndoStroke);
		if (!m_pCurrentStroke)
		{
			CriticalStop();
		}
	}
}

void PaintUndoRedo::EndUndoStroke()
{
	if (m_pCurrentStroke)
	{
		iferr (m_UndoStrokes.Append(m_pCurrentStroke))
      return;
		m_pCurrentStroke = nullptr;
	}
}


Bool PaintUndoRedo::AddUndoTile(PaintLayerBmp *pBitmap, Int x, Int y)
{
	if (m_pCurrentStroke)
	{
		if (m_pCurrentStroke->Find(x, y))
			return false;

		PaintUndoTile *pUndoTile = NewObjClear(PaintUndoTile);
		if (!pUndoTile)
		{
			return false;
		}
		if (!pUndoTile->Init(pBitmap, x, y))
		{
			DeleteObj(pUndoTile);
			return false;
		}

		m_pCurrentStroke->AddUndoTile(pUndoTile);
		return true;
	}
	return false;
}

//=======================================================
// Sculpt Undo/Redo Scene Hook
//=======================================================
PaintUndoSystem::PaintUndoSystem()
: m_pUndoRedo(nullptr)
, undoEvent(true)
{
}

PaintUndoSystem::~PaintUndoSystem()
{
	if (m_pUndoRedo)
	{
		DeleteObj(m_pUndoRedo);
	}
}

NodeData *PaintUndoSystem::Alloc()
{
	return NewObjClear(PaintUndoSystem);
}


Bool PaintUndoSystem::Init(GeListNode* node)
{
	m_pUndoRedo = NewObjClear(PaintUndoRedo);
	if (!m_pUndoRedo) 
		return false;
	return true;
}

Bool PaintUndoSystem::Message(GeListNode *node, Int32 type, void *data)
{
	switch (type)
	{
		case MSG_STRINGUNDO:
		{
			StringUndo *su = (StringUndo*)data;
			if (su && su->str == SCULPTPAINTUNDOSTRING)
			{
				if (su->redo) 
					Redo();
				else 
					Undo();
			}
			break;
		}
		default:
			break;
	}

	return SceneHookData::Message(node, type, data);
}

Bool PaintUndoSystem::AddUndoRedo(PaintLayerBmp *pBitmap, Int x, Int y)
{
	return m_pUndoRedo->AddUndoTile(pBitmap, x, y);
}

Bool PaintUndoSystem::Undo()
{
	if (m_pUndoRedo && m_lock.AttemptLock())
	{
		m_pUndoRedo->Undo();
		m_lock.Unlock();
	}
	return true;
}

Bool PaintUndoSystem::Redo()
{
	if (m_pUndoRedo && m_lock.AttemptLock())
	{
		m_pUndoRedo->Redo();
		m_lock.Unlock();
	}
	return true;
}

Bool PaintUndoSystem::StartStroke()
{
	if (m_pUndoRedo)
	{
		m_pUndoRedo->StartUndoStroke();
		return true;
	}
	return false;
}

Bool PaintUndoSystem::EndStroke()
{
	if (m_pUndoRedo)
	{
		m_pUndoRedo->EndUndoStroke();
		CreateSculptUndo(GetActiveDocument());
		return true;
	}
	return false;
}

PaintUndoStroke *PaintUndoSystem::GetCurrentStroke()
{
	if (m_pUndoRedo)
	{
		return m_pUndoRedo->GetCurrentStroke();
	}
	return nullptr;
}

Bool PaintUndoSystem::FlushUndoBuffer()
{
	if (m_pUndoRedo)
	{
		m_pUndoRedo->FlushUndoBuffer();
		return true;
	}
	return false;
}

Bool RegisterPaintUndoSystem()
{
	return RegisterSceneHookPlugin(ID_PAINT_UNDOREDO_SCENEHOOK, "Paint Brush"_s, PLUGINFLAG_HIDE|PLUGINFLAG_SCENEHOOK_SUPPORT_ANIMATION, PaintUndoSystem::Alloc, EXECUTIONPRIORITY_GENERATOR, SCENEHOOK_VERSION);
}

Bool FreePaintUndoSystem()
{
	return true;
}





PaintUndoSystem *GetPaintUndoSystem(BaseDocument *doc)
{
	if (!doc)  
	{ 
		return nullptr; 
	}

	PaintUndoSystem *pUndoRedo = nullptr;
	BaseSceneHook *pUndoRedoHook = doc->FindSceneHook(ID_PAINT_UNDOREDO_SCENEHOOK);
	if (pUndoRedoHook)
	{
		pUndoRedo = pUndoRedoHook->GetNodeData<PaintUndoSystem>();
	}
	return pUndoRedo;
}
