/* ema_local */


#ifndef	EMALOCAL_INCLUDE
#define	EMALOCAL_INCLUDE	1


#include	<envstandards.h>
#include	<localmisc.h>			/* additional types */


#define	CMD_FL		struct cmd_flags
#define	CMD_AFL		struct cmd_aflags
#define	CMD_LOCAL	struct cmd_local


struct cmd_flags {
	uint		count:1 ;
	uint		info:1 ;
	uint		expand:1 ;		/* also 'flat' */
} ;

struct cmd_aflags {
	uint		address:1 ;
	uint		route:1 ;
	uint		comment:1 ;
	uint		best:1 ;
	uint		original:1 ;
	uint		any:1 ;
} ;

struct cmd_local {
	CMD_FL		f ;
	CMD_AFL		af ;
	int		spc ;
} ;


#endif /* EMALOCAL_INCLUDE */


