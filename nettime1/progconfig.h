/* progconfig */


#ifndef	PROGCONFIG_INCLUDE
#define	PROGCONFIG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<msfile.h>
#include	<paramfile.h>
#include	<expcook.h>

#include	"config.h"
#include	"defs.h"		/* for PROGINFO */


struct config_flags {
	uint		p:1 ;
	uint		lockinfo:1 ;
} ;

struct config {
	struct proginfo	*pip ;
	struct config_flags	f ;
	PARAMFILE	p ;
	EXPCOOK	cooks ;
	time_t		ti_lastcheck ;
	int		intcheck ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	config_start(struct config *,struct proginfo *,
			const char *,int) ;
extern int	config_check(struct config *) ;
extern int	config_read(struct config *) ;
extern int	config_finish(struct config *) ;

#ifdef	__cplusplus
}
#endif


#endif /* PROGCONFIG_INCLUDE */



