/* pwi */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PWI_INCLUDE
#define	PWI_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<ipasswd.h>
#include	<localmisc.h>


#define	PWI_MAGIC	0x99889998
#define	PWI		struct pwi_head
#define	PWI_CUR		struct pwi_c
#define	PWI_FL		struct pwi_flags


struct pwi_c {
	int		i ;
} ;

struct pwi_flags {
	uint		f:1 ;
} ;

struct pwi_head {
	uint		magic ;
	PWI_FL		f ;
	IPASSWD		db ;
	int		i ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int pwi_open(PWI *,const char *,const char *) ;
extern int pwi_lookup(PWI *,char *,int,const char *) ;
extern int pwi_close(PWI *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PWI_INCLUDE */


