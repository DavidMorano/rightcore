/* ucsem */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	UCSEM_INCLUDE
#define	UCSEM_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<semaphore.h>
#include	<localmisc.h>


#define	UCSEM_MAGIC	0x31419877
#define	UCSEM		struct ucsem
#define	UCSEM_NAMELEN	(MAXNAMELEN + 1)


struct ucsem {
	uint		magic ;
	sem_t		s, *sp ;
	char		name[UCSEM_NAMELEN + 1] ;
} ;


#if	(! defined(UCSEM_MASTER)) || (UCSEM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ucsem_open(UCSEM *,cchar *,int,mode_t,uint) ;
extern int	ucsem_close(UCSEM *) ;
extern int	ucsem_start(UCSEM *,int,uint) ;
extern int	ucsem_destroy(UCSEM *) ;
extern int	ucsem_getvalue(UCSEM *,int *) ;
extern int	ucsem_wait(UCSEM *) ;
extern int	ucsem_waiti(UCSEM *) ;
extern int	ucsem_trywait(UCSEM *) ;
extern int	ucsem_post(UCSEM *) ;
extern int	ucsem_unlink(UCSEM *) ;

extern int	ucsemunlink(const char *) ;
extern int	unlinkucsem(const char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* UCSEM_MASTER */

#endif /* UCSEM_INCLUDE */


