/* pingtab */


#ifndef	PINGTAB_INCLUDE
#define	PINGTAB_INCLUDE		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netdb.h>

#include	<bfile.h>


#define	PINGTAB_MAGIC	0x9876dcba
#define	PINGTAB		struct pingtab_head
#define	PINGTAB_ENT	struct pingtab_ent


struct pingtab_head {
	uint		magic ;
	bfile		pfile ;
} ;

struct pingtab_ent {
	char		hostname[MAXHOSTNAMELEN + 1] ;
	int		intminping ;
	int		timeout ;
} ;


#if	(! defined(PINGTAB_MASTER)) || (PINGTAB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pingtab_open(PINGTAB *,const char *) ;
extern int pingtab_close(PINGTAB *) ;
extern int pingtab_read(PINGTAB *,PINGTAB_ENT *) ;
extern int pingtab_rewind(PINGTAB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(PINGTAB_MASTER)) || (PINGTAB_MASTER == 0) */


#endif /* PINGTAB_INCLUDE */



