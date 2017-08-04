/* pq */
/* lang=C++11 */

/* regular (no-frills) pointer queue (not-circular) */


/* revision history:

	= 1998-03-03, David A�D� Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright � 1998 David A�D� Morano.  All rights reserved. */

#ifndef	SLIST_INCLUDE
#define	SLIST_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* for 'uint' */
#include	<initializer_list>

template <class T>
class slist ;

template <typename T>
class slist_node {
	slist_node<T>	*next = NULL ;
	slist_node<T>	*prev = NULL ;
	T		val ;
	slist_node<T>(T av) : val(av) { } ;
 	friend slist<T> ;
} ;

template <typename T>
class slist {
	slist_node<T>	*head = NULL ;
	slist_node<T>	*tail = NULL ;
	int		c = 0 ;
public:
	typedef		slist_node<T>* interator ;
	typedef		T value_type ;
	slist() { } ;
	~slist() {
	    slist_node<T>	*nn, *n = head ;
	    while (n != NULL) {
		nn = n.next ;
		delete n ;
		n = nn ;
	    } /* end while */
	    head = NULL ;
	    tail = NULL ;
	    c = 0 ;
	} ;
	int add(T v) {
	    slist_node<T>	*n = tail ;
	    slist_node<T>	*nn = new slist_node<T>(v) ;
	    if (n != NULL) {
	        n.next = nn ;
	    } else {
	        head = nn ;
	    }
	    tail = nn ;
	    return ++c ;
	} ;
	int front(T &rp) const {
	    *rp = (head != NULL) ? &head->val : NULL ;
	    return c ;
	} ;
	int back(T &rp) const {
	    *rp = (tail != NULL) ? &tail->val : NULL ;
	    return c ;
	} ;
	int insfront(T v) {
	    slist_node<T>	*nn = new slist_node<T>(v) ;
	    slist_node<T>	*n = head ;
	    if (n != NULL) {
		nn.next = n.next ;
	    } else {
	        tail = nn ;
	    }
	    head = nn ;
	    return ++c ;
	} ;
	int insback(T v) {
	    return add(v) ;
	}
	int count() const {
	    return c ;
	} ;
	int remfront(T *rp) {
	    int	rc = -1 ;
	    if (head != NULL) {
		slist_node<T>	*n = head.next ;
		*rp = &n->val ;
		head = n.next ;
		if (n.next == NULL) {
		    tail = NULL ;
		}
		delete n ;
		rc = --c ;
	    } else {
		*rp = NULL ;
	    }
	    return rc ;
	} ;
} ;

#endif /* SLIST_INCLUDE */


