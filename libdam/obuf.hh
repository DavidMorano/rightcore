/* obuf */

/* Output Buffer */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2016 David A­D­ Morano.  All rights reserved. */

#ifndef	OBUF_INCLUDE
#define	OBUF_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>
#include	<string>
#include	<localmisc.h>


#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif


class obuf {
	std::string	b ;
	int		oi ;		/* output index */
public:
	obuf() : oi(0) { 
	} ;
	obuf(cchar *sbuf) : oi(0) { 
	    int	i ;
	    for (i = 0 ; sbuf[i] ; i += 1) {
		b.push_back(sbuf[i]) ;
	    }
	} ;
	obuf(cchar *sbuf,int slen) : oi(0) {
	    int	i ;
	    if (slen < 0) slen = strlen(sbuf) ;
	    for (i = 0 ; sbuf[i] ; i += 1) {
		b.push_back(sbuf[i]) ;
	    }
	} ;
	int operator [] (int i) const {
	    const int	n = b.size() ;
	    int		rch = 0 ;
	    if ((oi+i) < n) rch = MKCHAR(b[oi+i]) ;
	    return rch ;
	} ;
	obuf &operator += (int ch) {
	    b.push_back(ch) ;
	    return *this ;
	} ;
	int add(int ch) {
	    b.push_back(ch) ;
	    return (b.size() - oi) ;
	} ;
	int add(cchar *sp,int sl = -1) {
	    int	i ;
	    if (sl < 0) sl = strlen(sp) ;
	    for (i = 0 ; i < sl ; i += 1) {
		b.push_back(sp[i]) ;
	    }
	    return (b.size() - oi) ;
	} ;
	int count() const {
	    return (b.size() - oi) ;
	} ;
	int len() const {
	    return (b.size() - oi) ;
	} ;
	int at(int i) const {
	    const int	n = b.size() ;
	    int		rch = 0 ;
	    if ((oi+i) < n) rch = MKCHAR(b[oi+i]) ;
	    return rch ;
	} ;
	int adv(int al) ;
} ;


#endif /* OBUF_INCLUDE */


