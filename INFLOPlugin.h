//-*-Mode: C++;-*-
#ifndef _INFLOPlugin_h_
#define _INFLOPlugin_h_

#include "GKPlugin.h"
#include "INFLOUtil.h"

class GKLayer;
class GKGeoObject;
class GGui;
class INFLODescription;

//---- INFLOPluginFactory -----------------------------------------------------------

extern "C" INFLOEXPORT GKPlugin * INFLOPluginFactory();

class INFLOEXPORT INFLOPlugin : public GKPlugin
{
	Q_OBJECT

public:
    INFLOPlugin();
    ~INFLOPlugin();

	QString verify();

	void initGui( GGui * gui );

	/*! Register the Aimsun Extension.
	*/
	void install();
	/*! Init the Aimsun Extension API when the user opens the first document.
	*/
	void docOpened( GGui * gui, GKModel * model );
};

#endif
