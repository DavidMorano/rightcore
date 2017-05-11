/* pcsgetnames */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSGETNAMES_INCLUDE
#define	PCSGETNAMES_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


enum pcsnametypes {
	pcsnametype_name,
	pcsnametype_fullname,
	pcsnametype_projinfo,
	pcsnametype_org,
	pcsnametype_overlast
} ;


#if	(! defined(PCSGETNAMES_MASTER)) || (PCSGETNAMES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pcsgetname(const char *,char *,int,const char *) ;
extern int pcsgetfullname(const char *,char *,int,const char *) ;
extern int pcsgetnames(const char *,char *,int,const char *,int) ;
extern int pcsgetprojinfo(const char *,char *,int,const char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSGETNAMES_MASTER */

#endif /* PCSGETNAMES_INCLUDE */


