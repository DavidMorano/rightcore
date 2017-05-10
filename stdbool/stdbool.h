/* stdbool.h */
 
 
/* revision history:

	= 2001-02-09, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a total hack. I tried to find out what is supposed to be in here
        and made up guesses (based on standards printed on the web).

	Mostly: if it is not defined below, it is probably already defined in
	existing system headers.


*******************************************************************************/


#ifndef _STDBOOL_H_
#define _STDBOOL_H_	1


#if (! defined(__cplusplus))

#ifndef	_bool_true_false_are_defined
#define	_bool_true_false_are_defined	1

#ifndef	bool
#define bool		_Bool
#endif

#ifndef	true
#define	true		1
#endif

#ifndef	false
#define	false		0
#endif

#endif /* _bool_true_false_are_defined */

#endif /* if not C++ */


#endif /* _STDBOOL_H_ */


