/* b_look */

/* AT&T version (Advanced Software Technologies) */


#define	CF_DEBUGS	0		/* non-switchable debug print outs */
#define	CF_OURWORDS	0		/* use our words file as default */


/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1992-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * look for lines beginning with <prefix> in sorted a sorted file
 *
 *   David Korn


	= 2002-03-17, David A­D­ Morano

	I modified it to search for a dictionary file when being executed
	in strange and alien environments (that may not have a dictionary
	file in the standard places).

	I also added support to print out a program version and to enable
	debugging mode.


 */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<shell.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	PROGNAME	"look"
#define	VERSION		"0"

#ifndef	WORDF
#define	WORDF		"share/dict/words"
#endif

#ifndef DICT_FILE
#   define DICT_FILE	"/usr/add-on/ncmp/share/dict/words"
#endif

#ifndef	WORDSFNAME
#define	WORDSFNAME	"/usr/add-on/ncmp/share/dict/words"
#endif

#ifndef	ERROR_CATALOG
#define	ERROR_CATALOG	"libast"
#endif

#ifndef	WORDSVAR
#define	WORDSVAR	"LOOK_WORDS"
#endif


static const char usage[] =
"[-?@(#)$Id: look (AT&T Labs Research) 2002-04-15 $\n]"
"[+NAME?look - displays lines beginning with a given prefix]"
"[+DESCRIPTION?\blook\b displays all lines in the sorted \afile\a arguments"
"	that begin with the given prefix \aprefix\a onto the standard output."
"	The results are unspecified if any \afile\a is not sorted. If"
"	\amax-prefix\a is specified then then records matching prefixes"
"	in the inclusive range \aprefix\a..\amax-prefix\a are displayed.]"
"	[+?If \afile\a is not specified, \blook\b uses the file \b"DICT_FILE"\b"
"	and enables \b--dictionary\b and \b--ignorecase\b.]"
"[V:version?Print the program version and then exit.]"
"[D:debug?Turn on debugging mode.]"
"[d:dictionary?`Phone dictionary order': only letters, digits, and"
"	white space characters are significant in string comparisons.]"
"[f:fold|ignorecase?The search is case insensitive.]"
"[h:header?Skip flat file header (all lines up to first blank line.)]"
"\n"
"\nprefix [- max-prefix] [file ...]\n"
"\n"
"[+EXIT STATUS?]{"
"	[+0?The specified \aprefix\a was found in the file.]"
"	[+1?The specified \aprefix\a was not found in the file.]"
"	[+>1?An error occurred.]"
"}"
"[+SEE ALSO?\bgrep\b(1), \bsort\b(1)]"
 ;


#define D_FLAG		0x01
#define F_FLAG		0x02
#define H_FLAG		0x04
#define	VERSION_FLAG	0x08
#define	DEBUG_FLAG	0x10

#define CLOSE		256

#define EXTRACT(p,b,n)	((b)?extract(p,b,n):(p))


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;


/* local variables */

static const char	*progroots[] = {
	"LOCAL",
	"NCMP",
	"GNU",
	"EXTRA",
	NULL
} ;

static const char	*wordfiles[] = {
	"/usr/add-on/local/share/dict/words",
	"/usr/add-on/ncmp/share/dict/words",
	"/usr/add-on/gnu/share/dict/words",
	"/usr/share/lib/dict/words",
	"/usr/share/dict/words",
	"/usr/dict/words",
	NULL
} ;


/* local subroutines */


static char*
extract(register const char* cp, char* buff, int len)
{
	register char*	bp = buff ;
	register char*	ep = bp + len ;
	register int	n ;


	while (n = *cp++)
	{
	    if (n == '\n')
	    {
	        *bp = 0 ;
	        break ;
	    }
	    if (isalnum(n) || isspace(n))
	    {
	        *bp++ = n ;
	        if (bp >= ep)
	            break ;
	    }
	}

	return buff ;
}
/* end subroutine (extract) */


static int
look(Sfio_t* fp, char* prefix, char* maxprefix, int flags)
{
	Sfoffset_t		low ;
	Sfoffset_t		mid ;
	Sfoffset_t		high ;
	int		n ;
	int		len ;
	int		found ;
	register char*	cp ;
	register char*	dp ;
	register char*	buff = NULL ;
	int		(*compare)(const char*, const char*, size_t) ;


	compare = (flags & F_FLAG) ? strncasecmp : strncmp ;
	if (flags & D_FLAG) {

	    cp = dp = prefix ;
	    while (n = *cp++) {

	        if (isalnum(n) || isspace(n))
	            *dp++ = n ;

	    }

	    *dp = 0 ;
	    len = strlen(prefix) ;
	    if (maxprefix) {

	        cp = dp = maxprefix ;
	        while (n = *cp++)
	            if (isalnum(n) || isspace(n))
	                *dp++ = n ;

	        *dp = 0 ;
	        if ((n = strlen(maxprefix)) < len)
	            n = len ;

	    } else
	        n = len ;

	    buff = (void*)malloc(n + 1) ;

	} else
	    n = len = strlen(prefix) ;

	if (maxprefix && (*compare)(prefix, maxprefix, n) > 0)
	    return 1 ;

	if (flags & H_FLAG)
	    while (sfgetr(fp, '\n', 0) && sfvalue(fp) > 1) ;

	if ((low = sfseek(fp, (Sfoffset_t)0, SEEK_CUR)) < 0 || 
	    (high = sfsize(fp)) <= 0) {

	    found = 0 ;
	    while (cp = sfgetr(fp, '\n', 0))
	    {
	        n = (*compare)(prefix, EXTRACT(cp, buff, len), len) ;
	        if (n <= 0)
	            break ;
	    }

	    if (!cp)
	        return 1 ;

	    if (maxprefix)
	    {
	        prefix = maxprefix ;
	        len = strlen(prefix) ;
	        if (n && 
	            (*compare)(prefix, 
	            EXTRACT(cp, buff, len), len) >= 0)
	            n = 0 ;
	    }
	    found = !n ;
	    while (!n)
	    {
	        sfprintf(sfstdout, "%.*s", sfvalue(fp), cp) ;
	        if (!(cp = sfgetr(fp, '\n', 0)))
	            break ;
	        n = (*compare)(prefix, EXTRACT(cp, buff, len), len) ;
	        if (maxprefix && n > 0)
	            n = 0 ;
	    }

	} else {

	    while ((high - low) > (len + CLOSE)) {

	        mid = (low + high) / 2 ;
	        sfseek(fp, mid, SEEK_SET) ;
	        sfgetr(fp, '\n', 0) ;
	        mid = sftell(fp) ;
	        if (mid > high)
	            break ;

	        if (!(cp = sfgetr(fp, '\n', 0))) {

	            low = mid ;

	        } else {

	            n = (*compare)(prefix, EXTRACT(cp, buff, len), len) ;
	            if (n < 0) {

	                high = mid - len ;

	            } else if (n > 0) {

	                low = mid ;

	            } else {

	                if ((mid+=sfvalue(fp)) >= high)
	                    break ;

	                high = mid ;

	            }
	        }
	    }

	    sfseek(fp, low, SEEK_SET) ;

	    while (low <= high) {

	        if (!(cp = sfgetr(fp, '\n', 0)))
	            return 1 ;

	        n = (*compare)(prefix, EXTRACT(cp, buff, len), len) ;
	        if (n <= 0)
	            break ;

	        low += sfvalue(fp) ;
	    }

	    if (maxprefix) {

	        prefix = maxprefix ;
	        len = strlen(prefix) ;
	        if (n && (*compare)(prefix, 
	            EXTRACT(cp, buff, len), len) >= 0)
	            n = 0 ;

	    }

	    found = !n ;
	    while (!n) {

	        sfprintf(sfstdout, "%.*s", sfvalue(fp), cp) ;

	        if (!(cp = sfgetr(fp, '\n', 0)))
	            break ;

	        n = (*compare)(prefix, EXTRACT(cp, buff, len), len) ;
	        if (maxprefix && n > 0)
	            n = 0 ;

	    }

	    if (buff)
	        free((void*)buff) ;

	}

	return (! found) ;
}
/* end subroutine (look) */


int b_look(int argc, char** argv, void* context)
{
	Sfio_t	*fp ;

	int	rs = 0 ;
	int	i, n ;
	int	flags = 0 ;
	int	f ;

	static const char	*dict[] = { 
	    DICT_FILE, "/usr/lib/dict/words" 	} ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	*ep = 0 ;
	char	*bp ;
	char	*file ;
	char	*cp ;


	cmdinit(argc,argv, context, ERROR_CATALOG, 0) ;

	f = FALSE ;
	while (! f) {

	    switch (optget(argv, usage)) {

	    case 'D':
	        flags |= DEBUG_FLAG ;
	        break ;

	    case 'V':
	        flags |= VERSION_FLAG ;
	        break ;

	    case 'd':
	        flags |= D_FLAG ;
	        continue ;

	    case 'f':
	        flags |= F_FLAG ;
	        continue ;

	    case 'h':
	        flags |= H_FLAG ;
	        continue ;

	    case ':':
	        error(2, "%s", opt_info.arg) ;
	        break ;

	    case '?':
	        error(ERROR_usage(2), "%s", opt_info.arg) ;
	        break ;

	    default:
	        f = TRUE ;
		break ;

	    } /* end switch */

	} /* end while */

	argv += opt_info.index ;
	if (error_info.errors || !(bp = *argv++)) {

#if	CF_DEBUGS
	    debugprintf("b_look: err 1\n") ;
#endif

	    error(ERROR_usage(2), "%s", optusage(NiL)) ;

	}

	if (file = *argv) {

	    argv++ ;
	    if (streq(file, "-") && (ep = *argv)) {

	        argv++ ;
	        if (streq(ep, "-")) {

	            file = ep ;
	            ep = 0 ;

	        } else if (file = *argv)
	            argv++ ;
	    }
	}

/* try to find a file if we don't have one already */

	if (file == NULL) {

	    rs = SR_NOENT ;
	    if ((cp = getenv(WORDSVAR)) != NULL) {

	        rs = perm(cp,-1,-1,NULL,R_OK) ;

	        file = cp ;

	    }

	    if (rs < 0) {

	        for (i = 0 ; progroots[i] != NULL ; i += 1) {

	            cp = getenv(progroots[i]) ;

	            if (cp == NULL) continue ;

	            rs = mkpath2(tmpfname,cp,WORDF) ;

	            if (rs > 0)
	                rs = perm(tmpfname,-1,-1,NULL,R_OK) ;

	            file = tmpfname ;
	            if (rs >= 0)
	                break ;

	        } /* end for */

	    } /* end if (moderately tough measures) */

	    if (rs < 0) {

	        for (i = 0 ; wordfiles[i] != NULL ; i += 1) {

	            rs = perm(wordfiles[i],-1,-1,NULL,R_OK) ;

	            file = (char *) wordfiles[i] ;
	            if (rs >= 0)
	                break ;

	        } /* end for */

	    } /* end if (tough measures) */

#if	CF_OURWORDS
	    if (rs < 0) {
	        rs = SR_OK ;
	        file = WORDSFNAME ;
	    }
#else
	    if (rs < 0)
	        file = NULL ;
#endif /* CF_OURWORDS */

	    if (rs >= 0)
	        flags |= (D_FLAG | F_FLAG) ;

	} /* end if (finding file) */

/* try David Korn's way */

	if (! file) {

	    for (n = 0 ; n < elementsof(dict) ; n += 1) {

	        if (!access(dict[n], R_OK))
	        {
	            file = (char*)dict[n] ;
	            break ;
	        }

	    }

	    if (!file) {

#if	CF_DEBUGS
	        debugprintf("b_look: err 2\n") ;
#endif

	        error(ERROR_system(1), "%s: not found", dict[0]) ;

	    }

	    flags |= (D_FLAG|F_FLAG) ;
	    argv-- ;

	} /* end if */

	if (flags & DEBUG_FLAG) {

	    sfprintf(sfstderr,"%s: dict=%s\n",
	        PROGNAME,file) ;

	}

	if (flags & VERSION_FLAG) {

	    sfprintf(sfstderr,"%s: version=%s\n",
	        PROGNAME,VERSION) ;

	    goto ret0 ;
	}

	n = 0 ;
	do {

	    if (streq(file, "-") || 
	        streq(file, "/dev/stdin") || 
	        streq(file, "/dev/fd/0")) {

	        fp = sfstdin ;

	    } else if (!(fp = sfopen(NiL, file, "r"))) {

#if	CF_DEBUGS
	        debugprintf("b_look: err 3\n") ;
#endif

#ifdef	COMMENT
	        error(ERROR_system(0), "%s: cannot open", file) ;
#endif

	        continue ;
	    }

	    if (look(fp, bp, ep, flags))
	        n = 1 ;

	    if (fp != sfstdin)
	        sfclose(fp) ;

	} while (file = *argv++) ;

ret0:
	return n || error_info.errors ;
}
/* end subroutine (b_look) */



