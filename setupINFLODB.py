import os

import win32com.client as com

class DetectorInfo:
	def __init__(self):
		self.detectorId = 0
		self.stationId = 0
		self.sectionId = 0
		self.mileMarker = 0
		self.nbLanes = 0		
		
def export( subpath ):
	global db	
	global detectorsInfo
	global linksInfo
	global mileMarkerColumn
	global endMileMarkerColumn
	global toMiles	
	
	column = subpath.getModel().getColumn("GKSubpath::INFLO::IncreasingDirection")
	conn = com.Dispatch(r'ADODB.Connection')	
	DSN = ('PROVIDER = Microsoft.ACE.OLEDB.12.0;Data Source = ' + db)
	conn.Open(DSN)
	rs = com.Dispatch(r'ADODB.Recordset')
	state1 = "DELETE * FROM Configuration_TSSDetectorStation"
	state2 = "DELETE * FROM Configuration_RoadwaySubLinks"
	state3 = "DELETE * FROM Configuration_Roadway"
	state4 = "DELETE * FROM Configuration_RoadwayLinks"
	state5 = "DELETE * FROM Configuration_TSSDetectionZone"
	rs = conn.Execute(state1) #clear detector station sheet
	rs = conn.Execute(state2) #clear roadway sublinks
	rs = conn.Execute(state3) #clear roadway 
	rs = conn.Execute(state4) #clear roadway links
	rs = conn.Execute(state5) #clear detector zones
	
	mileMarker = 0
	indexSublink = 1
	indexZone = 1
	for section in subpath.getRoute():				
		section.setDataValue( mileMarkerColumn, QVariant(mileMarker) )
		sectionLength = max( round(section.length2D() * toMiles, 1), 0.1 )
		sectionLengthMM = mileMarker + sectionLength
		section.setDataValue( endMileMarkerColumn, QVariant(sectionLengthMM) )
		nbDetectors = 0
		sql = ""
		detectorStationId = ""
		if section in detectorsInfo:
			nbDetectors = len(detectorsInfo[section])
			if nbDetectors > 0:
				detectorStationId = ""
				for indexDetector in range(nbDetectors):
					separator = ""
					if indexDetector > 0:
						separator = " "
					detectorStationId = str(detectorsInfo[section][indexDetector].stationId)
					nbDetectors = 1
			sql = "INSERT INTO Configuration_RoadwayLinks (RoadwayId, LinkId, BeginMM, EndMM, NumberLanes, SpeedLimit, Direction, NumberDetectorStations, DetectorStations) VALUES (" +\
				str(subpath.getId()) + "," + str(section.getId()) + "," + str(mileMarker) + "," + str(sectionLengthMM) + "," + str(section.getDataValueIntByID(GKSection.nblanesAtt) ) + "," +\
				str(model.speedModelToGui( section.getSpeed() ) ) + ",\"" + str(subpath.getExternalId()) + "\"," + str(nbDetectors) + ",\"" + detectorStationId + "\");"	
		else:
			sql = "INSERT INTO Configuration_RoadwayLinks (RoadwayId, LinkId, BeginMM, EndMM, NumberLanes, SpeedLimit, Direction, NumberDetectorStations) VALUES (" +\
				str(subpath.getId()) + "," + str(section.getId()) + "," + str(mileMarker) + "," + str(sectionLengthMM) + "," + str(section.getDataValueIntByID(GKSection.nblanesAtt) ) + "," +\
				str(model.speedModelToGui( section.getSpeed() ) ) + ",\"" + str(subpath.getExternalId()) + "\"," + str(nbDetectors) + ");"
		print sql
		rs = conn.Execute( sql )	
		# nbSegments = section.getNbSegments()
		
		# for index in range( nbSegments ):	
			# segmentLength = section.getSegmentLength( index ) * toMiles
			# sql = "INSERT INTO Configuration_RoadwaySubLinks (RoadwayId, SubLinkId, BeginMM, EndMM, Direction, SpeedLimit, NumberLanes) VALUES (" +\
			# str(subpath.getId()) + "," + str(indexSublink) + "," + str(mileMarker) + "," + str(mileMarker+segmentLength) + ",\"" + str(subpath.getExternalId()) + "\"," +str(model.speedModelToGui( section.getSpeed() ) ) + "," + \
			# str(section.getDataValueIntByID(GKSection.nblanesAtt) ) + ");"	
			# indexSublink = indexSublink + 1	
			# print sql
			# rs = conn.Execute( sql )
			# mileMarker = mileMarker + segmentLength
		print sectionLength*10
		for index in range( int(sectionLength*10) ):
			segmentLength = 1
			sql = "INSERT INTO Configuration_RoadwaySubLinks (RoadwayId, SubLinkId, BeginMM, EndMM, Direction, SpeedLimit, NumberLanes) VALUES (" +\
			 str(subpath.getId()) + "," + str(indexSublink) + "," + str(mileMarker) + "," + str(mileMarker + 0.1) + ",\"" + str(subpath.getExternalId()) + "\"," +str(model.speedModelToGui( section.getSpeed() ) ) + "," + \
			 str(section.getDataValueIntByID(GKSection.nblanesAtt) ) + ");"
			indexSublink = indexSublink + 1
			mileMarker = mileMarker + 0.1
			rs = conn.Execute( sql )
		mileMarker = sectionLengthMM
		
		#detectors in section 	
		if section in detectorsInfo:
			print str(section.getId())
			detectionZones = ""
			stationId = ""
			detMileMarker = 0
			for detInfo in detectorsInfo[section]:	
				stationId = detInfo.stationId
				detMileMarker = detInfo.mileMarker 
				for lane in range(1, detInfo.nbLanes+1 ):
					sql = "INSERT INTO Configuration_TSSDetectionZone (DSId, DZId, Direction, LaneNumber) VALUES (" + \
					str(detInfo.detectorId) + "," + str( indexZone ) + ",\"" + subpath.getExternalId() + "\"," + str(lane) + ");"
					if detectionZones != "":
						detectionZones = detectionZones + ","
					detectionZones = detectionZones + str(indexZone)
					print sql
					indexZone = indexZone + 1
					rs = conn.Execute( sql )
			
			sql = "INSERT INTO Configuration_TSSDetectorStation (LinkId, DSId, MMLocation, NumberLanes, NumberDetectionZones, DetectionZones) VALUES (" + \
			str(section.getId()) + "," + str(stationId) + "," + str(round(section.getDataValueDouble(mileMarkerColumn) + detMileMarker, 1)) + \
			"," + str(len(detectorsInfo[section])) + "," + str(len(detectorsInfo[section])) + ",\"" + detectionZones + "\");"		
			
			print sql
			rs = conn.Execute( sql )
				
				
				
	sql = "INSERT INTO Configuration_Roadway (RoadwayId, Name, BeginMM, EndMM, Direction, MMIncreasingDirection, RecurringCongestionMMLocation) VALUES (" +\
			str(subpath.getId()) + ",\"" + str(subpath.getName()) + "\"," + str(0) + "," + str(mileMarker) + ",\"" + str( subpath.getExternalId() ) + "\",\"" + \
			str(subpath.getDataValueString(column)) + "\"," + str( 8 ) + ");"	
	print sql
	rs = conn.Execute( sql )	
	
	
	conn.close

def collectSubpathInfo( subpath ):
	global detectorsInfo
	global toMiles	
	for section in subpath.getRoute():
		if section.getTopObjects() != None:
			detectorsByPos = {}
			for topObject in section.getTopObjects():
				if topObject.isA( "GKDetector" ):
					detectorsByPos[topObject.getPosition()] = topObject
			
			for detector in detectorsByPos.itervalues():
				eid = "%s"%detector.getExternalId()
				if eid != "":
					stationId = int(eid.split("_")[0])
					if detector.getFromLane() == detector.getToLane() and stationId != 0:
						info = DetectorInfo()
						info.detectorId = detector.getId()
						info.section = detector.getSection()
						info.stationId = stationId
						info.mileMarker = detector.getPosition() * toMiles
						info.nbLanes = detector.getToLane() - detector.getFromLane() + 1
						if detector.getSection() not in detectorsInfo:
							detectorsInfo[detector.getSection()] = []
						detectorsInfo[detector.getSection()].append( info )

def initModel():
	global mileMarkerColumn
	global endMileMarkerColumn
	sectionType = model.getType("GKSection")
	mileMarkerColumn = sectionType.addColumn( "GKSection::MileMarker", "Start Mile Marker", GKColumn.Double )
	endMileMarkerColumn = sectionType.addColumn( "GKSection::EndMileMarker", "End Mile Marker", GKColumn.Double )
	
def main():
	initModel()
	collectSubpathInfo( target )
	export( target )

toMiles = 0.000621371
mileMarkerColumn = None
endMileMarkerColumn = None
detectorsInfo = {}
db = "C:/Users/paolo/Desktop/INFLO PM4/INFLODatabase.accdb"
main()
print "done"