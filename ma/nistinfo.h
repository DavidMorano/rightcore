/* nistinfo */

/* NIST information */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	NISTINFO_INCLUDE
#define	NISTINFO_INCLUDE	1


#define	NISTINFO		struct nistinfo
#define	NISTINFO_BUFLEN		80
#define	NISTINFO_ORGLEN		16


struct nistinfo {
	int		tt ;
	int		l ;
	int		h ;
	int		adv ;
	char		org[NISTINFO_ORGLEN+1] ;
} ;


#endif /* NISTINFO_INCLUDE */


