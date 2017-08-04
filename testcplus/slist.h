/* slist */
/* lang=C++11 */

/* regular (no-frills) pointer queue (not-circular) */


#ifndef	CF_DEBUGS
#define	CF_DEBUGS	1		/* compile-time debugging */
#endif


/* revision history:

	= 2013-03-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

#ifndef	SLIST_INCLUDE
#define	SLIST_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<limits.h>
#include	<new>
#include	<initializer_list>
#include	<localmisc.h>


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif


/* local structures */

template <typename T>
class slist ;

template <typename T>
class slist_iterator ;

template <typename T>
class slist_node {
	slist_node<T>	*next = NULL ;
	slist_node<T>	*prev = NULL ;
	T		val ;
	slist_node(T av) : val(av) { 
	} ;
	~slist_node() {
	} ;
 	friend slist<T> ;
 	friend slist_iterator<T> ;
} ;

template <typename T>
class slist_iterator {
	slist_node<T>	*n = NULL ;
	mutable T	defval ;
public:
	slist_iterator() { } ;
	slist_iterator(slist_node<T>* an) : n(an) { } ;
	slist_iterator &operator = (slist_iterator<T> it) {
	    n = it.n ;
	    return (*this) ;
	} ;
	slist_iterator &operator = (slist_iterator<T> &it) {
	    n = it.n ;
	    return (*this) ;
	} ;
	slist_iterator &operator = (slist_iterator<T> *ip) {
	    n = ip->n ;
	    return (*this) ;
	} ;
	~slist_iterator() {
	    n = NULL ;
	} ;
	friend bool operator == (const slist_iterator<T> &i1,
		const slist_iterator<T> &i2) {
	    return (i1.n == i2.n) ;
	} ;
	friend bool operator != (const slist_iterator<T> &i1,
		const slist_iterator<T> &i2) {
	    return (i1.n != i2.n) ;
	} ;
	T &operator * () const {
	    T &rv = defval ;
	    if (n != NULL) {
		rv = n->val ;
	    }
	    return rv ;
	} ;
	slist_iterator &operator ++ () {
	    if (n != NULL) {
	        n = n->next ;
	    }
	    return (*this) ;
	} ;
	slist_iterator &operator ++ (int) {
	    if (n != NULL) {
	        n = n->next ;
	    }
	    return (*this) ;
	} ;
	slist_iterator &operator += (int inc) {
	    if (n != NULL) {
		while ((n != NULL) && (inc-- > 0)) {
	            n = n->next ;
		}
	    }
	    return (*this) ;
	} ;
	operator int() {
	    return (n != NULL) ;
	} ;
	operator bool() {
	    return (n != NULL) ;
	} ;
} ;

template <typename T>
class slist {
	slist_node<T>	*head = NULL ;
	slist_node<T>	*tail = NULL ;
	int		c = 0 ;
public:
	typedef		slist_iterator<T> iterator ;
	typedef		T value_type ;
	slist() { 
	} ;
	slist(const slist<T> &al) {
	    slist_node<T>	*an = al.head ;
	    if (head != NULL) clear() ;
	    while (an != NULL) {
		add(an->val) ;
	        an = an->next ;
	    }
	} ;
	slist(const slist<T> &&al) {
	    if (head != NULL) clear() ;
	    head = al.head ;
	    tail = al.tail ;
	    c = al.c ;
	    al.head = NULL ;
	    al.tail = NULL ;
	    al.c = 0 ;
	} ;
	slist &operator = (const slist<T> &al) {
	    slist_node<T>	*an = al.head ;
	    if (head != NULL) clear() ;
	    while (an != NULL) {
		add(an->val) ;
	        an = an->next ;
	    }
	} ;
	slist &operator = (const slist<T> &&al) {
	    if (head != NULL) clear() ;
	    head = al.head ;
	    tail = al.tail ;
	    c = al.c ;
	    al.head = NULL ;
	    al.tail = NULL ;
	    al.c = 0 ;
	} ;
	slist(const std::initializer_list<T> &list) {
	    if (head != NULL) clear() ;
	    for (const T &v : list) {
		add(v) ;
	    }
	} ;
	slist &operator = (const std::initializer_list<T> &list) {
	    if (head != NULL) clear() ;
	    for (const T &v : list) {
		add(v) ;
	    }
	    return (*this) ;
	} ;
	slist &operator += (const std::initializer_list<T> &list) {
	    for (const T &v : list) {
		add(v) ;
	    }
	    return (*this) ;
	} ;
	slist &operator += (const T v) {
	    add(v) ;
	    return (*this) ;
	} ;
	~slist() {
	    slist_node<T>	*nn, *n = head ;
	    while (n != NULL) {
		nn = n->next ;
		delete n ;
		n = nn ;
	    } /* end while */
	    head = NULL ;
	    tail = NULL ;
	    c = 0 ;
	} ;
	int clear() {
	    slist_node<T>	*nn, *n = head ;
	    int		rc = c ;
	    while (n != NULL) {
		nn = n->next ;
		delete n ;
		n = nn ;
		rc += 1 ;
	    } /* end while */
	    head = NULL ;
	    tail = NULL ;
	    c = 0 ;
	    return rc ;
	} ;
	int add(const T v) {
	    slist_node<T>	*nn = new(std::nothrow) slist_node<T>(v) ;
	    int			rc = -1 ;
	    if (nn != NULL) {
	        slist_node<T>	*n = tail ;
	        if (n != NULL) {
	            n->next = nn ;
	        } else {
	            head = nn ;
	        }
	        tail = nn ;
	        rc = c++ ;		/* return previous value */
	    }
	    return rc ;
	} ;
	int front(const T **rpp) const {
	    *rpp = (head != NULL) ? &head->val : NULL ;
	    return c ;
	} ;
	int back(const T **rpp) const {
	    *rpp = (tail != NULL) ? &tail->val : NULL ;
	    return c ;
	} ;
	int insfront(const T v) {
	    slist_node<T>	*nn = new(std::nothrow) slist_node<T>(v) ;
	    int			rc = -1 ;
	    if (nn != NULL) {
	        slist_node<T>	*n = head ;
	        if (n != NULL) {
		    nn->next = n->next ;
	        } else {
	            tail = nn ;
	        }
	        head = nn ;
	        rc = c++ ;		/* return previous value */
	    }
	    return rc ;
	} ;
	int insback(const T v) {
	    return add(v) ;
	}
	int count() const {
	    return c ;
	} ;
	int empty() const {
	    return (c == 0) ;
	} ;
	operator int() const {
	    return (c != 0) ;
	} ;
	operator bool() const {
	    return (c != 0) ;
	} ;
	int remfront(T **rpp) {
	    int	rc = -1 ;
	    if (head != NULL) {
		slist_node<T>	*n = head->next ;
		*rpp = &n->val ;
		head = n->next ;
		if (n->next == NULL) {
		    tail = NULL ;
		}
		delete n ;
		rc = --c ;
	    } else {
		*rpp = NULL ;
	    }
	    return rc ;
	} ;
	iterator begin() const {
	    iterator it(head) ;
	    return it ;
	} ;
	iterator end() const {
	    iterator it ;
	    return it ;
	} ;
} ;

#endif /* SLIST_INCLUDE */


