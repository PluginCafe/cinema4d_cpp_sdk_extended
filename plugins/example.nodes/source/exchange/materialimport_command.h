#ifndef MATERIALIMPORT_COMMAND_H__
#define MATERIALIMPORT_COMMAND_H__

#include "c4d_commanddata.h"
#include "gui.h"
#include "maxon/material/materialimport.h"

class BaseDocument;
namespace maxonsdk
{

class MaterialImportCommand : public CommandData
{
public:
	virtual Bool Execute(BaseDocument* doc, GeDialog* parentManager) override;

	virtual Int32 GetState(BaseDocument* doc, GeDialog* parentManager)  override;

	static maxon::Result<void> Register();

private:
	static maxon::Result<void> ImportMaterial(const maxon::Class<maxon::material::MaterialImportRef>& importerClass, BaseDocument& baseDocument);

};

} // namespace maxonsdk

#endif // MATERIALIMPORT_COMMAND_H__