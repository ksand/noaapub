/* station.c
   Get the name of a weather station based on its IOAC code */

#include <string.h>

char *station(char *code, char data[])
{
	if (strcmp(code, "KPOU") == 0)
		strcpy(data, "Poughkeepsie Airport");
	else if (strcmp(code, "KALB") == 0)
		strcpy(data, "Albany International Airport");
	else if (strcmp(code, "KLGA") == 0)
		strcpy(data, "La Guardia");
	else
		strcpy(data, code);
	
	return data;
}

