/* testdate */



#include	<vsystem.h>

#include	"localmisc.h"



struct testdate {
	time_t	sec ;			/* seconds */
	ushort	msec ;			/* milliseconds */
	short	timeoff ;		/* minutes west of GMT */
	char	tzname[8] ;		/* timezone name */
} ;



