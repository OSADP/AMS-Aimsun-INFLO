
#include "INFLOPlugin.h"
#include "INFLODescription.h"
#include "INFLOExtension.h"
#include "GKSystem.h"
#include "GKModel.h"
#include "GKGeoObject.h"
#include "GKSignalRouter.h"

//---- INFLOPluginFactory -----------------------------------------------------------

GKPlugin * INFLOPluginFactory()
{
    return new INFLOPlugin();
}

//---- AimsunExtTest ---------------------------------------------------------

INFLOPlugin::INFLOPlugin() : GKPlugin()
{
    name = tr( "INFLO Extension" );
}

INFLOPlugin::~INFLOPlugin()
{
}

void INFLOPlugin::initGui(GGui * /*gui*/ )
{
}

QString INFLOPlugin::verify()
{
	QString		res;

#ifdef _ANG_DEMO
	isDemo();
#endif
	return res;
}

void INFLOPlugin::install()
{
    GKSystem::getSystem().registerAimsunExtension( new INFLODescription() );
}

void INFLOPlugin::docOpened( GGui * /*gui*/, GKModel * /*model*/ )
{	
}

