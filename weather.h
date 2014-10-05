/* weather.h */

#define BUFSIZE 255
#define MAXSTRINGS 100
#define MAXSTRSIZE 32
#define SUCCESS 0
#define SKIP 1

enum rectype {METAR, TAF};

/* The tm structure tmreport is UTC time the report was generated.
 * sec, min, hour, mday, mon, year, gmtoff and zone are the
 * local time  */

struct metdata
{
	char *station;
	enum rectype type;
	struct tm *tmreport;
	int sec;
	int min;
	int hour;
	int mday;
	int mon;
	int year;
	int gmtoff;
	char zone[4];
	int remarks;
	char prev[MAXSTRSIZE];
};
