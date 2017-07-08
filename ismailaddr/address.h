/* address */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	ADDRESS_INCLUDE
#define	ADDRESS_INCLUDE		1


/* mail address */
#ifndef	MAILADDRLEN
#define	MAILADDRLEN		(3 * MAXHOSTNAMELEN)
#endif


#define	ADDRESSTYPE_NOHOST	-1
#define	ADDRESSTYPE_LOCAL	0
#define	ADDRESSTYPE_UUCP	1
#define	ADDRESSTYPE_ARPA	2
#define	ADDRESSTYPE_ARPAROUTE	3


#define	ADDRESS_LOCALHOST	"*LOCAL*"
#define	LOCALHOSTPART		"*LOCAL*"


#ifdef	__cplusplus
extern "C" {
#endif

extern int addressparse(const char *,int,char *,char *) ;
extern int addressjoin(char *,int,const char *,const char *,int) ;
extern int addressarpa(char *,int,const char *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* ADDRESS_INCLUDE */


