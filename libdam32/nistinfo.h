/* nistinfo */

/* NIST information */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	NISTINFO_INCLUDE
#define	NISTINFO_INCLUDE	1


#define	NISTINFO_BUFSIZE	80
#define	NISTINFO_ORGSIZE	16


struct nistinfo {
	int		tt ;
	int		l ;
	int		h ;
	int		adv ;
	char		org[NISTINFO_ORGSIZE + 1] ;
} ;


#endif /* NISTINFO_INCLUDE */


