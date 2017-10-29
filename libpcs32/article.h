/* article */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	ARTICLE_INCLUDE
#define	ARTICLE_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vechand.h>
#include	<retpath.h>
#include	<dater.h>
#include	<ema.h>
#include	<localmisc.h>

#include	"ng.h"


#define	ARTICLE		struct article_head
#define	ARTICLE_FL	struct article_flags


enum articleaddrs {
	articleaddr_sender,
	articleaddr_replyto,
	articleaddr_from,
	articleaddr_to,
	articleaddr_cc,
	articleaddr_bcc,
	articleaddr_errorsto,
	articleaddr_deliveredto,
	articleaddr_xoriginalto,
	articleaddr_newsgroups,
	articleaddr_overlast
} ;

enum articlestrs {
	articlestr_messageid,
	articlestr_articleid,
	articlestr_envfrom,
	articlestr_subject,
	articlestr_ngdname,
	articlestr_overlast
} ;

struct article_flags {
	uint		path:1 ;
	uint		envdates:1 ;
	uint		msgdate:1 ;
	uint		ngs:1 ;
	uint		spam:1 ;
} ;

struct article_head {
	RETPATH		path ;
	NG		ngs ;
	VECHAND		envdates ;
	DATER		msgdate ;
	EMA		addr[articleaddr_overlast] ;
	const char	*strs[articlestr_overlast] ;
	ARTICLE_FL	f ;
	uint		aoff ;
	uint		alen ;
	int		clen ;
	int		clines ;
	char		af[articleaddr_overlast] ;
} ;


#if	(! defined(ARTICLE_MASTER)) || (ARTICLE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int article_start(ARTICLE *) ;
extern int article_addenvdate(ARTICLE *,DATER *) ;
extern int article_addmsgdate(ARTICLE *,DATER *) ;
extern int article_addpath(ARTICLE *,const char *,int) ;
extern int article_addng(ARTICLE *,const char *,int) ;
extern int article_addstr(ARTICLE *,int,const char *,int) ;
extern int article_addaddr(ARTICLE *,int,const char *,int) ;
extern int article_ao(ARTICLE *,uint,uint) ;
extern int article_countenvdate(ARTICLE *) ;
extern int article_getenvdate(ARTICLE *,int,DATER **) ;
extern int article_getstr(ARTICLE *,int,const char **) ;
extern int article_getaddrema(ARTICLE *,int,EMA **) ;
extern int article_finish(ARTICLE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* ARTICLE_MASTER */

#endif /* ARTICLE_INCLUDE */


