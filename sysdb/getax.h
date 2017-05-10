/* getax */

/* UNIX® System databases */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	GETAX_INCLUDE
#define	GETAX_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<pwd.h>
#include	<grp.h>
#include	<shadow.h>
#include	<project.h>


#if	(! defined(GETAX_MASTER)) || (GETAX_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int getpw_begin() ;
extern int getpw_ent(struct passwd *,char *,int) ;
extern int getpw_end() ;
extern int getpw_name(struct passwd *,char *,int,const char *) ;
extern int getpw_uid(struct passwd *,char *,int,uid_t) ;

extern int getsp_begin() ;
extern int getsp_ent(struct spwd *,char *,int) ;
extern int getsp_end() ;
extern int getsp_name(struct spwd *,char *,int,const char *) ;

extern int getgr_begin() ;
extern int getgr_ent(struct group *,char *,int) ;
extern int getgr_end() ;
extern int getgr_name(struct group *,char *,int,const char *) ;
extern int getgr_gid(struct group *,char *,int,gid_t) ;

extern int getpj_begin() ;
extern int getpj_ent(struct project *,char *,int) ;
extern int getpj_end() ;
extern int getpj_name(struct project *,char *,int,const char *) ;
extern int getpj_pjid(struct project *,char *,int,projid_t) ;

#ifdef	__cplusplus
}
#endif

#endif /* GETAX_MASTER */

#endif /* GETAX_INCLUDE */


