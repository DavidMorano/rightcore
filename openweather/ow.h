/* ow (Open Weather) */


#ifndef	OW_INCLUDE
#define	OW_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>
#include	<paramfile.h>
#include	<logfile.h>
#include	<logsys.h>
#include	<localmisc.h>

#define	OW_SEARCHNAME	"weather"
#define	OW_CFNAME	"conf"
#define	OW_VDNAME	"var"
#define	OW_LDNAME	"log"
#define	OW_METARDNAME	"metars"
#define	OW_INTSHORT	(2*60)
#define	OW_INTLONG	(15*60)
#define	OW_WHOST	"weather.noaa.gov"
#define	OW_WPREFIX	"pub/data/observations/metar/decoded"
#define	OW_WS		"kbos"
#define	OW_TO		5		/* web-access time-out */
#define	OW_WEBPORT	"www"
#define	OW_LOGFACILITY	"daemon"

#define	OW		struct ow
#define	OW_FLAGS	struct ow_flags

#include	"owconfig.h"


struct ow_flags {
	uint		stores:1 ;
	uint		svars:1 ;
	uint		lf:1 ;		/* LOGFILE */
	uint		ls:1 ;		/* LOGSYS */
} ;

struct ow {
	OW_FLAGS	f, open ;
	VECSTR		stores ;
	VECSTR		svars ;
	OWCONFIG	c ;
	LOGFILE		lf ;
	LOGSYS		ls ;
	const char	*pr ;		/* from arguments */
	const char	*sn ;		/* from static config */
	const char	*vd ;		/* from static config */
	const char	*ws ;		/* from arguments */
	const char	*rn ;		/* root-name (derived) */
	const char	*ld ;		/* log-file directory component name */
	const char	*nodename ;
	const char	*domainname ;
	const char	*cfname ;
	const char	*whost ;	/* from configuration */
	const char	*wprefix ; 	/* from configuration */
	const char	*defws ;	/* from configuration */
	const char	*logfname ;	/* for LOGFILE */
	const char	*logfacility ;	/* for LOGSYS */
	time_t		daytime ;
	time_t		ti_weather ;	/* weather-file m-time */
	uid_t		pruid ;
	gid_t		prgid ;
	pid_t		pid ;
	int		intpoll ;	/* from configuration */
	int		oflags ;	/* from arguments */
	int		to ;		/* from arguments */
	int		wmin ;		/* from current time-of-day */
	int		wfd ;		/* "weather" FD */
	char		logid[LOGFILE_LOGIDLEN+1] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	ow_start(OW *,const char *,const char *,const char *,
			const char *,time_t,int,int) ;
extern int	ow_finish(OW *,int) ;
extern int	ow_setentry(OW *,const char **,const char *,int) ;
extern int	ow_nodedomain(OW *) ;
extern int	ow_setmin(OW *) ;
extern int	ow_prid(OW *) ;
extern int	ow_isvalid(OW *,time_t) ;
extern int	ow_logprintf(OW *,const char *,...) ;

#ifdef	__cplusplus
}
#endif


#endif /* OW_INCLUDE */



