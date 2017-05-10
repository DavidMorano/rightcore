/* havemsg */


#ifndef	HAVEMSG_INCLUDE
#define	HAVEMSG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	NODENAMELEN
#define	NODENAMELEN	257		/* System V */
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#define	HAVEMSG_LCALENDAR	MAILADDRLEN


/* client request message */
struct havemsg_request {
	uint	tag ;
	uint	timestamp ;
	char	calendar[HAVEMSG_LCALENDAR + 1] ;
	uchar	type ;
	uchar	seq ;
} ;

/* server report */
struct havemsg_report {
	uint	tag ;
	uint	timestamp ;
	char	calendar[HAVEMSG_LCALENDAR + 1] ;
	uchar	type ;
	uchar	seq ;
	uchar	rc ;
} ;


/* request types */
enum havemsgtypes {
	havemsgtype_request,
	havemsgtype_report,
	havemsgtype_overlast
} ;

/* response codes */
enum havemsgrcs {
	havemsgrc_ok,
	havemsgrc_invalid,
	havemsgrc_notavail,
	havemsgrc_goingdown,
	havemsgrc_overlast
} ;


#if	(! defined(HAVEMSG_MASTER)) || (HAVEMSG_MASTER == 0)

extern int	havemsg_request(char *,int,int,struct havemsg_request *) ;
extern int	havemsg_report(char *,int,int,struct havemsg_report *) ;

#endif /* HAVEMSG_MASTER */

#endif /* HAVEMSG_INCLUDE */


