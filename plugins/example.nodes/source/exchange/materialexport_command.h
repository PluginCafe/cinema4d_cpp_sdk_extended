#ifndef MATERIALEXPORT_COMMAND_H__
#define MATERIALEXPORT_COMMAND_H__

#include "c4d_commanddata.h"
#include "gui.h"
#include "maxon/material/materialexport.h"

class BaseMaterial;

namespace maxonsdk
{

class MaterialExportCommand : public CommandData
{
public:
	virtual Bool Execute(BaseDocument* doc, GeDialog* parentManager) override;

	virtual Int32 GetState(BaseDocument* doc, GeDialog* parentManager)  override;

	static maxon::Result<void> Register();

private:
	static BaseMaterial* GetSelectedMaterial(BaseDocument* baseDocument);

	static maxon::Result<maxon::material::MaterialExchangeData> ExportMaterial(const BaseMaterial& baseMaterial);

	static void PrintMaterialData(const maxon::material::MaterialExchangeData& materialData);

};

} // namespace maxonsdk

#endif // MATERIALEXPORT_COMMAND_H__