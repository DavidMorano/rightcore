/* noise */

/* noise generator */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	NOISE_INCLUDE
#define	NOISE_INCLUDE	1


#define	NOISE		struct noise_head ;


struct noise_flags {
	int	daemon:1 ;
} ;


struct noise_head {
	void	**va ;
	int	i ;		/* highest index */
	int	e ;		/* extent of array */
	int	c ;		/* count of items in list */
	int	policy ;
	int	f_sorted ;	/* flag indicating sort state */
} ;


#endif /* NOISE_INCLUDE */


