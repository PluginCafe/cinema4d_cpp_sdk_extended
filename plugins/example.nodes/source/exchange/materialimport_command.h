#ifndef MATERIALIMPORT_COMMAND_H__
#define MATERIALIMPORT_COMMAND_H__

#include "c4d_commanddata.h"
#include "gui.h"
#include "maxon/material/materialimport.h"

namespace cinema
{

class BaseDocument;

} // namespace cinema

namespace maxonsdk
{

class MaterialImportCommand : public cinema::CommandData
{
public:
	virtual cinema::Bool Execute(cinema::BaseDocument* doc, cinema::GeDialog* parentManager) override;

	virtual cinema::Int32 GetState(cinema::BaseDocument* doc, cinema::GeDialog* parentManager) override;

	static maxon::Result<void> Register();

private:
	static maxon::Result<void> ImportMaterial(const maxon::Class<maxon::material::MaterialImportRef>& importerClass, cinema::BaseDocument& baseDocument);

};

} // namespace maxonsdk

#endif // MATERIALIMPORT_COMMAND_H__
