#ifndef MATERIALEXPORT_COMMAND_H__
#define MATERIALEXPORT_COMMAND_H__

#include "c4d_commanddata.h"
#include "gui.h"
#include "maxon/material/materialexport.h"

namespace cinema
{

class BaseMaterial;

} // namespace cinema

namespace maxonsdk
{

class MaterialExportCommand : public cinema::CommandData
{
public:
	virtual cinema::Bool Execute(cinema::BaseDocument* doc, cinema::GeDialog* parentManager) override;

	virtual cinema::Int32 GetState(cinema::BaseDocument* doc, cinema::GeDialog* parentManager)  override;

	static maxon::Result<void> Register();

private:
	static cinema::BaseMaterial* GetSelectedMaterial(cinema::BaseDocument* baseDocument);

	static maxon::Result<maxon::material::MaterialExchangeData> ExportMaterial(const cinema::BaseMaterial& baseMaterial);

	static void PrintMaterialData(const maxon::material::MaterialExchangeData& materialData);

};

} // namespace maxonsdk

#endif // MATERIALEXPORT_COMMAND_H__
