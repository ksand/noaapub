/* parse.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <limits.h>
#include "weather.h"

const char *ord[] = {"", "1st", "2nd", "3rd", "4th", "5th", "6th", "7th", "8th", "9th", 
	"10th", "11th", "12th", "13th", "14th", "15th", "16th", "17th", "18th", "19th", "20th", 
	"21st", "22nd", "23rd", "24th", "25th", "26th", "27th", "28th", "29th", "30th", "31st"};

char *windfrom(char *degrees, char buf[]);
void disperr(int code, regex_t *exp);

int parse(char *datum, struct metdata *info, char data[])
{
	regex_t compiled;
	size_t length;
	int errorcode;
	char buf[32], tmp[64];
	int i, start, index, speed, gust, elevation;
	float mph, gustmph;
	float temp_centigrade, temp_fahrenheit, dewpoint_centigrade, dewpoint_fahrenheit;
	int t, td, f, fd;
	int from_year, from_month, from_day, from_hour, to_year, to_month, to_day, to_hour;
	time_t utcsecs;
	struct tm tmutc, *tmlocal;
	int barmov;
	float bardelta;
	static int miles = 0;

	/* Observation time in format ddhhmmZ. This is UTC "Zulu" Time. */
	errorcode = regcomp(&compiled, "Z$", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{		
			buf[0] = datum[0];
			buf[1] = datum[1];
			buf[2] = 0x00;
			tmutc.tm_mday = atoi(buf);

			buf[0] = datum[2];
			buf[1] = datum[3];
			buf[2] = 0x00;
			tmutc.tm_hour = atoi(buf) - 1;

			buf[0] = datum[4];
			buf[1] = datum[5];
			buf[2] = 0x00;
			tmutc.tm_min = atoi(buf) - 1;

			tmutc.tm_mon = info->tmreport->tm_mon;
			tmutc.tm_year = info->tmreport->tm_year;
			tmutc.tm_sec = 0;

			putenv("TZ=UTC");
			utcsecs = mktime(&tmutc);
			unsetenv("TZ");
			tmlocal = localtime(&utcsecs);

			if (info->type == TAF)
				sprintf(data, "%-15s %02d/%02d/%d %02d:%02d %s", "TAF:", tmlocal->tm_mon + 1, tmlocal->tm_mday, tmlocal->tm_year + 1900, tmlocal->tm_hour + 1, tmlocal->tm_min + 1, tmlocal->tm_zone);
			else
				sprintf(data, "%-15s %02d/%02d/%d %02d:%02d %s", "REPORT:", tmlocal->tm_mon + 1, tmlocal->tm_mday, tmlocal->tm_year + 1900, tmlocal->tm_hour + 1, tmlocal->tm_min + 1, tmlocal->tm_zone);

			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);
	
	regfree(&compiled);

	/* TAF */
	if (strcmp(datum, "TAF") == 0)
	{
		/* do nothing */
		return SKIP;
	}
	
	/* Weather Station */
	if (strcmp(datum, info->station) == 0)
	{
		/* do nothing */
		return SKIP;
	}
	
	/* AUTO */
	if (strcmp(datum, "AUTO") == 0)
	{
		/* do nothing */
		return SKIP;
	}

	/* RA (rain) */
	errorcode = regcomp(&compiled, "RA$", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{		
			if (datum[0] == 'R')
				sprintf(data, "%-15s Rain", "PRECIP:");
			else
			{
				if (datum[0] == '-')
					sprintf(data, "%-15s Light rain", "PRECIP:");

				if (datum[0] == '+')
					sprintf(data, "%-15s Heavy rain", "PRECIP:");
			}
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);

	/* SN (snow) */
	errorcode = regcomp(&compiled, "SN$", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{		
			if (datum[0] == 'S')
				sprintf(data, "%-15s Snow", "PRECIP:");
			else
			{
				if (datum[0] == '-')
					sprintf(data, "%-15s Light snow", "PRECIP:");

				if (datum[0] == '+')
					sprintf(data, "%-15s Heavy snow", "PRECIP:");
			}
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);

	/* -RAPL (light rain with ice pellets) */
	if (strcmp(datum, "-RAPL") == 0)
	{
		sprintf(data, "%-15s Light rain with ice pellets", "PRECIP:");
		return SUCCESS;
	}
	
	/* -FZRAPL (light freezing rain with ice pellets) */
	if (strcmp(datum, "-FZRAPL") == 0)
	{
		sprintf(data, "%-15s Light freezing rain with ice pellets", "PRECIP:");
		return SUCCESS;
	}
	
	/* BR (mist) */
	if (strcmp(datum, "BR") == 0)
	{
		sprintf(data, "%-15s Mist", "ACTIVITY:");
		return SUCCESS;
	}
	
	/* Wind velocity in knots dddffKT */
	errorcode = regcomp(&compiled, "^[0-9][0-9][0-9][0-9][0-9]KT", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = datum[0];
			buf[1] = datum[1];
			buf[2] = datum[2];
			buf[3] = 0x00;
			windfrom(buf, tmp);
			buf[0] = datum[3];
			buf[1] = datum[4];
			buf[2] = 0x00;
			speed = atoi(buf);
			mph = speed * 1.15078;
			sprintf(data, "%-15s From the %s at %4.1f mph (%d knots)", "WIND:", tmp, mph, speed);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);
	
	/* Wind velocity in knots with gusts dddffGffKT */
	errorcode = regcomp(&compiled, "^[0-9][0-9][0-9][0-9][0-9]G[0-9][0-9]KT", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = datum[0];
			buf[1] = datum[1];
			buf[2] = datum[2];
			buf[3] = 0x00;
			windfrom(buf, tmp);
			buf[0] = datum[3];
			buf[1] = datum[4];
			buf[2] = 0x00;
			speed = atoi(buf);
			mph = speed * 1.15078;
			buf[0] = datum[6];
			buf[1] = datum[7];
			buf[2] = 0x00;
			gust = atoi(buf);
			gustmph = gust * 1.15078;
			sprintf(data, "%-15s From the %s at %4.1f mph (%d knots), gusts up to %4.1f mph (%d knots)", "WIND:", tmp, mph, speed, gustmph, gust);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);

	/* Variable wind direction VRBddKT */
	errorcode = regcomp(&compiled, "^VRB[0-9][0-9]KT", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = datum[3];
			buf[1] = datum[4];
			buf[2] = 0x00;
			speed = atoi(buf);
			mph = speed * 1.15078;
			sprintf(data, "%-15s Variable at %4.1f mph (%d knots)", "WIND:", mph, speed);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);

	/* Visibility in statute miles */
	errorcode = regcomp(&compiled, "SM$", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			length = strlen(datum);
			index = 0;

			if (datum[0] == 'P')
			{
				start = 1;	
				strcpy(tmp, "More than ");
			}
			else
			{
				if (miles > 0 && miles < 6)
				{
					sprintf(buf, "%d ", miles);
					index = strlen(buf);
					miles = 0;
				}
				
				start = 0;
				tmp[0] = 0x00;
			}

			for (i = start; i < length; i++)
			{
				if (datum[i] == 'S')
					break;

				buf[index] = datum[i];
				index++;
			}
			buf[index] = 0x00;
			sprintf(data, "%-15s %s%s miles", "VISIBILITY:", tmp, buf);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);

	/* Overcast with a ceiling */
	errorcode = regcomp(&compiled, "^OVC[0-9][0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = datum[3];
			buf[1] = datum[4];
			buf[2] = datum[5];
			buf[3] = 0x00;
			elevation = atoi(buf) * 100;
			sprintf(data, "%-15s Overcast at %d feet", "CLOUDS:", elevation);
			return SUCCESS;
		}	
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);

	/* Broken clouds and elevation in feet */
	errorcode = regcomp(&compiled, "^BKN[0-9][0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = datum[3];
			buf[1] = datum[4];
			buf[2] = datum[5];
			buf[3] = 0x00;
			elevation = atoi(buf) * 100;
			sprintf(data, "%-15s Broken at %d feet", "CLOUDS:", elevation);
			return SUCCESS;
		}	
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);
	
	/* Scattered clouds and elevation in feet */
	errorcode = regcomp(&compiled, "^SCT[0-9][0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = datum[3];
			buf[1] = datum[4];
			buf[2] = datum[5];
			buf[3] = 0x00;
			elevation = atoi(buf) * 100;
			sprintf(data, "%-15s Scattered at %d feet", "CLOUDS:", elevation);
			return SUCCESS;
		}	
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);
	

	/* Clear Skies */
	if (strcmp(datum, "CLR") == 0)
	{
		sprintf(data, "%-15s Clear", "SKY:", elevation);
		return SUCCESS;
	}

	/* Temperature and dew point nn/nn */
	errorcode = regcomp(&compiled, "^[0-9][0-9]/[0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = datum[1];
			buf[1] = datum[2];
			buf[2] = 0x00;
			t = atoi(buf);
			f = (t * 9)/5 + 32;
			buf[0] = datum[4];
			buf[1] = datum[5];
			buf[2] = 0x00;
			td = atoi(buf);
			fd = (td * 9)/5 + 32;
			sprintf(data, "%-15s %dF (%dC), Dew Point %dF (%dC)", "TEMPERATURE:", f, t, fd, td);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);
	
	/* Temperature and dew point Mnn/Mnn */
	errorcode = regcomp(&compiled, "^M[0-9][0-9]/M[0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = '-';
			buf[1] = datum[1];
			buf[2] = datum[2];
			buf[3] = 0x00;
			t = atoi(buf);
			f = (t * 9)/5 + 32;
			buf[0] = '-';
			buf[1] = datum[5];
			buf[2] = datum[6];
			buf[3] = 0x00;
			td = atoi(buf);
			fd = (td * 9)/5 + 32;
			sprintf(data, "%-15s %dF (%dC), Dew Point %dF (%dC)", "TEMPERATURE:", f, t, fd, td);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);
	
	/* Temperature and dew point nn/Mnn */
	errorcode = regcomp(&compiled, "^[0-9][0-9]/M[0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = datum[1];
			buf[1] = datum[2];
			buf[2] = 0x00;
			t = atoi(buf);
			f = (t * 9)/5 + 32;
			buf[0] = '-';
			buf[1] = datum[4];
			buf[2] = datum[5];
			buf[3] = 0x00;
			td = atoi(buf);
			fd = (td * 9)/5 + 32;
			sprintf(data, "%-15s %dF (%dC), Dew Point %dF (%dC)", "TEMPERATURE:", f, t, fd, td);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);
	
	/* Altimenter */
	errorcode = regcomp(&compiled, "^A[0-9][0-9][0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			/* do nothing */;
			return SKIP;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);
	
	/* RMK */
	if (strcmp(datum, "RMK") == 0)
	{
		/* Flag the start of the remarks section */
		info->remarks = 1;
		return SKIP;
	}

	/* AO2 remark */
	if (strcmp(datum, "AO2") == 0)
	{
		/* do nothing */
		return SKIP;
	}

	/* AO1 remark */
	if (strcmp(datum, "AO1") == 0)
	{
		/* do nothing */
		return SKIP;
	}
	
	/* SLP remark (barometric pressure at sea level) hPa is "hectopascal" which is equivalent to "millibar" */
	errorcode = regcomp(&compiled, "^SLP[0-9][0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = '1';
			buf[1] = ',';
			buf[2] = '0';
			buf[3] = datum[3];
			buf[4] = datum[4];
			buf[5] = '.';
			buf[6] = datum[5];
			buf[7] = 0x00;
			sprintf(data, "%-15s %s hPa at sea level", "PRESSURE:", buf);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);
		
	regfree(&compiled);

	/* TSB remark (thunderstorm) */
	errorcode = regcomp(&compiled, "^TSB[0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = datum[3];
			buf[1] = datum[4];
			buf[2] = 0x00;
			sprintf(data, "%-15s Thunderstorms at %s minutes past the hour", "ACTIVITY:", buf);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);
		
	regfree(&compiled);

	/* Atmospheric pressure at sea level*/
	errorcode = regcomp(&compiled, "^FEW[0-9][0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = '1';
			buf[1] = ',';
			buf[2] = '0';
			buf[3] = datum[3];
			buf[4] = datum[4];
			buf[5] = '.';
			buf[6] = datum[5];
			buf[7] = 0x00;
			sprintf(data, "%-15s %s hPa at sea level", "PRESSURE:", buf);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);

	/* Precipitation in last hour in inches */
	errorcode = regcomp(&compiled, "^P[0-9][0-9][0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = datum[1];
			buf[1] = datum[2];
			buf[2] = '.';
			buf[3] = datum[3];
			buf[4] = datum[4];
			buf[5] = 0x00;
			sprintf(data, "%-15s %s inches in the last hour", "PRECIP:", buf);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);

	/* Temperature remark Taaaabbbb */
	errorcode = regcomp(&compiled, "^T[0-1][0-9][0-9][0-9][0-1][0-9][0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			/* Temperature */
			if (datum[1] == '1')
			{
				buf[0] = '-';
				buf[1] = datum[2];
				buf[2] = datum[3];
				buf[3] = '.';
				buf[4] = datum[4];
				buf[5] = 0x00;
				temp_centigrade = atof(buf);
				temp_fahrenheit = (temp_centigrade * 9)/5 + 32;
			}
			else
			{
				buf[0] = datum[2];
				buf[1] = datum[3];
				buf[2] = '.';
				buf[3] = datum[4];
				buf[4] = 0x00;
				temp_centigrade = atof(buf);
				temp_fahrenheit = (temp_centigrade * 9)/5 + 32;
			}

			/* Dew Point */
			if (datum[5] == '1')
			{
				buf[0] = '-';
				buf[1] = datum[6];
				buf[2] = datum[7];
				buf[3] = '.';
				buf[4] = datum[8];
				buf[5] = 0x00;
				dewpoint_centigrade = atof(buf);
				dewpoint_fahrenheit = (dewpoint_centigrade * 9)/5 + 32;
			}
			else
			{
				buf[0] = datum[6];
				buf[1] = datum[7];
				buf[2] = '.';
				buf[3] = datum[8];
				buf[4] = 0x00;
				dewpoint_centigrade = atof(buf);
				dewpoint_fahrenheit = (dewpoint_centigrade * 9)/5 + 32;
			}
			sprintf(data, "%-15s %3.1fF (%3.1fC), Dew Point %3.1fF (%3.1fC)", "TEMPERATURE:", temp_fahrenheit, temp_centigrade, dewpoint_fahrenheit, dewpoint_centigrade);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);
		
	regfree(&compiled);

	/* "1" remark - highest temp recorded in the previous 6 hours */
	if (info->remarks == 1)
	{
		errorcode = regcomp(&compiled, "^1[0-9][0-9][0-9][0-9]", REG_EXTENDED);
		if (errorcode == 0)
		{
			errorcode = regexec(&compiled, datum, 0, NULL, 0);
			if (errorcode == 0)
			{
				if (datum[1] == '1')
					buf[0] = '-';
				else
					buf[0] = '+';

				buf[1] = datum[2];
				buf[2] = datum[3];
				buf[3] = datum[4];
				buf[4] = 0x00;
				temp_centigrade = atof(buf) / 10;
				temp_fahrenheit = (temp_centigrade * 9)/5 + 32;
				sprintf(data, "%-15s %3.1fF (%3.1fC) highest in previous 6 hours", "TEMPERATURE:", temp_fahrenheit, temp_centigrade);
				return SUCCESS;
			}
		}
		else
			disperr(errorcode, &compiled);
	}

	regfree(&compiled);

	/* "2" remark - lowest temp recorded in the previous 6 hours */
	if (info->remarks == 1)
	{
		errorcode = regcomp(&compiled, "^2[0-9][0-9][0-9][0-9]", REG_EXTENDED);
		if (errorcode == 0)
		{
			errorcode = regexec(&compiled, datum, 0, NULL, 0);
			if (errorcode == 0)
			{
				if (datum[1] == '1')
					buf[0] = '-';
				else
					buf[0] = '+';

				buf[1] = datum[2];
				buf[2] = datum[3];
				buf[3] = datum[4];
				buf[4] = 0x00;
				temp_centigrade = atof(buf) / 10;
				temp_fahrenheit = (temp_centigrade * 9)/5 + 32;
				sprintf(data, "%-15s %3.1fF (%3.1fC) lowest in previous 6 hours", "TEMPERATURE:", temp_fahrenheit, temp_centigrade);
				return SUCCESS;
			}
		}
		else
			disperr(errorcode, &compiled);
	}

	regfree(&compiled);

	/* "5" remark - change in barometric pressure */
	if (info->remarks == 1)
	{
		errorcode = regcomp(&compiled, "^5[0-9][0-9][0-9][0-9]", REG_EXTENDED);
		if (errorcode == 0)
		{
			errorcode = regexec(&compiled, datum, 0, NULL, 0);
			if (errorcode == 0)
			{
				buf[0] = datum[1];
				buf[1] = 0x00;
				barmov = atoi(buf);

				buf[0] = datum[2];
				buf[1] = datum[3];
				buf[2] = datum[4];
				buf[3] = 0x00;
				bardelta = atof(buf) / 10;
				sprintf(data, "%-15s %s %3.1f hPa", "BAROMETER:", "", bardelta);

				if (barmov >= 0 && barmov <= 3)
					sprintf(data, "%-15s %s %3.1f hPa", "BAROMETER:", "Fall", bardelta);
				if (barmov == 4)
					sprintf(data, "%-15s %s %3.1f hPa", "BAROMETER:", "Steady", bardelta);
				if (barmov >= 5 && barmov <= 8)
					sprintf(data, "%-15s %s %3.1f hPa", "BAROMETER:", "Rise", bardelta);
				
				return SUCCESS;
			}
		}
		else
			disperr(errorcode, &compiled);
	}

	regfree(&compiled);

	/* "6" remark - 3 and 6 hour precipitation */
	errorcode = regcomp(&compiled, "^6[0-9][0-9][0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			buf[0] = datum[1];
			buf[1] = datum[2];
			buf[2] = '.';
			buf[3] = datum[3];
			buf[4] = datum[4];
			buf[5] = 0x00;
			sprintf(data, "%-15s %s inches rain in past 3 or 6 hours", "PRECIP:", buf);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);


	/* From Record  FMddhhmm Indicates a rapid change in prevailing conditions */
	errorcode = regcomp(&compiled, "^FM[0-9][0-9][0-9][0-9][0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{		
			buf[0] = datum[2];
			buf[1] = datum[3];
			buf[2] = 0x00;
			tmutc.tm_mday = atoi(buf);

			buf[0] = datum[4];
			buf[1] = datum[5];
			buf[2] = 0x00;
			tmutc.tm_hour = atoi(buf) - 1;

			buf[0] = datum[6];
			buf[1] = datum[7];
			buf[2] = 0x00;
			tmutc.tm_min = atoi(buf);

			tmutc.tm_mon = info->tmreport->tm_mon;
			tmutc.tm_year = info->tmreport->tm_year;
			tmutc.tm_sec = 0;

			putenv("TZ=UTC");
			utcsecs = mktime(&tmutc);
			unsetenv("TZ");
			tmlocal = localtime(&utcsecs);

			sprintf(data, "%-15s %02d/%02d/%d %02d:%02d %s", "CHANGE AT:", tmlocal->tm_mon + 1, tmlocal->tm_mday, tmlocal->tm_year + 1900, tmlocal->tm_hour + 1, tmlocal->tm_min, tmlocal->tm_zone);
			return SUCCESS;
		}
	}
	else
		disperr(errorcode, &compiled);

	regfree(&compiled);

	/* TEMPO */
	if (strcmp(datum, "TEMPO") == 0)
	{
		sprintf(data, "TEMPORARY CONDITIONS");
		return SUCCESS;
	}
	
	/* Forecast validity period in DDHH format */
	errorcode = regcomp(&compiled, "^[0-9][0-9][0-9][0-9]/[0-9][0-9][0-9][0-9]", REG_EXTENDED);
	if (errorcode == 0)
	{
		errorcode = regexec(&compiled, datum, 0, NULL, 0);
		if (errorcode == 0)
		{
			/* Month */	
			buf[0] = datum[0];
			buf[1] = datum[1];
			buf[2] = 0x00;
			tmutc.tm_mday = atoi(buf);

			/* Day */
			buf[0] = datum[2];
			buf[1] = datum[3];
			buf[2] = 0x00;
			tmutc.tm_hour = atoi(buf) - 1;

			tmutc.tm_mon = info->tmreport->tm_mon;
			tmutc.tm_year = info->tmreport->tm_year;
			tmutc.tm_min = 0;
			tmutc.tm_sec = 0;

			/* Convert from UTC to Local Time */
			putenv("TZ=UTC");
			utcsecs = mktime(&tmutc);
			unsetenv("TZ");
			tmlocal = localtime(&utcsecs);

			/* Store in local variables */
			from_year = tmlocal->tm_year + 1900;
			from_month = tmlocal->tm_mon + 1;
			from_day = tmlocal->tm_mday;
			from_hour = tmlocal->tm_hour + 1;

			printf("FROM ==> %02d/%02d/%d %02d:%02d UTC\n", tmutc.tm_mon + 1, tmutc.tm_mday, tmutc.tm_year + 1900, tmutc.tm_hour + 1, tmutc.tm_min);

			/* Month */
			buf[0] = datum[5];
			buf[1] = datum[6];
			buf[2] = 0x00;
			tmutc.tm_mday = atoi(buf);

			/* Day */
			buf[0] = datum[7];
			buf[1] = datum[8];
			buf[2] = 0x00;
			tmutc.tm_hour = atoi(buf) - 1;

			/* Convert from UTV to Local Time */
			putenv("TZ=UTC");
			utcsecs = mktime(&tmutc);
			unsetenv("TZ");
			tmlocal = localtime(&utcsecs);
			
			/* Store in local variables */
			to_year = tmlocal->tm_year + 1900;
			to_month = tmlocal->tm_mon + 1;
			to_day = tmlocal->tm_mday;
			to_hour = tmlocal->tm_hour + 1;

			printf("TO ==>   %02d/%02d/%d %02d:%02d UTC\n", tmutc.tm_mon + 1, tmutc.tm_mday, tmutc.tm_year + 1900, tmutc.tm_hour + 1, tmutc.tm_min);

			sprintf(data, "%-15s From %02d/%02d/%d %02d:00 %s to %02d/%02d/%d %02d:00 %s", "VALID:", from_month, from_day, from_year, from_hour, tmlocal->tm_zone, to_month, to_day, to_year, to_hour, tmlocal->tm_zone);
			return SUCCESS;
		}
		regfree(&compiled);
	}
	else
		disperr(errorcode, &compiled);
	
	regfree(&compiled);

	/* AMD */
	if (strcmp(datum, "AMD") == 0)
	{
		sprintf(data, "AMENDED");
		return SUCCESS;
	}
	
	/* SKC */
	if (strcmp(datum, "SKC") == 0)
	{
		sprintf(data, "%-15s Clear", "SKY:");
		return SUCCESS;
	}
	
	/* VCSH */
	if (strcmp(datum, "VCSH") == 0)
	{
		sprintf(data, "%-15s Precipitation within 16 kilometers (but not at) the aerodrome", "ACTIVITY:");
		return SUCCESS;
	}

	/* Default  */
	miles = atoi(datum);

	if (!(miles > 0 && miles < 6))
		sprintf(data, "-- %s", datum);
	else
		return SKIP;

	
	return SUCCESS;
}



