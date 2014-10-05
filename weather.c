/* weather.c

   METAR Aviation Routine Weather Report
   TAF Terminal Aerodrome Forecast

   ICAO International Civil Aviation Organization
   KPOU Poughkeepsie Airport
   KLGA La Guardia
   KALB Albany International Airport `*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include "weather.h"

#define ICAO_CODE "KPOU"

size_t mkarray(char *str, char strarray[][100], size_t size);
time_t utctime(char *timestr, struct tm *tmreport);
char *station(char *code, char data[]);
int parse(char *datum, struct metdata *info, char data[]);
int charpos(char *str, char c);

int main(int argc, char *argv[])
{
	char buf[BUFSIZE], data[BUFSIZE];
	char strarray[MAXSTRINGS][100];
	char icaocode[5];
	size_t strcount, i;
	int pos;
	FILE * fp;
	time_t utcsecs;
	struct tm tmreport, *tmlocal;
	struct stat filestatus;
	struct metdata info;
	char *argval, rawflag;

	/* Set the default weather station */
	strcpy(icaocode, ICAO_CODE);

	/* Set default flags */
	rawflag = 'F';

	/* Parse command-line arguments */
	if (argc > 1)
	{
		for (i = 1; i < argc; i++)
		{
			strcpy(buf, argv[i]);
			if (buf[0] == '-' && buf[1] == '-')
			{
				strcpy(data, &buf[2]);
				argval = strchr(data, '=');
				if (argval != NULL)
				{
					strcpy(buf, &argval[1]);
					pos = charpos(data, '=');
					data[pos] = '\0';
					if (strlen(data) > 0 && strlen(buf) > 0)
					{
						if (strcmp(data, "station") == 0)
						{
							if (strlen(buf) == 4)
								strcpy(icaocode, buf);
						}
					}
				}
				else
				{
					if (strlen(data) > 0)
					{
						if (strcmp(data, "raw") == 0)
							rawflag = 'T';
					}
				}
			}
		}
	}

	/* Delete the local file weather.out if it exists  */
	if (stat("weather.out", &filestatus) == 0)
		system("rm weather.out");
	
	/* Download the NOAA METAR Observations */
	sprintf(buf, "wget --output-document=weather.out --dns-timeout=40 --quiet http://weather.noaa.gov/pub/data/observations/metar/stations/%s.TXT", icaocode);

	if (system(buf) != 0)
	{
		fprintf(stderr, "wget failed: %s\n", buf);
		return 1;
	}

	fp = fopen("weather.out", "r");

	if (fp == NULL)
		return 1;

	if (fgets(buf, BUFSIZE, fp) == NULL)
	{
		fclose(fp);
		return 1;
	}

	/* Display the name of the weather station based on the ICAO code */
	buf[strlen(buf) - 1] = 0x00;
	printf("%-15s %s (%s) [%s UTC]\n", "WEATHER RPT:", icaocode, station(icaocode, data), buf);
	
	/* Convert the string from the METAR header to a struct tm */
	utcsecs = utctime(buf, &tmreport);

	/* Convert UTC to local time */
	tmlocal = localtime(&utcsecs);
	printf("%-15s %02d/%02d/%d %02d:%02d %s (UTC %ld)\n", "METAR:", tmlocal->tm_mon + 1, tmlocal->tm_mday, tmlocal->tm_year + 1900, tmlocal->tm_hour + 1, tmlocal->tm_min + 1, tmlocal->tm_zone, tmlocal->tm_gmtoff/3600);

	/* Fill in the meteorlogical data structure */
	info.station = icaocode;
	info.tmreport = &tmreport;
	info.type = METAR;
	info.sec = 0;
	info.min = tmlocal->tm_min;
	info.hour = tmlocal->tm_hour;
	info.mday = tmlocal->tm_mday;
	info.mon = tmlocal->tm_mon;
	info.year = tmlocal->tm_year;
	info.gmtoff = tmlocal->tm_gmtoff;
	strcpy(info.zone, tmlocal->tm_zone);
	info.remarks = 0;

	while (fgets(buf, BUFSIZE, fp) != NULL)
	{
		if (rawflag == 'T')
			printf("%-15s %s", "RAW DATA:", buf);

		strcount = mkarray(buf, strarray, MAXSTRINGS);

		for (i = 1; i < strcount; i++)
		{
			if (parse(strarray[i], &info, data) != SKIP)
				printf("%s\n", data);
		}
	}
	
	fclose(fp);
	
	/* Delete the local file weather.out if it exists  */
	if (stat("weather.out", &filestatus) == 0)
		system("rm weather.out");
	
	/* Download the NOAA TAF Forecasts */
	sprintf(buf, "wget --output-document=weather.out --dns-timeout=40 --quiet http://weather.noaa.gov/pub/data/forecasts/taf/stations/%s.TXT", icaocode);

	if (system(buf) != 0)
	{
		fprintf(stderr, "wget failed\n");
		return 1;
	}
	
	fp = fopen("weather.out", "r");

	if (fp == NULL)
		return 1;

	if (fgets(buf, BUFSIZE, fp) == NULL)
	{
		fclose(fp);
		return 1;
	}
	
	info.type = TAF;
	info.remarks = 0;
	
	while (fgets(buf, BUFSIZE, fp) != NULL)
	{
		if (rawflag == 'T')
			printf("%-15s %s", "RAW DATA:", buf);
		
		strcount = mkarray(buf, strarray, MAXSTRINGS);
		info.prev[0] == 0x00;

		for (i = 0; i < strcount; i++)
		{
			if (i > 0)
				strcpy(info.prev, strarray[i-1]);
			
			if (parse(strarray[i], &info, data) != SKIP)
				printf("%s\n", data);
		}
	}

	fclose(fp);
	return 0;
}
