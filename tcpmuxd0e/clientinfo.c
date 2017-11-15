/* clientinfo */

/* manage client information (a little bit) */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 2008-06-23, David A­D­ Morano
        I updated this subroutine to just poll for machine status and write the
        Machine Status (MS) file. This was a cheap excuse for not writing a
        whole new daemon program just to poll for machine status. I hope this
        works out! :-)

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module (not really an object) manages some of the client data. This
        data is stored in a structure 'clientinfo'.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<sockaddress.h>
#include	<connection.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"config.h"
#include	"clientinfo.h"


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snddd(char *,int,uint,uint) ;
extern int	snsdd(char *,int,const char *,uint) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	dupup(int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	nlspeername(const char *,const char *,char *) ;
extern int	isascoket(int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	clientinfo_load(CLIENTINFO *,const char *,vecstr *) ;


/* local variables */


/* exported subroutines */


int clientinfo_start(CLIENTINFO *cip)
{
	int		rs ;

	if (cip == NULL) return SR_FAULT ;

	memset(cip,0,sizeof(CLIENTINFO)) ;
	cip->nnames = -1 ;

	rs = vecstr_start(&cip->stores,1,0) ;

	return rs ;
}
/* end subroutine (clientinfo_start) */


int clientinfo_finish(CLIENTINFO *cip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (cip == NULL) return SR_FAULT ;

	if (cip->fd_input >= 0) {
	    u_close(cip->fd_input) ;
	    cip->fd_input = -1 ;
	}

	if (cip->fd_output >= 0) {
	    u_close(cip->fd_output) ;
	    cip->fd_output = -1 ;
	}

	if (cip->nnames >= 0) {
	    cip->nnames = 0 ;
	    rs1 = vecstr_finish(&cip->names) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = vecstr_finish(&cip->stores) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (clientinfo_finish) */


int clientinfo_loadnames(CLIENTINFO *cip,cchar *dname)
{
	int		rs = SR_OK ;
	int		opts = VECSTR_OCOMPACT ;

	if (cip == NULL) return SR_FAULT ;

	if (cip->nnames < 0) {
	    cip->nnames = 0 ;
	    rs = vecstr_start(&cip->names,4,opts) ;
	}

	if (rs >= 0) {
	    rs = clientinfo_load(cip,dname,&cip->names) ;
	}

	return rs ;
}
/* end subroutine (clientinfo_loadnames) */


/* local subroutines */


static int clientinfo_load(CLIENTINFO *cip,cchar *dname,vecstr *nlp)
{
	CONNECTION	conn, *cnp = &conn ;
	int		rs ;
	int		rs1 = 0 ;
	int		c = 0 ;

	if (cip == NULL) return SR_FAULT ;
	if (nlp == NULL) return SR_FAULT ;

/* use 'connection(3dam)' */

	if ((rs = connection_start(cnp,dname)) >= 0) {
	   char		hostname[MAXHOSTNAMELEN + 1] ;
 
	    if (cip->salen > 0) {
	        SOCKADDRESS	*sap = &cip->sa ;
	        int		sal = cip->salen ;
	        rs1 = connection_peername(cnp,sap,sal,hostname) ;
	    } else {
	        const int	ifd = cip->fd_input ;
	        rs1 = connection_sockpeername(cnp,hostname,ifd) ;
	    }

	    if (rs1 >= 0) {
	        rs1 = connection_mknames(&conn,nlp) ;
	        if (rs1 >= 0) 
		    c += rs1 ;
	    }

	    if (rs1 >= 0) {
	        rs1 = vecstr_adduniq(nlp,hostname,-1) ;
	        if ((rs1 >= 0) && (rs1 < INT_MAX))
		    c += 1 ;
	    }

	    rs1 = connection_finish(&conn) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (connection_start) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutines (clientinfo_load) */


