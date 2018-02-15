/* tagtrack */

/* track tagtypes in DWB documents */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This module was changed to serve in the REFERM program.

	= 2005-10-01, David A­D­ Morano
        This was changed to work in the MMCITE program. The old REFERM program
        is really obsolete. It used a database lookup strategy to remote
        databases. The high-level problem is: what to do if the cited BIB entry
        isn't found? How does a maintainer of the present (local) document know
        what that BIB entry was? The new strategy (implemented by the MMCITE
        program) is more like what is done with BibTeX in the TeX (or LaTeX)
        world. All BIB databases are really expected to be maintained by the
        document creator -- not some centralized entiry. The older centralized
        model reflected more the use in the corporate world (where different
        people create BIB entries) than in the more "modern"
        personal-responsibility type of world! :-) Anyway, this is the way the
        gods seem to now want to do things. Deal with it!

*/

/* Copyright © 1998,2005 David A­D­ Morano.  All rights reserved. */


#ifndef	TAGTRACK_INCLUDE
#define	TAGTRACK_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vechand.h>
#include	<vecobj.h>
#include	<localmisc.h>		/* extra types */


#define	TAGTRACK		struct tagtrack_head
#define	TAGTRACK_TAG		struct tagtrack_tag
#define	TAGTRACK_ESC		struct tagtrack_esc
#define	TAGTRACK_ENT		struct tagtrack_e
#define	TAGTRACK_CUR		struct tagtrack_cur


enum tagtypes {
	tagtype_table,
	tagtype_example,
	tagtype_figure,
	tagtype_equation,
	tagtype_overlast
} ;

/* store tags here */
struct tagtrack_tag {
	const char	*name ;		/* tag name */
	int		c ;		/* tag-type count */
	int		tagtype ;	/* tag-type */
} ;

struct tagtrack_esc {
	struct tagtrack_tag	*tagp ;
	uint		eoff ;
	int		elen ;
	int		fi ;
} ;

struct tagtrack_e {
	uint		eoff ;
	int		elen ;
	int		fi ;
	int		v ;
} ;

struct tagtrack_head {
	uint		magic ;
	int		c[tagtype_overlast] ;
	int		lc ;		/* last count */
	int		ltt ;		/* last tag-type */
	vechand		tags ;
	vecobj		list ;		/* list of escapes */
} ;

struct tagtrack_cur {
	int		i ;
} ;


#if	(! defined(TAGTRACK_MASTER)) || (TAGTRACK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	tagtrack_start(TAGTRACK *) ;
extern int	tagtrack_finish(TAGTRACK *) ;
extern int	tagtrack_scanline(TAGTRACK *,int,uint,const char *,int) ;
extern int	tagtrack_adds(TAGTRACK *,int,uint,int,const char *,int) ;
extern int	tagtrack_curbegin(TAGTRACK *,TAGTRACK_CUR *) ;
extern int	tagtrack_curend(TAGTRACK *,TAGTRACK_CUR *) ;
extern int	tagtrack_enum(TAGTRACK *,TAGTRACK_CUR *,TAGTRACK_ENT *) ;
extern int	tagtrack_audit(TAGTRACK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TAGTRACK_MASTER */


#endif /* TAGTRACK_INCLUDE */



