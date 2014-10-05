/* mkarray.c */

#include <string.h>

size_t mkarray(char *str, char strarray[][100], size_t size)
{
	size_t i, j, length, counter;
	char c;

	length = strlen(str);
	counter = j = 0;
	
	for (i = 0; i < length; i++)
	{
		c = str[i];
		if (c == ' ' || i == length - 1)
		{
			if (j > 0)
			{
				strarray[counter][j] = 0x00;
				counter++;
				j = 0;
			}
		}
		else
		{
			strarray[counter][j] = c;
			j++;
		}
	}
	return counter;
}
