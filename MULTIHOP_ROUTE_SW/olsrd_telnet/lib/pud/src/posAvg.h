#ifndef _PUD_POSAVG_H_
#define _PUD_POSAVG_H_

/* Plugin includes */

/* OLSR includes */

/* System includes */
#include <nmea/info.h>
#include <stdbool.h>

/** Stores angle components */
typedef struct _AngleComponents {
		double x; /**< cos of the angle (in radians) */
		double y; /**< sin of the angle (in radians) */
} AngleComponents;

/** Stores an nmeaINFO entry, used in the averaging */
typedef struct _PositionUpdateEntry {
		nmeaINFO nmeaInfo; /**< the position information */

		/* used for averaging of angles */
		AngleComponents track; /**< the track angle components */
		AngleComponents mtrack; /**< the mtrack angle components */
		AngleComponents magvar; /**< the magvar angle components */
} PositionUpdateEntry;

/**
 Counts the number of GPxxx based entries. Some parameters in nmeaINFO are
 dependent on different GPxxx NMEA sentences. These counters are used to
 determine which information would be valid for the average position.
 Also counts the fix values.
 */
typedef struct _PositionUpdateCounters {
		/* present */
		unsigned long long smask; /**< the number of entries with SMASK present */
		unsigned long long utcdate; /**< the number of entries with UTCDATE present */
		unsigned long long utctime; /**< the number of entries with UTCTIME present */
		unsigned long long sig; /**< the number of entries with SIG present */
		unsigned long long fix; /**< the number of entries with FIX present */
		unsigned long long pdop; /**< the number of entries with PDOP present */
		unsigned long long hdop; /**< the number of entries with HDOP present */
		unsigned long long vdop; /**< the number of entries with VDOP present */
		unsigned long long lat; /**< the number of entries with LAT present */
		unsigned long long lon; /**< the number of entries with LON present */
		unsigned long long elv; /**< the number of entries with ELV present */
		unsigned long long speed; /**< the number of entries with SPEED present */
		unsigned long long track; /**< the number of entries with TRACK present */
		unsigned long long mtrack; /**< the number of entries with MTRACK present */
		unsigned long long magvar; /**< the number of entries with MAGVAR present */
		unsigned long long satinusecount; /**< the number of entries with SATINUSECOUNT present */
		unsigned long long satinuse; /**< the number of entries with SATINUSE present */
		unsigned long long satinview; /**< the number of entries with SATINVIEW present */

		/* smask */
		unsigned long long gpgga; /**< the number of GPGGA based entries */
		unsigned long long gpgsa; /**< the number of GPGSA based entries */
		unsigned long long gpgsv; /**< the number of GPGSV based entries */
		unsigned long long gprmc; /**< the number of GPRMC based entries */
		unsigned long long gpvtg; /**< the number of GPVTG based entries */

		/* sig */
		unsigned long long sigBad; /**< the number of entries with a bad sig */
		unsigned long long sigLow; /**< the number of entries with a low sig */
		unsigned long long sigMid; /**< the number of entries with a mid sig */
		unsigned long long sigHigh; /**< the number of entries with a high sig */

		/* fix */
		unsigned long long fixBad; /**< the number of entries with a bad fix */
		unsigned long long fix2d; /**< the number of entries with a 2D fix */
		unsigned long long fix3d; /**< the number of entries with a 3D fix */
} PositionUpdateCounters;

/**
 A list of position updates that are used to determine the average position.

 The list uses 1 extra entry: when 5 entries have to be averaged then the list
 will have 6 entries. The 6th entry is used for the incoming entry, so that
 it is already in the list (together with the old entry that will be removed
 from the average) and does not need to be copied into the list. This is for
 better performance.

 The list is a circular list.
 This means that there is a gap/unused entry in the list between the
 newest entry and the oldest entry, which is the 'incoming entry'.

 Note that 'positionAverageCumulative' stores cumulative values for parameters
 for which an average is calculated. The reason is to minimise the number of
 calculations to be performed.
 */
typedef struct _PositionAverageList {
		unsigned long long entriesMaxCount; /**< the maximum number of entries in the list */
		PositionUpdateEntry * entries; /**< the list entries */

		unsigned long long entriesCount; /**< the number of entries in the list */
		unsigned long long newestEntryIndex; /**< index of the newest entry in the list (zero-based) */
		PositionUpdateCounters counters; /**< the counters */

		PositionUpdateEntry positionAverageCumulative; /**< the average position with cumulative values */
		PositionUpdateEntry positionAverage; /**< the average position */
} PositionAverageList;

/**
 Enumeration describing the type of an entry position in the average list
 */
typedef enum _AverageEntryPositionType {
	OLDEST, NEWEST, INCOMING, AVERAGECUMULATIVE, AVERAGE
} AverageEntryPositionType;

bool initPositionAverageList(PositionAverageList * positionAverageList,
		unsigned long long maxEntries);
void flushPositionAverageList(PositionAverageList * positionAverageList);
void destroyPositionAverageList(PositionAverageList * positionAverageList);

PositionUpdateEntry * getPositionAverageEntry(
		PositionAverageList * positionAverageList,
		AverageEntryPositionType positionType);

void addNewPositionToAverage(PositionAverageList * positionAverageList,
		PositionUpdateEntry * newEntry);

#endif /* _PUD_POSAVG_H_ */
