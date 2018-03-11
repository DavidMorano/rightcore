/* env */

/* manipulate the process environment variable list */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is supposed to be Sun Solaris® safe!

	HOWEVER:

        Do not use this crap! This crap modifies the environment that was passed
        to us on invocation. But we have code that runs as both a regular UNIX®
        process as well as built-in commands within the SHELL. We also have tons
        of library code that does not know where it is running from! So, do not
        use this crap! This crap allocates memory for a new environment entry
        but there is no way to keep track of that allocation for later deletion.
        Instead of using this crap, make your own new copy of whatever
        environment that you want to use and manipulate that.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<stdlib.h>
#include	<string.h>


/* local defines */

#define	CF_A		(defined(HAVE_SETENV) && (HAVE_SETENV != 0))
#define	CF_B		(defined(SYSHAS_SETENV) && (SYSHAS_SETENV != 0))
#define	CF_SETENV	(! (CF_A || CF_B))

#define	CF_C		(defined(HAVE_UNSETENV) && (HAVE_UNSETENV != 0))
#define	CF_D		(defined(SYSHAS_UNSETENV) && (SYSHAS_UNSETENV != 0))
#define	CF_UNSETENV	(! (CF_C || CF_D))


/* external variables */

extern char	**environ ;


/* forward references */

#if	CF_SETENV || CF_UNSETENV
static char	*__findenv(const char *,int *) ;
#endif


/* exported subroutines */


#if	CF_SETENV

/*
 * setenv --
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rewrite is set, replace any current value.
 *
 * PUBLIC: #ifndef HAVE_SETENV
 * PUBLIC: int setenv __P((const char *, const char *, int));
 * PUBLIC: #endif
 */

int setenv(const char *name,const char *value,int rewrite)
{
	static int	alloced ;		/* if allocated space before */
	char 		*c ;
	int 		l_value, offset ;

	if (*value == '=')			/* no `=' in value */
	    ++value ;
	l_value = strlen(value) ;
	if ((c = __findenv(name, &offset))) {	/* find if already exists */

	    if (!rewrite)
	        return (0) ;
	    if (strlen(c) >= l_value) {	/* old larger; copy over */
	        while (*c++ = *value++) ;
	        return (0) ;
	    }

	} else {					/* create new slot */
	    int 	cnt ;
	    char	**p ;

	    for (p = environ, cnt = 0; *p; ++p, ++cnt) ;

	    if (alloced) {			/* just increase size */
	        environ = (char **)realloc((char *)environ,
	            (size_t)(sizeof(char *) * (cnt + 2))) ;
	        if (!environ)
	            return (-1) ;

	    } else {				/* get new space */

	        alloced = 1;		/* copy old entries into it */
	        p = malloc((size_t)(sizeof(char *) * (cnt + 2))) ;
	        if (!p)
	            return (-1) ;
	        memmove(p, environ, cnt * sizeof(char *)) ;
	        environ = p ;
	    }
	    environ[cnt + 1] = NULL ;
	    offset = cnt ;

	} /* end if */

	for (c = (char *)name; *c && *c != '='; ++c);	/* no `=' in name */

	if (!(environ[offset] =			/* name + `=' + value */
	malloc((size_t)((int)(c - name) + l_value + 2))))
	    return (-1) ;

	for (c = environ[offset]; (*c = *name++) && *c != '='; ++c) ;

	for (*c++ = '='; *c++ = *value++;) ;

	return (0) ;
}
/* end subroutine (setenv) */

#endif /* CF_SETENV */


#if	CF_UNSETENV

/*
 * unsetenv(name) --
 *	Delete environmental variable "name".
 *
 * PUBLIC: #ifndef HAVE_UNSETENV
 * PUBLIC: void unsetenv __P((const char *));
 * PUBLIC: #endif
 */

void unsetenv(const char *name)
{
	char 		**p ;
	int 		offset ;

	while (__findenv(name, &offset)) {	/* if set multiple times */
	    for (p = &environ[offset];; ++p) {
	        if (!(*p = *(p + 1)))
	            break ;
	    }
	}

}
/* end subroutine (unsetenv) */

#endif /* CF_UNSETENV */


/* local subroutines */


#if	CF_SETENV || CF_UNSETENV

/*
 * __findenv --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Sets offset to be the offset of the name/value combination in the
 *	environmental array, for use by setenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 *
 *	This routine *should* be a static; don't use it.
 */

static char *__findenv(const char *name,int *offset)
{
	int 		len ;
	char 		*np ;
	char 		**p, *c ;

	if (name == NULL || environ == NULL)
	    return (NULL) ;

	for (np = (char *) name ; *np && *np != '='; ++np)
	    continue ;

	len = np - name ;
	for (p = environ; (c = *p) != NULL; ++p) {
	    if (strncmp(c, name, len) == 0 && c[len] == '=') {
	        *offset = p - environ ;
	        return (c + len + 1) ;
	    }
	}

	return NULL ;
}
/* end subroutine (__findenv) */

#endif /* (CF_SETENV || CF_UNSETENV) */


