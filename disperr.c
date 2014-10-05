/* disperr.c */

#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

void disperr(int code, regex_t *exp)
{
	size_t length;
	char *msg;

	length = regerror(code, exp, NULL, 0);
	msg = (char *)alloca(length);
	(void) regerror(code, exp, msg, length);
	fprintf(stderr, "error: %s\n", msg);
}
