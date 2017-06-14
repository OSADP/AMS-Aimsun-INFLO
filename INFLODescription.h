//-*-Mode: C++;-*-
#ifndef _INFLODescription_h_
#define _INFLODescription_h_

#include "INFLOUtil.h"
#include "GKPreferences.h"

#include "GKAimsunExtension.h"

//---- INFLODescription -----------------------------------------------------------

class INFLOEXPORT INFLODescription : public GKSimulatorExtensionDescription
{
public:
    INFLODescription();
    ~INFLODescription();

	GKAimsunExtension * createExtension();
	QString editParameters( GGui * gui, const QString & parameters );

	const GKPreferences * getPreferencesDefinition() const;
private:
	GKPreferences		prefs;
};

#endif
