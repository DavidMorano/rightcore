/* loadave */

/* loadave operations */


#ifndef	LOADAVE_INCLUDE
#define	LOADAVE_INCLUDE	1



#include	<sys/types.h>
#include	<time.h>
#include	<kstat.h>

#include	<vsystem.h>

#include	"misc.h"



/* object defines */

#define	LOADAVE		struct loadave_head
#define	LOADAVE_VALUES	struct loadave_values

#define	LOADAVE_IDLEN		31
#define	LOADAVE_INTUPDATE	(2 * 3600)
#define	LOADAVE_INTMAXOPEN	(4 * 3600)




struct loadave_values {
	time_t	tim_read ;		/* time of read */
	time_t	tim_boot ;		/* boot time of machine */
	uint	hostid ;
	uint	ncpus ;
	uint	nprocs ;
	uint	la1min ;
	uint	la5min ;
	uint	la15min ;
	char	serial[LOADAVE_IDLEN + 1] ;	/* vendor serial number */
	char	provider[LOADAVE_IDLEN + 1] ;	/* vendor name ? */
} ;

struct loadave_head {
	unsigned long	magic ;
	struct loadave_values	v ;	/* cached values */
	kstat_ctl_t	*kcp ;		/* the KSTAT chain pointer */
	kstat_t		*ksp ;		/* a KSTAT pointer */
	time_t		tim_open ;	/* time of KSTAT open */
	time_t		tim_update ;	/* time of last KSTAT chain update */
} ;




#if	(! defined(LOADAVE_MASTER)) || (LOADAVE_MASTER == 0)

extern int loadave_init(LOADAVE *) ;
extern int loadave_free(LOADAVE *) ;
extern int loadave_readvalues(LOADAVE *,LOADAVE_VALUES *) ;
extern int loadave_check(LOADAVE *,time_t) ;

extern loadave	obj_loadave() ;
extern loadave	*new_loadave() ;
extern void	free_loadave(LOADAVE *) ;

#endif /* LOADAVE_MASTER */


#endif /* LOADAVE_INCLUDE */



