#include "c4d_basedraw.h"
#include "c4d_videopost.h"
#include "c4d_gl.h"
#include "c4d_resource.h"
#include "c4d_symbols.h"
#include "c4d_basebitmap.h"
#include "vpstereo.h"
#include "c4d_includes.h"
#include "main.h"

#define SDKSTEREO_VIDEOPOST_ID 450000253

Vector g_vUV[4] = { Vector(0.0, 0.0, 0.0), Vector(1.0, 0.0, 0.0), Vector(1.0, 1.0, 0.0), Vector(0.0, 1.0, 0.0) };

class StereoVideoPost : public VideoPostData
{
public:
	static NodeData* Alloc() { return NewObjClear(StereoVideoPost); }

	virtual Bool Init(GeListNode* node);
	virtual void Free(GeListNode* node);

	virtual Bool GlDraw(BaseVideoPost* node, BaseDraw* bd, GlFrameBuffer* fbuf, Int32 colortex, Int32 depthtex, VIDEOPOST_GLDRAW flags);
	virtual Int32 StereoGetCameraCountEditor(BaseVideoPost* node, BaseDocument* doc, BaseDraw* bd);
	virtual Int32 StereoGetCameraCountRenderer(BaseVideoPost* node, BaseDocument* doc, RenderData* rd);
	virtual Bool StereoGetCameraInfo(BaseVideoPost* node, BaseDocument* doc, BaseDraw* bd, RenderData* rd, Int32 index, StereoCameraInfo& info);
	virtual VIDEOPOST_GLINFO GetGlInfo(BaseVideoPost* node, BaseDocument* doc, BaseDraw* bd);
	virtual Bool StereoMergeImages(BaseVideoPost* node, BaseBitmap* pDest, const BaseBitmap* const* ppSource, Int32 lCount, const BaseContainer& bcSettings, COLORSPACETRANSFORMATION transform);
	virtual Bool RenderEngineCheck(BaseVideoPost* node, Int32 id);

protected:
	static void* AllocCgDescription();
	static void FreeCgDescription(void* pData);
	static Bool ReadCgDescription(GlReadDescriptionData* pFile, void* pData);
	static Bool WriteCgDescription(GlWriteDescriptionData* pFile, const void* pData);

	CameraObject* m_pCamera;
	Int32					m_lCameraCount;
};

Bool RegisterStereoVideoPost()
{
	return RegisterVideoPostPlugin(SDKSTEREO_VIDEOPOST_ID, GeLoadString(IDS_VPSTEREO), PLUGINFLAG_VIDEOPOST_GL | PLUGINFLAG_VIDEOPOST_STEREO_EDITOR | PLUGINFLAG_VIDEOPOST_STEREO_RENDERING, StereoVideoPost::Alloc, "vpstereo"_s, 0, 0);
}

/************************************************************************/
/* StereoVideoPost                                                    */
/************************************************************************/
#define MAX_TEXURES	3
struct VPStereoDescData
{
	GlString															 strTexsize, strTextures[MAX_TEXURES];
	GlProgramParameter										 paramTexsize, paramTextures[MAX_TEXURES];
	Int32																	 lVectorCount;
	const GlVertexBufferVectorInfo* const* ppVectorInfo;
};

void* StereoVideoPost::AllocCgDescription()
{
	return NewObjClear(VPStereoDescData);
}

void StereoVideoPost::FreeCgDescription(void* pData)
{
	VPStereoDescData* pDescData = (VPStereoDescData*)pData;
	DeleteObj(pDescData);
}

Bool StereoVideoPost::ReadCgDescription(GlReadDescriptionData* pFile, void* pData)
{
	VPStereoDescData* pDescData = (VPStereoDescData*)pData;
	if (!GlProgramFactory::ReadParameter(pFile, pDescData->paramTexsize))
		return false;
	if (!GlProgramFactory::ReadParameter(pFile, pDescData->paramTextures[0]))
		return false;
	if (!GlProgramFactory::ReadParameter(pFile, pDescData->paramTextures[1]))
		return false;
	if (!GlProgramFactory::ReadParameter(pFile, pDescData->paramTextures[2]))
		return false;
	return true;
}

Bool StereoVideoPost::WriteCgDescription(GlWriteDescriptionData* pFile, const void* pData)
{
	const VPStereoDescData* pDescData = (const VPStereoDescData*)pData;
	if (!GlProgramFactory::WriteParameter(pFile, pDescData->paramTexsize))
		return false;
	if (!GlProgramFactory::WriteParameter(pFile, pDescData->paramTextures[0]))
		return false;
	if (!GlProgramFactory::WriteParameter(pFile, pDescData->paramTextures[1]))
		return false;
	if (!GlProgramFactory::WriteParameter(pFile, pDescData->paramTextures[2]))
		return false;
	return true;
}

Bool StereoVideoPost::Init(GeListNode* node)
{
	BaseContainer* pbcData = ((BaseVideoPost*)node)->GetDataInstance();
	pbcData->SetInt32(VP_STEREO_CAMERAS, VP_STEREO_CAMERAS_2);
	return true;
}

void StereoVideoPost::Free(GeListNode* node)
{
	GlProgramFactory::RemoveReference(node);
}

Bool StereoVideoPost::RenderEngineCheck(BaseVideoPost* node, Int32 id)
{
	// the following render engines are not supported by this effect
	if (id == RDATA_RENDERENGINE_CINEMAN)
		return false;

	return true;
}

Int32 StereoVideoPost::StereoGetCameraCountEditor(BaseVideoPost* node, BaseDocument* doc, BaseDraw* bd)
{
	return m_lCameraCount;
}

Int32 StereoVideoPost::StereoGetCameraCountRenderer(BaseVideoPost* node, BaseDocument* doc, RenderData* rd)
{
	BaseDraw*			 pRenderBaseDraw = doc->GetRenderBaseDraw();
	BaseObject*		 pCamera = pRenderBaseDraw->GetSceneCamera(doc);
	BaseContainer* pbcData = node->GetDataInstance();
	Int32 lCameraCount = 0;
	if (!pCamera)
		pCamera = pRenderBaseDraw->GetEditorCamera();
	BaseContainer* pbcCamera = pCamera->GetDataInstance();
	if (!pbcData->GetBool(VP_STEREO_ENABLE))
		return 0;
	if (pbcCamera->GetInt32(CAMERAOBJECT_STEREO_MODE) != CAMERAOBJECT_STEREO_MODE_MONO)
	{
		lCameraCount = pbcData->GetInt32(VP_STEREO_CAMERAS, 2);
	}
	return lCameraCount;
}

Bool StereoVideoPost::StereoGetCameraInfo(BaseVideoPost* node, BaseDocument* doc, BaseDraw* bd, RenderData* rd, Int32 index, StereoCameraInfo& info)
{
	BaseObject* pCamera;
	if (rd)
	{
		BaseDraw* pBaseDraw = doc->GetRenderBaseDraw();
		pCamera = pBaseDraw->GetSceneCamera(doc);
		if (!pCamera)
			pCamera = pBaseDraw->GetEditorCamera();
	}
	else
	{
		pCamera = m_pCamera;
	}

	if (!pCamera)
	{
		DebugAssert(false);
		return false;
	}

	Matrix				 mg = pCamera->GetMg();
	BaseContainer* pbcData = node->GetDataInstance();
	Int32 lCameraCount = pbcData->GetInt32(VP_STEREO_CAMERAS);
	Float rSeparation	 = pbcData->GetFloat(VP_STEREO_SEPARATION);

	info.off_x = pbcData->GetFloat(CAMERAOBJECT_FILM_OFFSET_X);
	info.off_y = pbcData->GetFloat(CAMERAOBJECT_FILM_OFFSET_Y);
	info.m = mg;

	Float f = Float(index) / Float(lCameraCount - 1);
	f = (f - .5) * 2.0;
	info.m.off += Vector(f * rSeparation * info.m.sqmat.v1);

	return true;
}

Bool StereoVideoPost::StereoMergeImages(BaseVideoPost* node, BaseBitmap* pDest, const BaseBitmap* const* ppSource, Int32 lCount, const BaseContainer& bcSettings, COLORSPACETRANSFORMATION transform)
{
	BaseContainer* pbcData = node->GetDataInstance();
	Int32 lCameraCount = pbcData->GetInt32(VP_STEREO_CAMERAS);

	if (lCameraCount != lCount || lCount < 2 || lCount > 3)
	{
		DebugAssert(false);
		return false;
	}

	Float32* prLines, *prLine1, *prLine2, *prLine3;
	Int32		 lWidth	 = ppSource[0]->GetBw();
	Int32		 lHeight = ppSource[0]->GetBh();
	Int32		 lLineSize = lWidth * 3;
	Int32		 y, x;

	if (pDest->GetBw() != lWidth || pDest->GetBh() != lHeight)
		return false;
	if (ppSource[1]->GetBw() != lWidth || ppSource[1]->GetBh() != lHeight)
		return false;
	if (lCount == 3)
	{
		if (ppSource[2]->GetBw() != lWidth || ppSource[2]->GetBh() != lHeight)
			return false;
	}

	iferr (prLines = NewMem(Float32, 3 * lLineSize))
		return false;

	prLine1 = prLines;
	prLine2 = prLines + lLineSize;
	prLine3 = prLines + 2 * lLineSize;
	if (lCount == 2)
		ClearMemType(prLine3, lLineSize);

	for (y = 0; y < lHeight; y++)
	{
		ppSource[0]->GetPixelCnt(0, y, lWidth, (UChar*)prLine1, 3 * sizeof(Float32), COLORMODE::RGBf, PIXELCNT::NONE);
		ppSource[1]->GetPixelCnt(0, y, lWidth, (UChar*)prLine2, 3 * sizeof(Float32), COLORMODE::RGBf, PIXELCNT::NONE);
		if (lCount == 3)
			ppSource[2]->GetPixelCnt(0, y, lWidth, (UChar*)prLine3, 3 * sizeof(Float32), COLORMODE::RGBf, PIXELCNT::NONE);
		for (x = 0; x < lWidth; x++)
		{
			prLine1[3 * x + 1] = prLine2[3 * x + 1];
			prLine1[3 * x + 2] = prLine3[3 * x + 2];
		}
		pDest->SetPixelCnt(0, y, lWidth, (UChar*)prLine1, 3 * sizeof(Float32), COLORMODE::RGBf, PIXELCNT::NONE);
	}

	DeleteMem(prLines);
	return true;
}

VIDEOPOST_GLINFO StereoVideoPost::GetGlInfo(BaseVideoPost* node, BaseDocument* doc, BaseDraw* bd)
{
	BaseContainer* pbcCamera, *pbcData;
	pbcData = node->GetDataInstance();
	m_pCamera = nullptr;
	BaseObject* pCamera = bd->GetSceneCamera(doc);
	m_lCameraCount = 0;
	if (!pCamera)
		pCamera = bd->GetEditorCamera();
	if (!pCamera || !pCamera->IsInstanceOf(Ocamera))
		return VIDEOPOST_GLINFO::DISABLED;
	if (!pbcData->GetBool(VP_STEREO_ENABLE))
		return VIDEOPOST_GLINFO::DISABLED;
	m_pCamera = (CameraObject*)pCamera;
	pbcCamera = pCamera->GetDataInstance();
	if (pbcCamera->GetInt32(CAMERAOBJECT_STEREO_MODE) != CAMERAOBJECT_STEREO_MODE_MONO)
	{
		m_lCameraCount = pbcData->GetInt32(VP_STEREO_CAMERAS, 2);
	}
	if (m_lCameraCount < 2 || m_lCameraCount > 3)
	{
		m_pCamera = nullptr;
		m_lCameraCount = 0;
		return VIDEOPOST_GLINFO::DISABLED;
	}
	return VIDEOPOST_GLINFO::DRAW;
}

#define VP_STEREO_SHADER_VERSION 0

Bool StereoVideoPost::GlDraw(BaseVideoPost* node, BaseDraw* bd, GlFrameBuffer* fbuf, Int32 colortex, Int32 depthtex, VIDEOPOST_GLDRAW flags)
{
	if (!m_pCamera)
		return false;

	if (flags & VIDEOPOST_GLDRAW::DRAW)
	{
		BaseContainer* pbcData = node->GetDataInstance();
		Int32 lCameraCount = LCut(pbcData->GetInt32(VP_STEREO_CAMERAS), 2, 3);
		Int32 x1, y1, x2, y2;
		bd->GetFrame(&x1, &y1, &x2, &y2);
		x2 -= x1;
		y2 -= y1;
		x2++;
		y2++;
		Float prScale[2];
		VPStereoDescData* pDescData = nullptr;
		Bool			bFactoryBound = false;
		C4DGLuint nTexture1, nTexture2, nTexture3;
		Int32			lAttributeCount, lVectorBufferCount;
		const GlVertexBufferAttributeInfo* const* ppAttibuteInfo;
		const GlVertexBufferVectorInfo* const*		ppVectorInfo;
		Matrix32 m1, m2;
		Vector32 c1, c2;
		C4D_ALIGN(Int32 lIdentity[2], 8);
		UINT nWidth, nHeight;

		lIdentity[0] = VP_STEREO_SHADER_VERSION;
		lIdentity[1] = lCameraCount;

		// get the scale ratios that we don't put the texture on the entire polygon
		fbuf->GetRatios(C4D_FRAMEBUFFER_COLOR, prScale[0], prScale[1]);
		if (prScale[0] <= 0.0f || prScale[1] <= 0.0f)
			return false;
		prScale[0] = 1.0f / prScale[0];
		prScale[1] = 1.0f / prScale[1];
		fbuf->GetSize(C4D_FRAMEBUFFER_COLOR, nWidth, nHeight, true);

		if (!bd->GetFullscreenPolygonVectors(lAttributeCount, ppAttibuteInfo, lVectorBufferCount, ppVectorInfo))
			return false;

		bd->SetDrawParam(DRAW_PARAMETER_USE_Z, false);

		GlProgramFactory* pFactory = GlProgramFactory::GetFactory(bd, (C4DAtom*)node, 0, nullptr, lIdentity, sizeof(lIdentity), nullptr, 0, GL_GET_PROGRAM_FACTORY_ALLOW_SHARING, ppAttibuteInfo, lAttributeCount, ppVectorInfo, lVectorBufferCount, nullptr);
		if (!pFactory)
			return false;

		pFactory->LockFactory();
		if (!pFactory->BindToView(bd))
		{
			pFactory->UnlockFactory();
			goto displayError;
		}

		pDescData = (VPStereoDescData*)pFactory->GetDescriptionData(0, 0, StereoVideoPost::AllocCgDescription, StereoVideoPost::FreeCgDescription, StereoVideoPost::ReadCgDescription, StereoVideoPost::WriteCgDescription);
		if (!pDescData)
		{
			pFactory->UnlockFactory();
			goto displayError;
		}

		if (!pFactory->IsProgram(CompiledProgram))
		{
			// add all necessary parameters
			pFactory->AddParameters(GL_PROGRAM_PARAM_UV | GL_PROGRAM_PARAM_UVW4_FP);
			pFactory->Init(0);
			pDescData->strTexsize = pFactory->AddUniformParameter(VertexProgram, UniformFloat4, "texsize");
			pDescData->strTextures[0] = pFactory->AddUniformParameter(FragmentProgram, UniformTexture2D, "texture1");
			pDescData->strTextures[1] = pFactory->AddUniformParameter(FragmentProgram, UniformTexture2D, "texture2");
			if (lCameraCount == 3)
				pDescData->strTextures[2] = pFactory->AddUniformParameter(FragmentProgram, UniformTexture2D, "texture3");
			if (!pFactory->HeaderFinished())
				goto displayError;

			// now, add the program source code
			pFactory->AddLine(VertexProgram, "oposition = vec4(iposition.xy, -1.0, 1.0);");
			pFactory->AddLine(VertexProgram, "ouvw = vec4(.5 * (iposition.xy + vec2(1.0)), 0.0, 0.0);");
			pFactory->AddLine(VertexProgram, "ouvw.zw = ouvw.xy * " + pDescData->strTexsize + ".zw;");
			pFactory->AddLine(VertexProgram, "ouvw.xy *= " + pDescData->strTexsize + ".xy;");
			pFactory->AddLine(FragmentProgram, "ocolor.r=texture2D(" + pDescData->strTextures[0] + ", uvw.xy).r;");
			pFactory->AddLine(FragmentProgram, "ocolor.g=texture2D(" + pDescData->strTextures[1] + ", uvw.xy).g;");
			if (lCameraCount == 3)
				pFactory->AddLine(FragmentProgram, "ocolor.b=texture2D(" + pDescData->strTextures[2] + ", uvw.xy).b;");
			else
				pFactory->AddLine(FragmentProgram, "ocolor.b=0.0;");
			pFactory->AddLine(FragmentProgram, "ocolor.a=1.0;");

			if (!pFactory->CompilePrograms())
			{
				pFactory->DestroyPrograms();
				goto displayError;
			}

			pDescData->paramTexsize = pFactory->GetParameterHandle(VertexProgram, pDescData->strTexsize.GetCString());
			pDescData->paramTextures[0] = pFactory->GetParameterHandle(FragmentProgram, pDescData->strTextures[0].GetCString());
			pDescData->paramTextures[1] = pFactory->GetParameterHandle(FragmentProgram, pDescData->strTextures[1].GetCString());
			pDescData->paramTextures[2] = pFactory->GetParameterHandle(FragmentProgram, pDescData->strTextures[2].GetCString());
			pFactory->GetVectorInfo(pDescData->lVectorCount, pDescData->ppVectorInfo);
		}
		if (!pFactory->BindPrograms())
		{
			pFactory->UnbindPrograms();
			goto displayError;
		}
		bFactoryBound = true;
		pFactory->UnlockFactory();

		// set the program parameters
		pFactory->InitSetParameters();
		pFactory->SetParameterReal4(pDescData->paramTexsize, (Float32)prScale[0], (Float32)prScale[1], (Float32)x2, (Float32)y2);
		nTexture1 = fbuf->GetTexture(colortex, C4D_FRAMEBUFFER_COLOR);
		nTexture2 = fbuf->GetTexture(colortex + 1, C4D_FRAMEBUFFER_COLOR);
		nTexture3 = lCameraCount == 3 ? fbuf->GetTexture(colortex + 2, C4D_FRAMEBUFFER_COLOR) : 0;
		pFactory->SetParameterTexture(pDescData->paramTextures[0], 2, nTexture1);
		pFactory->SetParameterTexture(pDescData->paramTextures[1], 2, nTexture2);
		if (lCameraCount == 3)
			pFactory->SetParameterTexture(pDescData->paramTextures[2], 2, nTexture3);

		bd->DrawFullscreenPolygon(pDescData->lVectorCount, pDescData->ppVectorInfo);

		pFactory->UnbindPrograms();
		return true;

	displayError:
		if (pFactory && bFactoryBound)
		{
			pFactory->UnbindPrograms();
		}
		pFactory->UnlockFactory();
		return false;
	}
	return true;
}
