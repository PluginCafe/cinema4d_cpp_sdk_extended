// example how to use GLSL materials and callback functions to generate a cube map
// it creates an OpenGL preview image with alpha channel if enhanced OpenGL is not active

#include "c4d.h"
#include "c4d_gl.h"
#include "c4d_symbols.h"
#include "main.h"

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_FUNKY_GL_MAT 450000245

#define FUNKY_GL_MAT_CUBEMAP_INDEX 0

class FunkyGlMaterial : public MaterialData
{
	INSTANCEOF(FunkyGlMaterial, MaterialData)

public:
	static NodeData* Alloc() { return NewObjClear(FunkyGlMaterial); }

	virtual Bool Init(GeListNode* node);
	virtual void Free(GeListNode* node);
	virtual GL_MESSAGE GlMessage(BaseMaterial* mat, Int32 type, void* data);
	virtual Bool InitGLImage(BaseMaterial* mat, BaseDocument* doc, BaseThread* th, BaseBitmap* bmp, Int32 doccolorspace, Bool linearworkflow);

	static void* AllocGlDescription();
	static void FreeGlDescription(void* p);
	static Bool ReadCgDescription(GlReadDescriptionData* pFile, void* pData);
	static Bool WriteCgDescription(GlWriteDescriptionData* pFile, const void* pData);
	static void CreateTextureFunctionCallback(const Float* prIn, Float* prOut, void* pData);
	static Int32 ErrorHandler(GlProgramType type, const char* pszError);
};

struct FunkyMaterialGlDescription
{
	GlString					 strCubeMap;
	GlProgramParameter paramCubeMap;
};

void* FunkyGlMaterial::AllocGlDescription()
{
	return NewObjClear(FunkyMaterialGlDescription);
}

void FunkyGlMaterial::FreeGlDescription(void* p)
{
	FunkyMaterialGlDescription* pObj = (FunkyMaterialGlDescription*)p;
	DeleteObj(pObj);
}

Bool FunkyGlMaterial::ReadCgDescription(GlReadDescriptionData* pFile, void* pData)
{
	FunkyMaterialGlDescription* pDesc = (FunkyMaterialGlDescription*)pData;
	if (!GlProgramFactory::ReadParameter(pFile, pDesc->paramCubeMap))
		return false;
	return true;
}

Bool FunkyGlMaterial::WriteCgDescription(GlWriteDescriptionData* pFile, const void* pData)
{
	const FunkyMaterialGlDescription* pDesc = (const FunkyMaterialGlDescription*)pData;
	if (!GlProgramFactory::WriteParameter(pFile, pDesc->paramCubeMap))
		return false;
	return true;
}

void FunkyGlMaterial::CreateTextureFunctionCallback(const Float* prIn, Float* prOut, void* pData)
{
	// prIn is in [-1, 1], map it to [0, 1]
	Vector v = Vector(prIn[0], prIn[1], prIn[2]);
	v = !v;
	v = v * .5 + Vector(.5);
	prOut[0] = v.x;
	prOut[1] = v.y;
	prOut[2] = v.z;
}

Int32 FunkyGlMaterial::ErrorHandler(GlProgramType type, const char* pszError)
{
	ApplicationOutput(String(pszError));
	return 0;
}

Bool FunkyGlMaterial::Init(GeListNode* node)
{
	return true;
}

void FunkyGlMaterial::Free(GeListNode* node)
{
	GlProgramFactory::RemoveTextureReference(node, FUNKY_GL_MAT_CUBEMAP_INDEX);
}

#define FUNKY_MAT_IDENTITY_SIZE	 4
#define FUNKY_MAT_SHADER_VERSION 0

GL_MESSAGE FunkyGlMaterial::GlMessage(BaseMaterial* mat, Int32 type, void* data)
{
	switch (type)
	{
		case GL_GET_IDENTITY:
		{
			GlGetIdentity* p = (GlGetIdentity*)data;
			if (!p->pBaseDraw->GetParameterData(BASEDRAW_DATA_HQ_OPENGL).GetInt32())
			{
				p->bSetTransparency = true;
				return GL_MESSAGE::FORCE_EMULATION;
			}
			UChar* pchBuffer = (UChar*)GlProgramFactory::IncreaseBufferSize(p, FUNKY_MAT_IDENTITY_SIZE);
			if (!pchBuffer)
				return GL_MESSAGE::ERROR_;

			SetUnalignedLong(pchBuffer, FUNKY_MAT_SHADER_VERSION);
			p->ulParameters |= GL_PROGRAM_PARAM_OBJECTCOORD;
			return GL_MESSAGE::OK;
		}

		case GL_ADD_UNIFORM_PARAMETERS:
		{
			// this is the code where we add all parameters that the program takes
			GlAddUniformParametersMsg* p = (GlAddUniformParametersMsg*)data;
			p->pchIdentity += FUNKY_MAT_IDENTITY_SIZE;
			p->lObjIndex++;	// this is necessary to find the object description within the factory
			FunkyMaterialGlDescription* pCgDesc = (FunkyMaterialGlDescription*)p->pFactory->GetDescriptionData(p->lObjIndex, 0,
																							FunkyGlMaterial::AllocGlDescription, FunkyGlMaterial::FreeGlDescription,
																							FunkyGlMaterial::ReadCgDescription, FunkyGlMaterial::WriteCgDescription);
			if (!pCgDesc)
				return GL_MESSAGE::ERROR_;

			p->pFactory->AddErrorHandler(ErrorHandler);

			// add a parameter named "cubemap". To avoid collisions between different shaders a unique ID is added by the factory
			// in our case the name is changed to "cubemap3F"
			pCgDesc->strCubeMap = p->pFactory->AddUniformParameter(FragmentProgram, UniformTextureCube, "cubemap");

			return GL_MESSAGE::OK;
		}

		case GL_ADD_PROGRAM_CODE:
		{
			// this is the place where we can calculate the surface color
			GlAddProgramCodeMsg* p = (GlAddProgramCodeMsg*)data;
			p->pchIdentity += FUNKY_MAT_IDENTITY_SIZE;
			p->lObjIndex++;	// don't forget
			FunkyMaterialGlDescription* pCgDesc = (FunkyMaterialGlDescription*)p->pFactory->GetDescriptionData(p->lObjIndex, 0,
																							FunkyGlMaterial::AllocGlDescription, FunkyGlMaterial::FreeGlDescription,
																							FunkyGlMaterial::ReadCgDescription, FunkyGlMaterial::WriteCgDescription);
			if (!pCgDesc)
				return GL_MESSAGE::ERROR_;

			// use the modified name of our cubemap
			p->pFactory->AddLine(FragmentProgram, "ocolor.rgb = textureCube(" + pCgDesc->strCubeMap + ", objectcoord.xyz).rgb;");
			p->pFactory->AddLine(FragmentProgram, "ocolor.a = 1.0;");

			// just a line to test the error handler
			// p->pFactory->AddLine(FragmentProgram, "I will cause an error");

			return GL_MESSAGE::OK;
		}

		case GL_INIT_DESCRIPTION:
		{
			// here we will obtain all parameter handles
			GlInitDescriptionMsg* p = (GlInitDescriptionMsg*)data;
			p->pchIdentity += FUNKY_MAT_IDENTITY_SIZE;
			p->lObjIndex++;	// remember?
			FunkyMaterialGlDescription* pCgDesc = (FunkyMaterialGlDescription*)p->pFactory->GetDescriptionData(p->lObjIndex, 0,
																							FunkyGlMaterial::AllocGlDescription, FunkyGlMaterial::FreeGlDescription,
																							FunkyGlMaterial::ReadCgDescription, FunkyGlMaterial::WriteCgDescription);
			if (!pCgDesc)
				return GL_MESSAGE::ERROR_;

			pCgDesc->paramCubeMap = p->pFactory->GetParameterHandle(FragmentProgram, pCgDesc->strCubeMap.GetCString());

			return GL_MESSAGE::OK;
		}

		case GL_SET_UNIFORM_PARAMETERS:
		{
			GlSetUniformParametersMsg* p = (GlSetUniformParametersMsg*)data;
			p->pchIdentity += FUNKY_MAT_IDENTITY_SIZE;
			p->lObjIndex++;
			FunkyMaterialGlDescription* pCgDesc = (FunkyMaterialGlDescription*)p->pFactory->GetDescriptionData(p->lObjIndex, 0,
																							FunkyGlMaterial::AllocGlDescription, FunkyGlMaterial::FreeGlDescription,
																							FunkyGlMaterial::ReadCgDescription, FunkyGlMaterial::WriteCgDescription);
			if (!pCgDesc)
				return GL_MESSAGE::ERROR_;

			// add a texture for this material at index 0
			p->pFactory->SetParameterTextureCubeMap(pCgDesc->paramCubeMap, mat, FUNKY_GL_MAT_CUBEMAP_INDEX, 0, C4D_GL_DATATYPE_UNSIGNED_BYTE,
				CreateTextureFunctionCallback, nullptr, 0, 3, 0, true, 256, true);

			return GL_MESSAGE::OK;
		}
	}
	return GL_MESSAGE::ERROR_;
}

Bool FunkyGlMaterial::InitGLImage(BaseMaterial* mat, BaseDocument* doc, BaseThread* th, BaseBitmap* bmp, Int32 doccolorspace, Bool linearworkflow)
{
	Int32				x, y;
	Int32				w = bmp->GetBw();
	Int32				h = bmp->GetBh();
	Int32				r, g, b, a;
	BaseBitmap* alpha = bmp->GetChannelNum(0);
	if (!alpha)
	{
		alpha = bmp->AddChannel(true, true);
	}
	b = 128;

	for (y = 0; y < h; y++)
	{
		g = y * 255 / h;
		for (x = 0; x < w; x++)
		{
			r = x * 255 / w;
			bmp->SetPixel(x, y, r, g, b);
			if (alpha)
			{
				a = x + y;
				if ((a & 96) == 0)
					a = 255;
				else if ((a & 96) == 64)
					a = 160;
				else if ((a & 96) == 32)
					a = 80;
				else
					a = 0;
				bmp->SetAlphaPixel(alpha, x, y, a);
			}
		}
	}

	return true;
}

Bool RegisterGLTestMaterial()
{
	String name = GeGetDefaultFilename(DEFAULTFILENAME_SHADER_VOLUME) + GeLoadString(IDS_FUNKY_GL_MATERIAL);	// place in default Shader section

	return RegisterMaterialPlugin(ID_FUNKY_GL_MAT, name, PLUGINFLAG_MATERIAL_GLIMAGE_WITH_ALPHA, FunkyGlMaterial::Alloc, String(), 0);
}
