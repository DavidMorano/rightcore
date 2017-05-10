/* owconfig */


#ifndef	OWCONFIG_INCLUDE
#define	OWCONFIG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<paramfile.h>
#include	<expcook.h>
#include	<localmisc.h>

#include	"ow.h"


#define	OWCONFIG	struct owconfig
#define	OWCONFIG_FLAGS	struct owconfig_flags


struct owconfig_flags {
	uint		p:1 ;
	uint		lockinfo:1 ;
} ;

struct owconfig {
	OWCONFIG_FLAGS	f ;
	PARAMFILE	p ;
	EXPCOOK		cooks ;
	OW		*lip ;
	const char	*cfname ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int owconfig_start(OWCONFIG *,OW *,const char *) ;
extern int owconfig_check(OWCONFIG *) ;
extern int owconfig_read(OWCONFIG *) ;
extern int owconfig_finish(OWCONFIG *) ;

#ifdef	__cplusplus
}
#endif


#endif /* OWCONFIG */



