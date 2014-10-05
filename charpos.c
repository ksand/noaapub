/* charpos.c */

#include <stdio.h>
#include <string.h>

int charpos(char *str, char c)
{
	int len, i, pos;

	pos = -1;
	len = strlen(str);
	for (i = 0; i < len; i++)
	{
		if (str[i] == c)
		{
			pos = i;
			break;
		}
	}
	return pos;
}
