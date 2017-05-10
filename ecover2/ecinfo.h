/* ecinfo */


#ifndef	ECINFO_INCLUDE
#define	ECINFO_INCLUDE	1


#include	<sys/types.h>

#include	"localmisc.h"


#define	ECINFO_REASONLEN	100


/* general acknowledgement response */
struct ecinfo_data {
	uint	filelen ;
	uint	filetime ;
	uint	filesum ;
	uint	msglen ;
	uint	msgtime ;
	uint	msgsum ;
	uint	tag ;
	uint	type ;			/* message type */
} ;


/* message types */
enum ecinfotypes {
	ecinfotype_data,
	ecinfotype_overlast
} ;

/* response codes */
enum ecinforcs {
	ecinforc_ok,
	ecinforc_invalid,
	ecinforc_overlast
} ;


#if	(! defined(ECINFO_MASTER)) || (ECINFO_MASTER == 0)

extern int	ecinfo_data(char *,int,int,struct ecinfo_data *) ;

#endif /* ECINFO_MASTER */


#endif /* ECINFO_INCLUDE */



