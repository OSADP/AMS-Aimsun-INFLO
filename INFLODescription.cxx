#include <qobject.h>

#include "GKPreferencesValues.h"
#include "GKPreferencesAttribute.h"
#include "GKPreferencesEditor.h"
#include "GKPreferencesGroup.h"
#include "GKPreferences.h"


#include "INFLOExtension.h"
#include "INFLODescription.h"

//---- AimsunExtADescription ------------------------------------------------

INFLODescription::INFLODescription() : GKSimulatorExtensionDescription()
{
    name = QObject::tr("INFLO");
	version = "8.0.0";
    internalName = "INFLOExt";

	GKPreferencesGroup		*maingroup;
    GKPreferencesGroup		*tcaGroup;
    GKPreferencesAttribute	*attr;

	maingroup = new GKPreferencesGroup();
    maingroup->setName( "INFLODescription::Option" );
	maingroup->setExternalName( QObject::tr("Options") );	
	prefs.addGroup( maingroup );
	
    attr = new GKPreferencesAttribute();
    attr->setName( "INFLODescription::Values::SubpathId" );
    attr->setExternalName( QObject::tr("Subpath Id:") );
    attr->setType( GKPreferencesAttribute::eInt );
    attr->setDefault( "0" );
    maingroup->addAttribute( attr );

	attr = new GKPreferencesAttribute();
	attr->setName("INFLODescription::Values::Cacc1");
	attr->setExternalName(QObject::tr("Cacc1:"));
	attr->setType(GKPreferencesAttribute::eInt);
	attr->setDefault("0");
	maingroup->addAttribute(attr);

	attr = new GKPreferencesAttribute();
	attr->setName("INFLODescription::Values::Cacc2");
	attr->setExternalName(QObject::tr("Cacc2:"));
	attr->setType(GKPreferencesAttribute::eInt);
	attr->setDefault("0");
	maingroup->addAttribute(attr);

    attr = new GKPreferencesAttribute();
    attr->setName( "INFLODescription::Values::Sleep" );
    attr->setExternalName( QObject::tr("INFLO Sleep Time (s):") );
    attr->setType( GKPreferencesAttribute::eInt );
    attr->setDefault( "5" );
    maingroup->addAttribute( attr );

    attr = new GKPreferencesAttribute();
    attr->setName( "INFLODescription::Values::SynchFile" );
    attr->setExternalName( QObject::tr("Synchronization File:") );
    attr->setType( GKPreferencesAttribute::eFileOut );
    attr->setDefault( "C:/tmp/Data.txt" );
    maingroup->addAttribute( attr );

    attr = new GKPreferencesAttribute();
    attr->setName( "INFLODescription::Values::InfloDB" );
    attr->setExternalName( QObject::tr("INFLO DB location:") );
    attr->setType( GKPreferencesAttribute::eFileIn );
    attr->setDefault( "C:/tmp/INFLODatabase.accdb" );
    maingroup->addAttribute( attr );

    attr = new GKPreferencesAttribute();
    attr->setName( "INFLODescription::Values::NotApplyInfloSpeeds" );
    attr->setExternalName( QObject::tr("Do Not Apply INFLO Speeds") );
    attr->setType( GKPreferencesAttribute::eBool );
    attr->setDefault( "0" );
    maingroup->addAttribute( attr );

    tcaGroup = new GKPreferencesGroup();
    tcaGroup->setName( "INFLODescription::TCA::Option" );
    tcaGroup->setExternalName( QObject::tr("TCA Options") );
    tcaGroup->setEditorPlace( GKPreferencesGroup::eBox );
    maingroup->addGroup( tcaGroup );

    attr = new GKPreferencesAttribute();
    attr->setName( "INFLODescription::Values::LatencyMin" );
    attr->setExternalName( QObject::tr("Latency Min (s):") );
    attr->setType( GKPreferencesAttribute::eDouble );
    attr->setDefault( "0" );
    tcaGroup->addAttribute( attr );

    attr = new GKPreferencesAttribute();
    attr->setName( "INFLODescription::Values::LatencyMax" );
    attr->setExternalName( QObject::tr("Latency Max (s):") );
    attr->setType( GKPreferencesAttribute::eDouble );
    attr->setDefault( "3" );
    tcaGroup->addAttribute( attr );

    attr = new GKPreferencesAttribute();
    attr->setName( "INFLODescription::Values::PackageLossMin" );
    attr->setExternalName( QObject::tr("Package Loss Min (%):") );
    attr->setType( GKPreferencesAttribute::eDouble );
    attr->setDefault( "0" );
    tcaGroup->addAttribute( attr );

    attr = new GKPreferencesAttribute();
    attr->setName( "INFLODescription::Values::PackageLossMax" );
    attr->setExternalName( QObject::tr("Package Loss Max (%):") );
    attr->setType( GKPreferencesAttribute::eDouble );
    attr->setDefault( "20" );
    tcaGroup->addAttribute( attr );

}

INFLODescription::~INFLODescription()
{
}

const GKPreferences * INFLODescription::getPreferencesDefinition() const
{
	return &prefs;
}

QString INFLODescription::editParameters( GGui * gui, const QString & parameters )
{
	GKPreferencesValues		values;
	GKPreferencesEditor		editor( gui );
	QString					res;
	QDomDocument			doc( "parameters" );
	QDomNode				node;
	
	doc.setContent( parameters );
	node = doc.firstChild();
	values.restore( node );
	values.setDefinition( &prefs );	
	editor.hidePreferencesList();
	editor.editThis( &prefs, &values );
    editor.setWindowTitle( QObject::tr("INFLO") );
	if( editor.exec() == QDialog::Accepted ){
		values.store( res );
	}else{
		res = parameters;
	}
	return res;
}

GKAimsunExtension * INFLODescription::createExtension()
{
    return new INFLOExtension();
}
