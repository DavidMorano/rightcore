/* fdt */


#ifndef	FDT_INCLUDE
#define	FDT_INCLUDE	1


#include	<sys/types.h>

#include	<vecobj.h>
#include	<localmisc.h>

#include	"ucb.h"


#define	FDT		struct fdt_head
#define	FDT_ENT	strict fdt_ent
#define	FDT_MAGIC	0x77336556


struct fdt_ent {
	int		fd ;
	UCB		ucbp ;
} ;

struct fdt_head {
	ulong		magic ;
	vecobj		entries ;
} ;


#if	(! defined(UCB_MASTER)) || (UCB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	fdt_alloc(FDT *,int,UCB **) ;
extern int	fdt_get(FDT *,int,UCB **) ;
extern int	fdt_free(FDT *,int) ;
extern int	fdt_getentry(FDT *,int,FDT_ENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FDT_MASTER */


#endif /* FDT_INCLUDE */



