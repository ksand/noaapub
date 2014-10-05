/* windfrom.c 
   Convert a range of degrees to compass directions */

#include <string.h>

char *windfrom(char *degrees, char buf[])
{
	int d;
	
	d = atoi(degrees);

	if ((d >= 0 && d < 12) || (d > 348 && d <= 360))
		strcpy(buf, "North");
	else if (d > 11 && d < 34)
		strcpy(buf, "North-Northeast");
	else if (d > 33 && d < 57)
		strcpy(buf, "North-East");
	else if (d > 56 && d < 79)
		strcpy(buf, "East-Northeast");
	else if (d > 78 && d < 102)
		strcpy(buf, "East");
	else if (d > 101 && d < 124)
		strcpy(buf, "East-Southeast");
	else if (d > 123 && d < 147)
		strcpy(buf, "South-East");
	else if (d > 146 && d < 169)
		strcpy(buf, "South-Southeast");
	else if (d > 168 && d < 192)
		strcpy(buf, "South");
	else if (d > 191 && d < 214)
		strcpy(buf, "South-Southwest");
	else if (d > 213 && d < 237)
		strcpy(buf, "South-West");
	else if (d > 236 && d < 259)
		strcpy(buf, "West-Southwest");
	else if (d > 258 && d < 282)
		strcpy(buf, "West");
	else if (d > 281 && d < 304)
		strcpy(buf, "West-Northwest");
	else if (d > 303 && d < 327)
		strcpy(buf, "North-West");
	else if (d > 326 && d < 349)
		strcpy(buf, "North-Northwest");
	else
		strcpy(buf, "(unknown)");
	
	return buf;
}

