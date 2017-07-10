/* mailmsgviewer */

/* create and cache message content files */


#ifndef	MAILMSGVIEWER_INCLUDE
#define	MAILMSGVIEWER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>


#define	MAILMSGVIEWER_MAGIC	0x54837492
#define	MAILMSGVIEWER		struct mailmsgviewer_head
#define	MAILMSGVIEWER_FL	struct mailmsgviewer_flags


struct mailmsgviewer_flags {
	uint		eof:1 ;
} ;

struct mailmsgviewer_head {
	uint		magic ;
	VECOBJ		lines ;
	MAILMSGVIEWER_FL f ;
	char		*fname ;
	char		*mapbuf ;
	size_t		mapsize ;
	int		ln ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsgviewer_open(MAILMSGVIEWER *,cchar *) ;
extern int mailmsgviewer_getline(MAILMSGVIEWER *,int,cchar **) ;
extern int mailmsgviewer_close(MAILMSGVIEWER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGVIEWER_INCLUDE */


