//-*-Mode: C++;-*-
#ifndef _INFLOExtension_h_
#define _INFLOExtension_h_

#include "GKAimsunExtension.h"

#include "INFLOUtil.h"
#include "SSNetwork.h"
#include <QMap>
#include <QVector>

//---- INFLOExtension -----------------------------------------------------------

class GKDetector;
class GKModel;
class GKSection;
class GKSubPath;
class ARandomGenerator;


class INFLOEXPORT INFLOExtension : public GKAimsunExtension
{
public:
	INFLOExtension();
	~INFLOExtension();

	void init();
	void finish();
	void postManage(double time);

	struct MileMarkerInfo{
		GKSection * section;
		int segment;
		double startMM;
	};

	struct DetectorInfo{
		int detectorId;
		double realTime;
		double volume;
		double occupancy;
		double speed;
	};

	struct VehicleInfo{
		int idVeh;
		double speed;
		int idSection;
		double position;
		bool queued;
		double mileMarker;
	};
	struct SpeedActionInfo{
		int vehPos;
		int idSection;
		int laneId;
		int segment;
		double speed;
		double position;
		bool queued;
		double mileMarker;
		double compliance;
	};
	struct SpeedInfo{
		double speed;
		double endMM;
	};

	class DetectorStats{
	public:
		DetectorStats(){
			speed.reset();
			occupancy = 0;
			counts = 0;
		}

		SSStatisticOutput speed;
		double occupancy;
		int counts;
		int idDetector;
		QVector< GKDetector* > detectors;
	};

private:

	int             mInterval;
	int             INFLOFreq;
	int             INFLOSleep;
	int			 mCacc1;
	int             mCacc2;
	double       mNextInfloStats;
	double       mCurrentTime;
	double       mLatency, mLatencyMin, mLatencyMax;
	double       mPackageLoss, mPackageLossMin, mPackageLossMax;
	QString      mDbLocation;
	QString      mSyncFile;
	int             mNbPackageDropped;
	bool            mApplySpeeds;

	ARandomGenerator *mRandomBall;

	GKModel 		*mModel;
	GKSubPath       *mSubpath;

	QMap< int, double >                 mSectionMileMarkers;
	QMap< double, MileMarkerInfo >      mMileMarkerInfo;

	QMap< int, DetectorStats* >         mDetectors;
	QMap< int, QVector< void* > >      mActions;

	QMap< double, QVector<SpeedActionInfo> > mDelayedActions; //the key is in timeSta

	void readDescriptionValues();
	void mapSubpathMileMarkers();
	void doTheManage(double timeSta);
	void manageDetectors();
	void manageSpeedAction( double timeSta );
	QVector<DetectorInfo> collectDetectorStats() const;
	QVector<VehicleInfo> collectVehicleStats();
	QVector< SpeedInfo > getSpeed() const;
	MileMarkerInfo findMileMarkerInfo( double endMileMarker ) const;
	void writeData( const QVector<DetectorInfo> & detectorsInfo, const QVector<VehicleInfo>& vehiclesInfo );
	void updateSD(const QVector< SpeedInfo >& speedData, double timeSta);
	double findSpeedToApply( const QVector< INFLOExtension::SpeedInfo >& speedData, double mileMarkerFrom, double mileMarkerTo );
	bool packageDropped() const;
	double latencyApplied() const;
	void removeActionsFromSection( int sectionId );
};


#endif
