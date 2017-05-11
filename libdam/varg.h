/* varg */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VARG_INCLUDE
#define	VARG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<stdarg.h>


#define	VARG_BUILTIN	1


#define	varg_list		__va_list

#define	varg_end(ap)		0 /* nothing */


#if	VARG_BUILTIN && (! defined(__lint)) && \
	(defined(__BUILTIN_VA_ARG_INCR) || defined(__sparc) || \
	defined(__i386))


#define	varg_starter(list,name)	\
	(void) (list = ((__va_list) &(name)))
#define	varg_start(list,name)	\
	(void) (list = ((__va_list) &name))
#define	varg_arg(list,mode)	\
	((mode *) __builtin_va_arg_incr((mode *) list))[0]


#else /* builtin  */


#define	varg_start(list,name)	\
	(void) (list = ((__va_list) &name))
#define	varg_arg(ap,type)	\
	((type *) (ap = (__va_list) (((char *) ap) + sizeof(type))))[-1]


#endif /* builtin */

#endif /* VARG_INCLUDE */


