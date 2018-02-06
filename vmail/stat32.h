/* stat32 */


/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

#ifndef	STAT32_INCLUDE
#define	STAT32_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>


#if	(! defined(_SYSCALL32))

struct ustat32 {
	uint		st_dev;
	uint		st_pad1[3];
	uint		st_ino;
	uint		st_mode;
	uint		st_nlink;
	int		st_uid;
	int		st_gid;
	uint		st_rdev;
	uint		st_pad2[2];
	int		st_size;
	uint		st_pad3;
	timestruc_t	st_atim;
	timestruc_t	st_mtim;
	timestruc_t	st_ctim;
	uint		st_blksize;
	uint		st_blocks;
	char		st_fstype[_ST_FSTYPSZ];
	uint		st_pad4[8];
} ;

#endif

#endif /* STAT32_INCLUDE */


