/* ftpsession */

/* FTPSESSION operations */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	FTPSESSION_INCLUDE
#define	FTPSESSION_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<vsystem.h>

#include	<FtpLibrary.h>


/* object defines */

#define	FTPSESSION		struct ftpsession_head


struct ftpsession_head {
	uint		magic ;
	FTP		*sp ;
} ;


typedef struct ftpsession_head	ftpsession ;


#ifndef	FTPSESSION_MASTER

#ifdef	__cplusplus
extern "C" {
#endif

extern int ftpsession_open(FTPSESSION *,char *,char *,char *,char *) ;
extern int ftpsession_fileread(FTPSESSION *,char *) ;
extern int ftpsession_setmode(FTPSESSION *,int) ;
extern int ftpsession_read(FTPSESSION *,char *,int) ;
extern int ftpsession_write(FTPSESSION *,char *,int) ;
extern int ftpsession_close(FTPSESSION *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FTPSESSION_MASTER */

#endif /* FTPSESSION_INCLUDE */


