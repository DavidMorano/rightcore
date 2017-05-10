/* filemap */

/* support some buffered file operations */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object supports some buffered file operations for
        low-overhead buffered I/O requirements.


*******************************************************************************/


#ifndef	FILEMAP_INCLUDE
#define	FILEMAP_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vsystem.h>		/* for 'struct ustat' */
#include	<localmisc.h>


/* local defines */

#define	FILEMAP		struct filemap_head


struct filemap_head {
	const char	*bp ;
	const void	*mapdata ;
	struct ustat	sb ;		/* requires 'vsystem.h' */
	size_t		mapsize ;
	size_t		maxsize ;
} ;


#if	(! defined(FILEMAP_MASTER)) || (FILEMAP_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	filemap_open(FILEMAP *,const char *,int,size_t) ;
extern int	filemap_stat(FILEMAP *,struct ustat *) ;
extern int	filemap_read(FILEMAP *,int,void *) ;
extern int	filemap_getline(FILEMAP *,const char **) ;
extern int	filemap_seek(FILEMAP *,offset_t,int) ;
extern int	filemap_tell(FILEMAP *,offset_t *) ;
extern int	filemap_rewind(FILEMAP *) ;
extern int	filemap_close(FILEMAP *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FILEMAP_MASTER */

#endif /* FILEMAP_INCLUDE */


