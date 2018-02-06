/* artinfo */


#ifndef	ARTINFO_INCLUDE
#define	ARTINFO_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	ARTINFO_LITEM		256


struct artinfo_flags {
	uint		noauthor:1 ;
} ;

struct artinfo {
	struct artinfo_flags	f ;
	time_t	date ;
	uint	lines ;
	char	htmfname[ARTINFO_LITEM + 1] ;
	char	title[ARTINFO_LITEM + 1] ;
	char	author[ARTINFO_LITEM + 1] ;
	char	publisher[ARTINFO_LITEM + 1] ;
} ;


#endif /* ARTINFO_INCLUDE */


