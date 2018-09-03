// In this example we show how to draw certain objects with OpenGL and enhanced OpenGL.

#include "c4d.h"
#include "ogltest.h"
#include "c4d_symbols.h"
#include "c4d_gl.h"
#include "main.h"

class GLTestObject : public ObjectData
{
public:
	static NodeData* Alloc() { return NewObjClear(GLTestObject); }

protected:
	virtual Bool Init(GeListNode* node);
	virtual void Free(GeListNode* node);
	virtual DRAWRESULT Draw(BaseObject* op, DRAWPASS type, BaseDraw* bd, BaseDrawHelp* bh);

	Bool DrawParticles(BaseObject* pObject, BaseDraw* bd, BaseDrawHelp* bh, Int32 lType);

	static Vector32*			 g_pvParticlePoint, *g_pvParticleColor, *g_pvParticleNormal;
	GlVertexSubBufferData* m_pSubBuffer;

public:
	static void FreeParticleData();
};

Vector32* GLTestObject::g_pvParticlePoint	 = nullptr;
Vector32* GLTestObject::g_pvParticleColor	 = nullptr;
Vector32* GLTestObject::g_pvParticleNormal = nullptr;

static maxon::BaseArray<GlVertexBufferVectorInfo>*		 g_pVectorInfo;
static maxon::BaseArray<GlVertexBufferAttributeInfo>* g_pAttributeInfo;

GlVertexBufferVectorInfo*		 g_ppVectorInfo[3];
GlVertexBufferAttributeInfo* g_ppAttributeInfo[3];

void GLTestObject::FreeParticleData()
{
	DeleteMem(g_pvParticlePoint);
	DeleteMem(g_pvParticleColor);
	DeleteMem(g_pvParticleNormal);
}

Bool GLTestObject::Init(GeListNode* node)
{
	BaseObject*		 pObject = (BaseObject*)node;
	BaseContainer* pbcData = pObject->GetDataInstance();

	pbcData->SetInt32(GLTEST_TYPE, GLTEST_TYPE_PARTICLE_FAST_BUFFER);

	return true;
}

void GLTestObject::Free(GeListNode* node)
{
	GlVertexBuffer::RemoveObject((C4DAtom*)node, 0);
	GlProgramFactory::RemoveReference((C4DAtom*)node);
}

DRAWRESULT GLTestObject::Draw(BaseObject* op, DRAWPASS type, BaseDraw* bd, BaseDrawHelp* bh)
{
	if (type != DRAWPASS::OBJECT)
		return DRAWRESULT::SKIP;

	BaseContainer* pbcData = op->GetDataInstance();
	Int32 lType = pbcData->GetInt32(GLTEST_TYPE);
	return DrawParticles(op, bd, bh, lType) ? DRAWRESULT::OK : DRAWRESULT::SKIP;
}

static Float32 value(Float32 nl, Float32 n2, Float32 hue)
{
	if (hue > 360.0f)
		hue -= 360.0f;
	else if (hue < 0.0f)
		hue += 360.0f;
	if (hue < 60.0f)
		return nl + (n2 - nl) * hue / 60.0f;
	if (hue < 180.0f)
		return n2;
	if (hue < 240.0f)
		return nl + (n2 - nl) * (240.0f - hue) / 60.0f;
	return nl;
}

static void HLStoRGB(Float32 h, Float32 l, Float32 s, Float32* r, Float32* g, Float32* b)
{
	Float32 m1, m2;
	if (l <= 0.5f)
		m2 = l * (1.0f + s);
	else
		m2 = l + s - l * s;
	m1 = 2.0f * l - m2;
	if (s == 0 || h == -1)
	{
		*r = *g = *b = l;
	}
	else
	{
		*r = value(m1, m2, h + 120.0f);
		*g = value(m1, m2, h);
		*b = value(m1, m2, h - 120.0f);
	}
}
Bool GLTestObject::DrawParticles(BaseObject* pObject, BaseDraw* bd, BaseDrawHelp* bh, Int32 lType)
{
	const Int32		lPoints = 1000000;
	Int32					l;
	Vector32			v(DC);
	Float32				r, rRad, x, z, n;
	Matrix				mg;
	BaseDocument* pDoc = bh->GetDocument();
	GlVertexBufferDrawSubbuffer di(C4D_VERTEX_BUFFER_POINTS, lPoints, 0);
	Int32 lVectorCount;
	const GlVertexBufferVectorInfo* const* ppVectorInfo;
	const GlLight**		ppLights = nullptr;
	GlProgramFactory* pFactory = nullptr;
	Int32 lLightCount = 0;

	Int32 lDrawportType = bd->GetDrawParam(BASEDRAW_DRAWPORTTYPE).GetInt32();
	if (lDrawportType != DRAWPORT_TYPE_OGL_HQ)
		return true;

	if (!g_pvParticlePoint || !g_pvParticleColor || !g_pvParticleNormal)
	{
		DeleteMem(g_pvParticlePoint);
		DeleteMem(g_pvParticleColor);
		DeleteMem(g_pvParticleNormal);

		// prepare the point array
		iferr (g_pvParticlePoint	 = NewMem(Vector32, lPoints))
			return false;
		iferr (g_pvParticleColor	 = NewMem(Vector32, lPoints))
			return false;
		iferr (g_pvParticleNormal = NewMem(Vector32, lPoints))
			return false;

		// make a disc with radius from 0 to 100 and let the radius grow each time
		v.z = 0.0f;
		for (l = 0; l < lPoints; l++)
		{
			// point
			r = (Float32)l / (Float32)lPoints;
			rRad = r * 100.0f;
			v.x	 = Cos(r * (Float32)PI2 * 300.0f) * rRad;
			v.y	 = Sin(r * (Float32)PI2 * 300.0f) * rRad;
			g_pvParticlePoint[l] = v;

			// color
			HLStoRGB(Modulo(r * 300.0f * 360.0f, 360.0f), 1.0f - .5f * r, 1.0f, &g_pvParticleColor[l].x, &g_pvParticleColor[l].y, &g_pvParticleColor[l].z);

			// normal
			n = v.x / 50.0f;
			n = Cos(n * (Float32)PI2) * DegToRad(20.0f);
			z = Cos(n);
			x = Sin(n);

			g_pvParticleNormal[l] = Vector32(x, 0.0, z);
		}
	}

	mg = pObject->GetMg() * MatrixRotZ(pDoc->GetTime().Get() * PI2);

	if (lType == GLTEST_TYPE_PARTICLE_FAST_BLOCK || lType == GLTEST_TYPE_PARTICLE_FAST_BUFFER)
	{
		Bool bEOGL = false;
		if (lDrawportType == DRAWPORT_TYPE_OGL_HQ && lType == GLTEST_TYPE_PARTICLE_FAST_BUFFER)
		{
			GlVertexBuffer* pBuffer = GlVertexBuffer::GetVertexBuffer(bd, pObject, 0, nullptr, 0, 0);
			if (!pBuffer)
				goto _no_eogl;
			if (pBuffer->IsDirty())
			{
				// let's allocate a buffer that holds our data
				m_pSubBuffer = pBuffer->AllocSubBuffer(bd, VBArrayBuffer, lPoints * (sizeof(Vector32) + sizeof(Vector32) + sizeof(Vector32)), nullptr);
				if (!m_pSubBuffer)
					goto _no_eogl;
				// map the buffer so that we can store our data
				// note that this memory is located on the graphics card
				void* pData = pBuffer->MapBuffer(bd, m_pSubBuffer, VBWriteOnly);
				if (!pData)
				{
					pBuffer->UnmapBuffer(bd, m_pSubBuffer);
					goto _no_eogl;
				}
				Vector32* pvData = (Vector32*)pData;
				for (l = 0; l < lPoints; l++)
				{
					*pvData++ = g_pvParticlePoint[l];
					*pvData++ = g_pvParticleNormal[l];
					*pvData++ = g_pvParticleColor[l];
				}
				pBuffer->UnmapBuffer(bd, m_pSubBuffer);
				pBuffer->SetDirty(false);
			}

			lLightCount = bd->GetGlLightCount();
			if (lLightCount > 0)
			{
				iferr (ppLights = NewMemClear(const GlLight*, lLightCount))
					goto _no_eogl;
				for (l = 0; l < lLightCount; l++)
					ppLights[l] = bd->GetGlLight(l);
			}
			else
			{
				ppLights = 0;
			}

			// we need a program to show the data
			pFactory = GlProgramFactory::GetFactory(bd, pObject, 0, nullptr, nullptr, 0, ppLights, lLightCount, 0, g_ppAttributeInfo, 3, g_ppVectorInfo, 3, nullptr);
			if (!pFactory)
				goto _no_eogl;
			pFactory->LockFactory();
			pFactory->BindToView(bd);
			if (!pFactory->IsProgram(CompiledProgram))
			{
				// just route the vertex information to the fragment program
				UInt64 ulParameters = GL_PROGRAM_PARAM_NORMALS | GL_PROGRAM_PARAM_COLOR;
				if (lLightCount > 0)
					ulParameters |= GL_PROGRAM_PARAM_EYEPOSITION;
				pFactory->AddParameters(ulParameters);
				pFactory->Init(0);
				pFactory->HeaderFinished();
				pFactory->AddLine(VertexProgram, "oposition = (transmatrix * vec4(iposition.xyz, 1.0));");
				pFactory->AddLine(VertexProgram, "ocolor = vec4(icolor.rgb, 1.0);");
				pFactory->AddLine(VertexProgram, "onormal = vec4(inormal.xyz, 1.0);");
				if (lLightCount)
				{
					pFactory->AddLine(VertexProgram, "worldcoord.xyz = objectmatrix_r * iposition.xyz + objectmatrix_p.xyz;");
					pFactory->AddLine(FragmentProgram, "vec3 V = normalize(eyeposition - worldcoord.xyz);");
					pFactory->AddLine(FragmentProgram, "ocolor = vec4(0.0);");
					pFactory->StartLightLoop();
					pFactory->AddLine(FragmentProgram, "ocolor.rgb += icolor.rgb * lightcolorD.rgb * abs(dot(normal.xyz, L.xyz));");
					pFactory->EndLightLoop();
					pFactory->AddLine(FragmentProgram, "ocolor.a = 1.0;");
				}
				else
				{
					// use the fragment color if no light source is used
					pFactory->AddLine(FragmentProgram, "ocolor = icolor;");
				}
				pFactory->CompilePrograms();
				if (lLightCount > 0)
					pFactory->InitLightParameters();
			}

			bd->SetMatrix_Matrix(nullptr, Matrix());
			pFactory->BindPrograms();
			pFactory->SetParameterMatrixTransform(pFactory->GetParameterHandle(VertexProgram, "transmatrix"));
			if (lLightCount > 0)
			{
				pFactory->SetLightParameters(ppLights, lLightCount, Matrix4d32());
				pFactory->SetParameterVector(pFactory->GetParameterHandle(FragmentProgram, "eyeposition"), (Vector32)bd->GetMg().off);
				pFactory->SetParameterMatrix(pFactory->GetParameterHandle(FragmentProgram, "objectmatrix_p"), pFactory->GetParameterHandle(FragmentProgram, "objectmatrix_r"), -1, Matrix32());
			}
			// obtain the vector information from the factory so that we have the proper attribute locations
			pFactory->GetVectorInfo(lVectorCount, ppVectorInfo);
			pBuffer->DrawSubBuffer(bd, m_pSubBuffer, nullptr, 1, &di, lVectorCount, ppVectorInfo);
			pFactory->UnbindPrograms();

			pFactory->BindToView(nullptr);
			pFactory->UnlockFactory();

			bEOGL = true;
		_no_eogl:
			;
		}
		if (!bEOGL)
		{
			// this will copy all data into a buffer and draw them immediately
			bd->DrawPointArray(lPoints, g_pvParticlePoint, &(g_pvParticleColor->x), 3, g_pvParticleNormal);
		}
	}
	else
	{
		// this will copy each point into an internal buffer and draw the buffer when a certain limit has been reached
		bd->SetTransparency(0);
		for (l = 0; l < lPoints; l++)
		{
			bd->SetPen((Vector)g_pvParticleColor[l]);
			//bd->Point3D((Vector)g_pvParticlePoint[l]);
		}
	}

	return true;
}

Bool RegisterGLTestObject()
{
	String name = GeLoadString(IDS_GL_TEST_OBJECT);
	if (!name.IsPopulated())
		return true;

	iferr (g_pVectorInfo = NewObj(maxon::BaseArray<GlVertexBufferVectorInfo>))
		return false;
	iferr (g_pAttributeInfo = NewObj(maxon::BaseArray<GlVertexBufferAttributeInfo>))
		return false;
	iferr (g_pVectorInfo->Resize(3))
		return false;
	iferr (g_pAttributeInfo->Resize(3))
		return false;


	maxon::BaseArray<GlVertexBufferVectorInfo>&		vertex = *g_pVectorInfo;
	maxon::BaseArray<GlVertexBufferAttributeInfo>& attrib = *g_pAttributeInfo;

	for (Int32 a = 0; a < 3; a++)
	{
		g_ppVectorInfo[a] = &(vertex[a]);
		g_ppAttributeInfo[a] = &(attrib[a]);
	}

	Int lStride = 3 * sizeof(Vector32);
	vertex[0].lDataType = C4D_GL_DATATYPE_FLOAT;
	vertex[0].lCount	= 3;
	vertex[0].strType = "vec3";
	vertex[0].strName = "attrib0";
	vertex[0].lOffset = 0;
	vertex[0].lStride = lStride;
	vertex[0].nAttribLocation = C4D_VERTEX_BUFFER_VERTEX;

	vertex[1].lDataType = C4D_GL_DATATYPE_FLOAT;
	vertex[1].lCount	= 3;
	vertex[1].strType = "vec3";
	vertex[1].strName = "attrib1";
	vertex[1].lOffset = sizeof(Vector32);
	vertex[1].lStride = lStride;
	vertex[1].nAttribLocation = C4D_VERTEX_BUFFER_NORMAL;

	vertex[2].lDataType = C4D_GL_DATATYPE_FLOAT;
	vertex[2].lCount	= 3;
	vertex[2].strType = "vec3";
	vertex[2].strName = "attrib2";
	vertex[2].lOffset = 2 * sizeof(Vector32);
	vertex[2].lStride = lStride;
	vertex[2].nAttribLocation = C4D_VERTEX_BUFFER_COLOR;

	attrib[0].lVector[0] = attrib[0].lVector[1] = attrib[0].lVector[2] = 0;
	attrib[0].lIndex[0]	 = 0; attrib[0].lIndex[1] = 1; attrib[0].lIndex[2] = 2;
	attrib[0].strDeclaration = "vec3 iposition = attrib0.xyz";
	attrib[0].strName = "iposition";

	attrib[1].lVector[0] = attrib[1].lVector[1] = attrib[1].lVector[2] = 0;
	attrib[1].lIndex[0]	 = 0; attrib[1].lIndex[1] = 1; attrib[1].lIndex[2] = 2;
	attrib[1].strDeclaration = "vec3 inormal = attrib1.xyz";
	attrib[1].strName = "inormal";

	attrib[2].lVector[0] = attrib[2].lVector[1] = attrib[2].lVector[2] = 0;
	attrib[2].lIndex[0]	 = 0; attrib[2].lIndex[1] = 1; attrib[2].lIndex[2] = 2;
	attrib[2].strDeclaration = "vec3 icolor = attrib2.xyz";
	attrib[2].strName = "icolor";

	return RegisterObjectPlugin(450000131, name, 0, GLTestObject::Alloc, "ogltest"_s, nullptr, 0);
}

void FreeGLTestObject()
{
	DeleteObj(g_pVectorInfo);
	DeleteObj(g_pAttributeInfo);

	GLTestObject::FreeParticleData();
}
