/* libut */

/* last modified %G% version %I% */
/* virtual-system definitions */


/* revision history:

	= 1998-03-21, David A­D­ Morano

	This module was originally written.


*/


#ifndef	LIBUT_INCLUDE
#define	LIBUT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<fcntl.h>
#include	<xti.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int ut_open(const char *,int,struct t_info *) ;

extern int ut_accept(s,addr,lenp) ;
extern int ut_alloc(fd,stype,fields,rpp) ;
extern int ut_bind(fd,req,ret) ;
extern int ut_close(fd) ;
extern int ut_connect(fd,sndcall,rcvcall) ;
extern int ut_free(p,stype) ;
extern int ut_listen(s,backlog) ;
extern int ut_look(fd) ;
extern int ut_sync(fd) ;

#ifdef	__cplusplus
}
#endif

#endif /* LIBUT_INCLUDE */



