/* getfield */


#define	DEBUG	0


/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
		David A.D. Morano

*									*
*	FUNCTIONAL DESCRIPTION:						*
*	'Getfield' reads a file for a field descriptor and returns	*
*	the data contained in it.					*
*									*
*	PARAMETERS:							*
*	file		filename					*
*	field		field name (eg, 'FROM')				*
*	data		data in field (must be of size BUFSIZE)
	buflen		length of data buffer above
*									*
*	RETURNED VALUE:							*
		BAD	no header found
		OK	a header was found
*									*
*	SUBROUTINES CALLED:						*

*									*
************************************************************************/



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/errno.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<baops.h>

#include	"localmisc.h"
#include	"config.h"

#if	DEBUG
#include	<stdio.h>
#include	"defs.h"
#endif



/* external subroutines */

extern int	fstat() ;
extern int	mm_getfield() ;


/* external variables */

#if	DEBUG
extern struct global	g ;
#endif




int getfield(file,field,data,buflen)
char	file[] ;
char	field[] ;
char	data[] ;
int	buflen ;
{
	struct ustat	stat_f ;

	bfile	mm_file, *fp = &mm_file ;

	int	len, dlen ;
	int	fd ;


#if	DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
		"getfield: want to get \"%s\" in file \"%s\"\n",
		field,file) ;
#endif

	buflen = BUFSIZE ;
	data[0] = '\0' ;

/* open file for reading */

	if (bopen(fp,file,"r",0666) < 0) return BAD ;

#if	DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"getfield: OK 1\n") ;
#endif

	if (bcontrol(fp,BC_STAT,&stat_f) < 0) goto badret ;

#if	DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"getfield: OK 3\n") ;
#endif

	if (mm_getfield(fp,(offset_t) 0,stat_f.st_size,field,data,buflen) < 0)
		goto badret ;

#if	DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"getfield: got stuff \n\"%s: %s\"\n",field,data) ;
#endif

	bclose(fp) ;

	return OK ;

badret:
	bclose(fp) ;

	return BAD ;
}
/* end subroutine (getfield) */


