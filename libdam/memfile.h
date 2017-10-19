/* memfile */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MEMFILE_INCLUDE
#define	MEMFILE_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* extra types */


#define	MEMFILE_MAGIC	0x54728822
#define	MEMFILE		struct memfile_head


struct memfile_head {
	uint		magic ;
	char		*dbuf ;
	void		*bp ;
	offset_t	off ;
	size_t		dlen ;
	uint		fsize ;
	int		pagesize ;
	int		fd ;
} ;


#if	(! defined(MEMFILE_MASTER)) || (MEMFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	memfile_open(MEMFILE *,cchar *,int,mode_t) ;
extern int	memfile_write(MEMFILE *,const void *,int) ;
extern int	memfile_len(MEMFILE *) ;
extern int	memfile_allocation(MEMFILE *) ;
extern int	memfile_tell(MEMFILE *,offset_t *) ;
extern int	memfile_buf(MEMFILE *,void *) ;
extern int	memfile_close(MEMFILE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MEMFILE_MASTER */

#endif /* MEMFILE_INCLUDE */


