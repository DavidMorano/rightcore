/* pcspolls */

/* access manager interface to PCS loadable-object polling */


/* revision history:

	- 2008-10-07, David A­D­ Morano
        This module was originally written to allow for the main part of the
        PCS-poll facility to be a loadable module.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#ifndef	PCSPOLLS_INCLUDE
#define	PCSPOLLS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<pthread.h>

#include	<thrcomm.h>
#include	<pcsconf.h>		/* need def for PCSCONF */
#include	<localmisc.h>


#define	PCSPOLLS	struct pcspolls_head
#define	PCSPOLLS_OBJ	struct pcspolls_obj
#define	PCSPOLLS_INFO	struct pcspolls_i
#define	PCSPOLLS_NAME	struct pcspolls_n
#define	PCSPOLLS_THREAD	struct pcspolls_thread
#define	PCSPOLLS_FL	struct pcspolls_flags
#define	PCSPOLLS_MAGIC	0x88773421

#define	PCSPOLLS_POLLCNAME	"pcspolls"


struct pcspolls_n {
	const char	*name ;
	uint		objsize ;
	uint		infosize ;
} ;

struct pcspolls_obj {
	const char	*name ;
	int		objsize ;
	int		infosize ;
} ;

struct pcspolls_i {
	int		dummy ;
} ;

struct pcspolls_flags {
	uint		working:1 ;
} ;

struct pcspolls_thread {
	PCSPOLLS	*op ;
	PCSCONF		*pcp ;
	const char	*sn ;
	const char 	*pr ;
	const char	**envv ;
	THRCOMM		tc ;
	pid_t		pid ;
	pthread_t	tid ;
	int		trs ;
	volatile int	f_exiting ;
} ;

struct pcspolls_head {
	uint		magic ;
	PCSCONF		*pcp ;
	const char	*a ;		/* memory-allocation */
	const char	*sn ;
	const char 	*pr ;
	const char	**envv ;
	PCSPOLLS_THREAD	t ;
	PCSPOLLS_FL	f ;
} ;


#if	(! defined(PCSPOLLS_MASTER)) || (PCSPOLLS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	pcspolls_start(PCSPOLLS *,PCSCONF *,const char *) ;
extern int	pcspolls_info(PCSPOLLS *,PCSPOLLS_INFO *) ;
extern int	pcspolls_cmd(PCSPOLLS *,int) ;
extern int	pcspolls_finish(PCSPOLLS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSPOLLS_MASTER */

#endif /* PCSPOLLS_INCLUDE */


