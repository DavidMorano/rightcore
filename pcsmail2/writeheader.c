/* putheader */


#define	CF_DEBUG	0


/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
 *		J.Mukerji						*
		David A.D. Morano
 *									*


 * putheader( fp, addbcc )
 *
 * putheader outputs a standard header for the message into the given open
 * file. If addbcc is 1 then a BCC line is also added to the header.


*************************************************************************/



#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<baops.h>
#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* external variables */

extern struct global	g ;




int writeheader(mfp, f_addbcc )
bfile	*mfp ;
int	f_addbcc ;
{
	int	rs = OK ;
	char	*cp ;


	if (! f_addbcc)
	bprintf(mfp,"X-Mailer:   AT&T PCS %s %s/%s\n",
	    g.progname,(g.f.sysv_ct ? "SYSV" : "BSD"),
	    VERSION) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"writeheader: message ID\n") ;
#endif

	if ((! f_addbcc) && (*mess_id != '\0'))
	    bprintf(mfp,"Message-Id: %s\n",
	        mess_id) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"writeheader: references\n") ;
#endif

	if (*reference != '\0')
	    bprintf(mfp,"References: %s\n",
	        reference) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"writeheader: from\n") ;
#endif

	if (*from != '\0')
	    bprintf(mfp,"From:       %s\n",
	        from) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"writeheader: sentby\n") ;
#endif

	if ((! f_addbcc) && (*sentby != '\0')) {

	    bprintf(mfp,"Sender:     %s\n",
	        sentby) ;

	    strcpy(from,sentby) ;

	}

#if	CF_DEBUG
	logfile_printf(&g.eh,"writeheader: recipient\n") ;
#endif

	if (recipient[0] == '\0')
	    strcpy(recipient,"!pcs") ;

	bprintf(mfp,"To:         %s\n",
	    (*realto != '\0') ? realto : recipient) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"writeheader: copyto\n") ;
#endif

	if (*copyto != '\0')
	    bprintf(mfp,"Cc:         %s\n",
	        copyto) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"writeheader: bcopyto\n") ;
#endif

	if (f_addbcc && (*bcopyto != '\0'))
	    bprintf(mfp,"Bcc:        %s\n",
	        bcopyto) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"writeheader: moptions\n") ;
#endif

	if (*moptions != '\0')
	    bprintf(mfp,"Options:    %s\n",
	        moptions) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"writeheader: header date string\n") ;
#endif

	if ((g.date_header != NULL) && (g.date_header[0] != '\0'))
	    bprintf(mfp,"Date:       %s\n",
	        g.date_header) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"writeheader: subject\n") ;
#endif

	if (*subject != '\0') {

	    cp = subject ;
	    while (ISWHITE(*cp)) cp += 1 ;

	    bprintf(mfp,"Subject:    %s\n",cp) ;

	}

#if	CF_DEBUG
	logfile_printf(&g.eh,"writeheader: keys\n") ;
#endif

	if (*keys != '\0')
	    bprintf(mfp,"Keywords:   %s\n",keys) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"writeheader: full_name\n") ;
#endif

	if ((! f_addbcc) && f_fullname && (g.fullname != NULL))
	    bprintf(mfp,"Full-Name:  %s\n",
	        g.fullname) ;

/* write out the END-OF-HEADER mark */

	bputc(mfp,'\n') ;

	return rs ;
}
/* end subroutine (writeheader) */


