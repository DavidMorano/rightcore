/* cmbuf */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was adopted for use from the DWD program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CMBUF_INCLUDE
#define	CMBUF_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<localmisc.h>


#define	CMBUF		struct cmbuf_head
#define	CMBUF_SPACE	struct cmbuf_space


struct cmbuf_space {
	char		*bp ;
	int		bl ;
} ;

struct cmbuf_head {
	uint		magic ;
	char		*buf ;
	char		*bp ;
	int		buflen ;
	int		bl ;
} ;


#if	(! defined(CMBUF_MASTER)) || (CMBUF_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int cmbuf_start(CMBUF *,const char *,int) ;
extern int cmbuf_space(CMBUF *,CMBUF_SPACE *) ;
extern int cmbuf_added(CMBUF *,int) ;
extern int cmbuf_getline(CMBUF *,int,const char **) ;
extern int cmbuf_getlastline(CMBUF *,const char **) ;
extern int cmbuf_finish(CMBUF *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CMBUF_MASTER */

#endif /* CMBUF_INCLUDE */


