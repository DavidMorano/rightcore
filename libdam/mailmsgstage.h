/* mailmsgstage */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGSTAGE_INCLUDE
#define	MAILMSGSTAGE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vechand.h>
#include	<localmisc.h>


/* object defines */

#define	MAILMSGSTAGE		struct mailmsgstage_head
#define	MAILMSGSTAGE_FL		struct mailmsgstage_flags
#define	MAILMSGSTAGE_MAGIC	0x53232856
#define	MAILMSGSTAGE_TMPDNAME	"/tmp"

#define	MAILMSGSTAGE_OUSECLEN	(1 << 0) /* option: use content-length */
#define	MAILMSGSTAGE_OUSECLINES	(1 << 1) /* option: use content-lines */

#define	MAILMSGSTAGE_MCLEN	(1 << 0)
#define	MAILMSGSTAGE_MCLINES	(1 << 1)
#define	MAILMSGSTAGE_MCTYPE	(1 << 2)
#define	MAILMSGSTAGE_MCENCODING	(1 << 3)
#define	MAILMSGSTAGE_MCTPLAIN	(1 << 4)
#define	MAILMSGSTAGE_MCEPLAIN	(1 << 5)
#define	MAILMSGSTAGE_MCPLAIN	(1 << 6)


struct mailmsgstage_flags {
	uint		useclen:1 ;
	uint		useclines:1 ;
} ;

struct mailmsgstage_head {
	uint		magic ;
	const char	*tmpfname ;
	char		*mapdata ;
	MAILMSGSTAGE_FL	f ;
	vechand		msgs ;
	size_t		tflen ;
	size_t		mapsize ;
	int		nmsgs ;
	int		tfd ;
} ;


#if	(! defined(MAILMSGSTAGE_MASTER)) || (MAILMSGSTAGE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsgstage_start(MAILMSGSTAGE *,int,int,int) ;
extern int mailmsgstage_count(MAILMSGSTAGE *) ;
extern int mailmsgstage_clen(MAILMSGSTAGE *,int) ;
extern int mailmsgstage_clines(MAILMSGSTAGE *,int) ;
extern int mailmsgstage_bodyget(MAILMSGSTAGE *,int,offset_t,const char **) ;
extern int mailmsgstage_bodyread(MAILMSGSTAGE *,int,offset_t,char *,int) ;
extern int mailmsgstage_envcount(MAILMSGSTAGE *,int) ;
extern int mailmsgstage_envaddress(MAILMSGSTAGE *,int,int,const char **) ;
extern int mailmsgstage_envdate(MAILMSGSTAGE *,int,int,const char **) ;
extern int mailmsgstage_envremote(MAILMSGSTAGE *,int,int,const char **) ;
extern int mailmsgstage_hdrcount(MAILMSGSTAGE *,int,const char *) ;
extern int mailmsgstage_hdrikey(MAILMSGSTAGE *,int,int,const char **) ;
extern int mailmsgstage_hdriline(MAILMSGSTAGE *,int,const char *,int,
		int,cchar **) ;
extern int mailmsgstage_hdrival(MAILMSGSTAGE *,int,const char *,int,cchar **) ;
extern int mailmsgstage_hdrval(MAILMSGSTAGE *,int,const char *,cchar **) ;
extern int mailmsgstage_flags(MAILMSGSTAGE *,int) ;
extern int mailmsgstage_finish(MAILMSGSTAGE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGSTAGE_MASTER */

#endif /* MAILMSGSTAGE_INCLUDE */


