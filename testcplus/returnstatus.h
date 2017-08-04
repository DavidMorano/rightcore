/* return-status */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */



class returnstatus {
	int		rs ;
public:
	returnstatus(int a) : rs(a) { } ;
	int get() const { return rs ; } ;
	void set(int a) { rs = a ; } ;
} ;


