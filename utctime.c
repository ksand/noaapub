/* utctime.c */

#include <stdio.h>
#include <time.h>


/* The first line of the METAR report is a string in the format yyyy/mm/dd hh:mm
	This is the UTC time the METAR report was generated. This function converts
	the string into a tm structure and returns a time_t. */

time_t utctime(char *timestr, struct tm *tmreport)
{
	time_t seconds;
	char scratchbuf[64];

	/* Year */
	sprintf(scratchbuf, "%c%c%c%c", timestr[0], timestr[1], timestr[2], timestr[3]);
	tmreport->tm_year = atoi(scratchbuf) - 1900;
	
	/* Month */
	sprintf(scratchbuf, "%c%c", timestr[5], timestr[6]);
	tmreport->tm_mon = atoi(scratchbuf) - 1;

	/* Day */
	sprintf(scratchbuf, "%c%c", timestr[8], timestr[9]);
	tmreport->tm_mday = atoi(scratchbuf);

	/* Hour */
	sprintf(scratchbuf, "%c%c", timestr[11], timestr[12]);
	tmreport->tm_hour = atoi(scratchbuf) - 1;

	/* Minutes */
	sprintf(scratchbuf, "%c%c", timestr[14], timestr[15]);
	tmreport->tm_min = atoi(scratchbuf) - 1;

	/* Seconds */
	tmreport->tm_sec = 0;

	/* Convert the tm structure into a time_t */
	putenv("TZ=UTC");
	seconds = mktime(tmreport);
	unsetenv("TZ");
	return seconds;
}
