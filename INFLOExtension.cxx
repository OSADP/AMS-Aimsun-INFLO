
#include "INFLOExtension.h"
#include "INFLODescription.h"

#include "AKIAction.h"
#include "AKIDetector.h"
#include "AKIInfVeh.h"
#include "AKISimul.h"

#include "GKColumn.h"
#include "GKCommon.h"
#include "GKConstants.h"
#include "GKDBUtil.h"
#include "GKDetector.h"
#include "GKExperiment.h"
#include "GKModel.h"
#include "GKReplication.h"
#include "GKScenario.h"
#include "GKSection.h"
#include "GKSubPath.h"
#include "GKType.h"

#include "FRand.h"

#include <QDebug>
#include <QSqlError>

#include <Windows.h>

double toMiles = 0.000621371;
double kmToMile = 0.621371;
double feetToMile = 0.000189394;

INFLOExtension::INFLOExtension() : GKAimsunExtension()
{
	mLatency = 0;
	mPackageLoss = 0;
	mModel = NULL;
	mNbPackageDropped = 0;
	mApplySpeeds = true;
}

INFLOExtension::~INFLOExtension()
{

}

void INFLOExtension::readDescriptionValues()
{
	QDomNode				firstNode;
	GKPreferencesValues values;
	QDomDocument		doc( "parameters" );

	doc.setContent( parameters );
	values.setDefinition( ( ( INFLODescription* )description )->getPreferencesDefinition() );
	firstNode = doc.firstChild();
	values.restore( firstNode );

	int subpathId = values.getValue( "INFLODescription::Values::SubpathId" ).toInt();
	mModel = replication->getModel();
	mSubpath = dynamic_cast<GKSubPath*>( mModel->getCatalog().find( subpathId ) );
	INFLOSleep = values.getValue( "INFLODescription::Values::Sleep" ).toInt();
	mDbLocation = values.getValue( "INFLODescription::Values::InfloDB" );
	mSyncFile = values.getValue( "INFLODescription::Values::SynchFile" );
	mCacc1 = values.getValue("INFLODescription::Values::Cacc1").toInt();
	mCacc2 = values.getValue("INFLODescription::Values::Cacc2").toInt();

	mLatencyMin = values.getValue( "INFLODescription::Values::LatencyMin").toDouble();
	mLatencyMax = values.getValue( "INFLODescription::Values::LatencyMax").toDouble();

	if( GK::AreEqual( mLatencyMin, mLatencyMax ) == true ){
		mLatency = mLatencyMin;
	}else{
		mLatency = mRandomBall->uniform( mLatencyMin, mLatencyMax );
	}

	mPackageLossMin = values.getValue( "INFLODescription::Values::PackageLossMin").toDouble() / 100.0;
	mPackageLossMax = values.getValue( "INFLODescription::Values::PackageLossMax" ).toDouble() / 100.0;

	mApplySpeeds = values.getValue("INFLODescription::Values::NotApplyInfloSpeeds").toInt() == 0;

	if( GK::AreEqual( mPackageLossMin, mPackageLossMax ) == true ){
		mPackageLoss = mPackageLossMin;
	}else{
		mPackageLoss = mRandomBall->uniform( mPackageLossMin, mPackageLossMax );
	}
}

void INFLOExtension::init()
{
	mRandomBall = new ARandomGenerator( A2KModel::getModel().SemillaAPIExtensions );
	readDescriptionValues();
	mapSubpathMileMarkers();
	INFLOFreq = 20;

	mNextInfloStats = INFLOFreq * 1000;
	mCurrentTime = 0;
	mInterval = 0;
}

void INFLOExtension::mapSubpathMileMarkers()
{
	GKColumn * startMileMarkerColumn = mModel->getColumn( "GKSection::MileMarker");

	foreach( GKSection * section, mSubpath->getRoute() ){
		double startMileMarker = section->getDataValueDouble( startMileMarkerColumn );
		mSectionMileMarkers[section->getId()] = startMileMarker;
		int nbSegments = section->getNbSegments();
		double segmentLength = 0;
		for( int index = 0; index < nbSegments; index++ ){
			INFLOExtension::MileMarkerInfo info;
			info.section = section;
			info.segment = index+1;
			info.startMM = startMileMarker+segmentLength;
			segmentLength += section->getSegmentLength( index ) * toMiles;
			mMileMarkerInfo[startMileMarker+segmentLength] = info;
			qDebug( "%f -> section %d segment %d", startMileMarker+segmentLength, info.section->getId(), index+1 );
		}

		GKGeoObjects* topObjects = section->getTopObjects();
		if( topObjects ){
			foreach( GKGeoObject* geoobject, *topObjects ){
				GKDetector * detector = dynamic_cast<GKDetector*>( geoobject );
				if( detector && detector->getToLane() == detector->getFromLane() ){
					QString stationIdStr = detector->getExternalId().split( "_" ).first();
					bool ok;
					int stationId = stationIdStr.toInt( &ok );
					if( ok ){
						DetectorStats * stats = mDetectors.value( stationId );
						if( stats == NULL ){
							stats = new DetectorStats();
							mDetectors[stationId] = stats;
							stats->idDetector = stationId;
						}
						stats->detectors.append( detector );
					}
				}
			}
		}
	}
}

void INFLOExtension::postManage( double time )
{
	mCurrentTime += cycle * 1000;
	double simDuration = replication->getExperiment()->getScenario()->getDuration().toSeconds();
	bool lastInterval = GK::AreEqual( time, simDuration, cycle ) == true;

	manageDetectors();
	if( mCurrentTime >= mNextInfloStats && lastInterval == false ){
		mNextInfloStats += INFLOFreq * 1000;
		doTheManage( AKIGetTimeSta() );
		mInterval++;
	}
	if( mApplySpeeds ){
		manageSpeedAction( AKIGetTimeSta() );
	}
}

void INFLOExtension::manageSpeedAction( double timeSta )
{
	QMap< double, QVector<SpeedActionInfo> >::iterator iter = mDelayedActions.begin();
	GKTimeDuration time = GKTimeDuration(0,0,0).addSecs( timeSta );

	for( ; iter != mDelayedActions.end(); ){
		if( iter.key() < timeSta ){
			foreach( SpeedActionInfo actionInfo, iter.value() ){
				removeActionsFromSection(  actionInfo.idSection );
				void * action = AKIActionAddDetailedSpeedAction( actionInfo.idSection,actionInfo. laneId, actionInfo.segment, actionInfo.speed, actionInfo.vehPos, actionInfo.compliance );
				mActions[actionInfo.idSection].append( action );

				qDebug("Time: %s. Speed change on section %d (from MM %f) segment %d: new speed %f",time.toString(), actionInfo.idSection,
											  actionInfo.mileMarker , actionInfo.segment, actionInfo.speed );
			}
			iter.value().clear();
			iter = mDelayedActions.erase( iter );
		}else{
			iter++;
		}
	}
}

void INFLOExtension::manageDetectors()
{
	foreach( DetectorStats* stats, mDetectors ){
		foreach( GKDetector * detector, stats->detectors ){
			int count = AKIDetGetCounterCyclebyId( detector->getId(), 0 );
			if( count > 0 ){
				stats->counts += count;
				double speed = AKIDetGetSpeedCyclebyId( detector->getId(), 0 );
				if( speed > 0 ){
					stats->speed.addValue( speed*count );
				}
				double occupancy = AKIDetGetTimeOccupedCyclebyId( detector->getId(), 0 );
				if( occupancy >= 0 ){
					stats->occupancy += occupancy;
				}
			}
		}
	}
}

QVector<INFLOExtension::DetectorInfo> INFLOExtension::collectDetectorStats() const
{
	QVector<INFLOExtension::DetectorInfo> res;

	foreach( DetectorStats* stats, mDetectors ){
		INFLOExtension::DetectorInfo info;
		info.detectorId = stats->idDetector;
		info.realTime = mCurrentTime;
		info.volume = stats->counts;
		info.occupancy = std::min<double>( 100, (stats->occupancy/stats->detectors.size()) / INFLOFreq );
		info.speed = stats->speed.getMean( stats->counts );
		stats->counts = 0;
		stats->occupancy = 0;
		stats->speed.reset();
		res.append( info );
	}
	return res;
}

QVector<INFLOExtension::VehicleInfo> INFLOExtension::collectVehicleStats()
{
	GKColumn * endMileMarkerColumn = mModel->getColumn( "GKSection::EndMileMarker");
	QVector<INFLOExtension::VehicleInfo> res;
	int cacc1 = AKIVehGetVehTypeInternalPosition( mCacc1 );
	int cacc2 = AKIVehGetVehTypeInternalPosition( mCacc2 );
	foreach( GKSection * section, mSubpath->getRoute() ){
		int nbVehicles = AKIVehStateGetNbVehiclesSection( section->getId(), true );
		for( int vehicleIndex = 0; vehicleIndex < nbVehicles; vehicleIndex++ ){
			InfVeh infVeh = AKIVehStateGetVehicleInfSection( section->getId(), vehicleIndex );
			if( infVeh.type == cacc1 || infVeh.type == cacc2 ){
				if( packageDropped() == false ){
					INFLOExtension::VehicleInfo vehicleInfo;
					vehicleInfo.idVeh = infVeh.idVeh;
					vehicleInfo.speed = infVeh.CurrentSpeed;
					vehicleInfo.idSection = section->getId();
					vehicleInfo.position = infVeh.CurrentPos * feetToMile;// #feet to miles
					vehicleInfo.queued = infVeh.stopped;
					vehicleInfo.mileMarker = std::min<double>( vehicleInfo.position + mSectionMileMarkers[section->getId()],
							section->getDataValueDouble( endMileMarkerColumn ) );
					res.append( vehicleInfo );
				}else{
					mNbPackageDropped++;
				}
			}
		}
	}

	return res;
}

bool INFLOExtension::packageDropped() const
{
	bool res = false;

	double randomValue = mRandomBall->uniform( 0.0, 1.0 );
	res = randomValue < mPackageLoss;
	return res;
}

void INFLOExtension::writeData( const QVector<INFLOExtension::DetectorInfo> & detectorsInfo, const QVector<INFLOExtension::VehicleInfo>& vehiclesInfo )
{
	GKDataBaseInfo dbinfo;
	dbinfo.setDatabaseName( mDbLocation );
	dbinfo.setDriverName( "Access" );
	{
		QSqlDatabase db = GKAddDatabase( NULL, &dbinfo, "INFLO" );
		if( db.isValid() ){
			if( db.open() ){
				QSqlQuery query( db );
				db.transaction();
				QString cmd;
				foreach( INFLOExtension::DetectorInfo detInfo, detectorsInfo ){
					cmd = QString( "INSERT INTO TME_TSSData_Input (DSId, Volume, Occupancy, AvgSpeed) VALUES (%1,%2,%3,%4);" ).arg( detInfo.detectorId ).arg(detInfo.volume).arg(detInfo.occupancy).arg(detInfo.speed);
					if ( query.exec(cmd) == false ) {
						qDebug() << query.lastError().text();
					}
				}
				foreach( INFLOExtension::VehicleInfo vehInfo, vehiclesInfo ){
					cmd = QString( "INSERT INTO TME_CVData_Input (NomadicDeviceID, Speed, MMLocation, CVQueuedState) VALUES (%1,%2,%3,%4);" ).arg(vehInfo.idVeh).arg(vehInfo.speed).arg(vehInfo.mileMarker).arg(vehInfo.queued);
					if ( query.exec(cmd) == false ) {
						qDebug() << query.lastError().text();
					}
				}

				db.commit();
			}else{
				qDebug() << db.lastError().text();
			}
			db.close();
		}
	}
	QSqlDatabase::removeDatabase( "INFLO" );

	QFile file( mSyncFile );
	file.open( QIODevice::WriteOnly );
	file.write( "TSSData\nCVData\n" );
	file.close();
}

QVector< INFLOExtension::SpeedInfo > INFLOExtension::getSpeed() const
{
	QVector< INFLOExtension::SpeedInfo > res;
	GKDataBaseInfo dbinfo;
	dbinfo.setDatabaseName( mDbLocation );
	dbinfo.setDriverName( "access" );
	{
		QSqlDatabase db = GKAddDatabase( NULL, &dbinfo, "INFLO" );
		if( db.isValid() ){
			if( db.open() ){
				QSqlQuery query( db );
				qDebug() << "Reading DB";
				QString cmd;
				cmd = "SELECT TMEOutput_SPDHARMMessage_CV.* FROM TMEOutput_SPDHARMMessage_CV;";
				query.setForwardOnly( true );
				if( query.exec( cmd ) ){
					while( query.next() ){
						INFLOExtension::SpeedInfo info;
						info.speed = query.value( 2 ).toDouble();
						info.endMM = query.value( 4 ).toDouble();

						qDebug() << "MM " << info.endMM << " Speed " << info.speed;
						res.append( info );
					}
				}

				db.transaction();
				cmd = "DELETE * FROM TMEOutput_SPDHARMMessage_CV";
				if( query.exec(cmd) == false ){
					qDebug() << "failed to delete";
				}
				db.commit();
			}
			db.close();
		}
	}
	QSqlDatabase::removeDatabase( "INFLO" );

	return res;
}

double INFLOExtension::findSpeedToApply( const QVector< INFLOExtension::SpeedInfo >& speedData, double mileMarkerFrom, double mileMarkerTo )
{
	double res = DBL_MAX;

	QVector< INFLOExtension::SpeedInfo >::const_iterator iter, iterEnd;
	iter = speedData.begin();
	iterEnd = speedData.end();

	for( ; iter != iterEnd; ++iter ){
		double fromMM = iter->endMM - 0.1;
		if( mileMarkerTo >= fromMM && mileMarkerFrom <= iter->endMM ){
			res = std::min<double>( res, iter->speed );
		}
	}

	return res;
}

void INFLOExtension::updateSD( const QVector< INFLOExtension::SpeedInfo >& speedData, double timeSta )
{
	QMap< double, MileMarkerInfo >::const_iterator itSectionSegments, itSectionSegmentsEnd;
	itSectionSegments = mMileMarkerInfo.begin();
	itSectionSegmentsEnd = mMileMarkerInfo.end();

	for( ;itSectionSegments != itSectionSegmentsEnd; ++itSectionSegments ){
		double speed = findSpeedToApply( speedData, itSectionSegments.value().startMM, itSectionSegments.key() );
		if( speed != DBL_MAX && GK::AreEqual( speed, 0) == false ){
			int laneId = -1;
			int segment = itSectionSegments.value().segment;
			double complianceLevel = 1.0 - mPackageLoss;

			SpeedActionInfo infoCacc1;
			infoCacc1.vehPos = AKIVehGetVehTypeInternalPosition( mCacc1 );
			infoCacc1.laneId = laneId;
			infoCacc1.segment = segment;
			infoCacc1.compliance = complianceLevel;
			infoCacc1.idSection = itSectionSegments.value().section->getId();
			infoCacc1.speed = speed;
			infoCacc1.mileMarker = itSectionSegments.key();
			mDelayedActions[timeSta+mLatency].append( infoCacc1 );

			SpeedActionInfo infoCacc2;
			infoCacc2.vehPos = AKIVehGetVehTypeInternalPosition( mCacc2 );
			infoCacc2.laneId = laneId;
			infoCacc2.segment = segment;
			infoCacc2.compliance = complianceLevel;
			infoCacc2.idSection = itSectionSegments.value().section->getId();
			infoCacc2.speed = speed;
			infoCacc2.mileMarker = itSectionSegments.key();
			mDelayedActions[timeSta+mLatency].append( infoCacc2 );
		}
	}
}

void INFLOExtension::removeActionsFromSection( int sectionId )
{
	foreach( void* action, mActions.value( sectionId ) ){
		AKIActionRemoveAction( action );
	}
	mActions.remove(  sectionId );
}

INFLOExtension::MileMarkerInfo INFLOExtension::findMileMarkerInfo( double endMileMarker ) const
{
	QMap< double, MileMarkerInfo >::const_iterator iter, iterEnd;
	MileMarkerInfo res;
	res.section = NULL;
	iter = mMileMarkerInfo.begin();
	iterEnd = mMileMarkerInfo.end();
	bool found = false;
	double tolerance = 0.1;
	for( ; iter != iterEnd && found == false; ++iter ){
		double startMM = iter.value().startMM;
		double endMM = iter.key();
		if( iter.value().startMM <= endMileMarker && ( endMileMarker < iter.key() || fabs( endMileMarker - iter.key() ) < tolerance ) ){
			res.section = iter.value().section;
			res.segment = iter.value().segment;
			res.startMM = iter.value().startMM;
			found = true;
		}
	}
	return res;
}

void INFLOExtension::doTheManage( double timeSta )
{
	QVector<INFLOExtension::DetectorInfo> detectorInfo = collectDetectorStats();
	QVector<INFLOExtension::VehicleInfo> vehicleInfo = collectVehicleStats();
	//writes the data into the DB
	writeData( detectorInfo, vehicleInfo );
	//#waits for INFLO to compute the new speeds
	Sleep( INFLOSleep * 1000 );
	QVector< INFLOExtension::SpeedInfo > speedData = getSpeed();
	updateSD( speedData, timeSta );
	if( mApplySpeeds == false ){
		qDebug() << "INFLO speeds won't be applied in Aimsun Model";
	}
}



void INFLOExtension::finish()
{
	qDebug() << "Number of packet loss (vehicles not sending their position): " << mNbPackageDropped;
	qDeleteAll( mDetectors );
}
