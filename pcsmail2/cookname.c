/* cookname */

/* convert real names into user names */


#define	CF_DEBUGS	0		/* compile-time debugging */


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
		David A­D­ Morano
 *		J.Mukerji						
 *		A.M.Toto						
 *									
*									
*									
*	FUNCTIONAL DESCRIPTION:						
*	'Cookname' marks entries in the table for mailing.		
*	It also sorts the recipients into destination mailbags.	(AMT)	
*									
*	PARAMETERS:							
*									
*	RETURNED VALUE:							
*									
*	SUBROUTINES CALLED:						
*									

************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<pwd.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<estrings.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<pcsconf.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN + 12)
#endif


/* external subroutines */

extern struct table	*on_nmalloc() ;

extern char		*parsename() ;


/* external variables */

extern struct global	g ;

extern struct userinfo	u ;

extern struct pcsconf	p ;


/* exported subroutines */


int cookname(realname,flag)
char	*realname ;
int	flag ;
{
	FILE		*fopen(), *fp ;

	struct table	*tname, *aname ;
	struct table	*mname ;

	int	i, k ;		/* Add k for aliases PAS-JM 2/14/85 */
	int	f_first ;

	char	group[BUFSIZE + 1] ;
	char	s[BUFSIZE + 1] ;
	char	*cp, *cp2 ;


#if	CF_DEBUGS
	if (g.debuglevel > 0)
	    logfile_printf(&g.eh,"cookname: entered w/ name=\"%s\" \n",
	        realname) ;
#endif

	realname = parsename(realname) ;

	if (*curaddr != '\0') {

/* address is given with the name so use it */
/* but first enter the name in the translation table */

	    if ((tname = on_nmalloc()) == NULL) {

	        fprintf(stderr,"translation table too large\n") ;

		return BAD ;
	    }

	    strcpy(tname->realname,selectname) ;

	    strcpy(tname->mailaddress, curaddr) ;

#if	CF_DEBUGS
	    if (g.debuglevel > 0)
	        logfile_printf(&g.eh,"cookname: current name given \n") ;
#endif

	    put_in_mailbag( tname, flag ) ;

	    return OK ;
	}

/* check for groupname */

	if ((*realname == STANLIST) || (*realname == ALTLIST)) {

	    realname++ ;

/* open mailing list */

	    if (genname(realname,group,sizeof(group)) != 1) return OK ;

	    if ((fp = fopen(group,"r")) == NULL) return OK ;

	    f_first = FALSE ;
	    while (fgetline(fp,s,BUFSIZE) > 0)  {

/* skip over leading white space */

	        cp = s ;
	        while ((*cp == ' ') || (*cp == '\t')) cp += 1 ;

/* ignore comment lines */

	        if (*cp == '#') continue ;

/* read up to some more white space or the end */

	        if ((cp2 = strpbrk(cp," \t\\\n")) != ((char *) 0))
	            *cp2 = '\0' ;

/* is it a UPAS first line ? */

	        if (! f_first) {

	            f_first = TRUE ;
	            if (strcmp(cp,realname) == 0) continue ;

	        }

	        if (*cp == '\0') continue ;

	        cookname(cp,flag + 10) ;

	    }

	    fclose(fp) ;

/* else check for -me */

	} else if (strcmp(realname,"-me") == 0) {

/*PAS-JM 2/8/85*/

	    if (myname < 0) {

/* assume it is a login name */

	        if ((tname = on_nmalloc()) == NULL) {

	            fprintf(stderr,"translation table too large\n") ;

			return BAD ;
	        }

	        strcpy(tname->realname,u.mailname) ;

	        strcpy(tname->mailaddress,u.username) ;

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"cookname: login name assumed\n") ;
#endif

	        put_in_mailbag( tname, flag ) ;

	    } else {

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"cookname: local translation ?\n") ;
#endif

	        mname = name[myname] ;
	        put_in_mailbag( mname, flag ) ;

	    }

	} else { /* translate name if can */

#if	CF_DEBUGS
	    if (g.debuglevel > 0)
	        logfile_printf(&g.eh,
			"cookname: translate if we can \"%s\"\n",realname) ;
#endif

	    i = 0 ;

/* there used to be a while loop here to take care of ambiguities */
/* but now that checkname guarantees that no ambiguous name get   */
/* through to here we can safely dispose of the loop and save    */
/* time and memory!						  */

	    if (findname(realname,&i,TT_ALL) >= 0) {

	        aname = name[i] ;

/* Make sure that aliases are handled right PAS-JM 2/14/85 */

	        if (aname->mailaddress[0] == ALIAS) k = 1 ;

	        else k = 0 ;

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"cookname: found translated ema=\"%s\"\n",
	                aname->mailaddress) ;
#endif

	        put_in_mailbag(aname,flag) ;

	    } else if ((flag < 10) || (strpbrk(realname,"@!/:%") != NULL)) {

/* if no match, add to end of table ; assume it is a login name */

	        if ((tname = on_nmalloc()) == NULL) {

	            fprintf(stderr,
	                "translation table too large\n") ;

			return BAD ;
	        }

	        strcpy(tname->realname,realname) ;

/* take care of the !login form */

	        if (*realname == '!') {

	            realname++ ;
	        }

/* now try to find a path to the site of the user */
/* and save that as the address of the user	  */

	        strcpy(tname->mailaddress,realname) ;

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"cookname: path to user ema=\"%s\"\n",
	                tname->mailaddress) ;
#endif

	        put_in_mailbag(tname, flag) ;

	    }
	}

	return OK ;
}
/* end subroutine (cookname) */


/* 
 * put_in_mailbag( aname, flag ) sorts the addressees into mailbags
 * based on the immediate neighbor to which they are to be sent. This 
 * uses the 'bld_link' function written by A.M.T.
*/

int put_in_mailbag(aname,flag)
struct table	*aname ;
int		flag ;
{
	int	i ;
	int	sdlen = -1, hlen = -1, ulen = -1, dlen = -1 ;
	int	f_companydomain = FALSE ;
	int	f_domainname = FALSE ;
	int	f_straightuser = FALSE ;

	char	sort[MAXPATHLEN + 1] ;
	char	address[MAXPATHLEN + 1], *up = NULL, *hp = NULL, *dp = NULL ;
	char	*cp, *mp ;


/* message destined for special mailbox like bb or netnews */

	if (flag == SPECIAL) {

	    aname->mail = 1 ;
	    strcpy(sort,"SPECIAL") ;

	} else {	/* normal mail message */

	    if ((aname->mail == 0) || (flag < aname->mail))
	        aname->mail = flag ;

/* now figure out who the immediate neighbour is     */

#if	CF_DEBUGS
	    if (g.debuglevel > 0)
	        logfile_printf(&g.eh,"putinmbag: normal mail address \"%s\"\n",
	            aname->mailaddress) ;
#endif

	    mp = aname->mailaddress ;

/* first remove leading alias marks and bangs if any */

	    if ((*mp == ALIAS) || (*mp == '!')) {

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"putinmbag: found an alias \"%s\"\n",
	                aname->mailaddress) ;
#endif

#ifdef	COMMENT
	        strcpy(aname->mailaddress, &aname->mailaddress[1]) ;
#else
	        for (i = 1 ; aname->mailaddress[i] != '\0' ; i += 1)
	            aname->mailaddress[i - 1] = aname->mailaddress[i] ;

	        aname->mailaddress[i - 1] = '\0' ;
#endif
	    }

/* are we dealing with a local mailaddress ? */

	    if (strpbrk(mp,"/=% \t()<>\":;,") != NULL) {

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"putinmbag: totally unknown\n") ;
#endif

	        strcpy(sort,"UNKNOWN") ;

	    } else if ((strpbrk(mp,"@!") == NULL) &&
	        (getpwnam(mp) != NULL)) {

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"putinmbag: easy local \n") ;
#endif

	        strcpy(sort,"LOCAL") ;

	    } else {

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"putinmbag: harder stuff\n") ;
#endif

	        strwcpy(address,mp,MAXPATHLEN) ;

	        strcpy(sort,"UNKNOWN") ;

/* break into host and user parts */

		hp = "" ;
	        up = address ;
	        if ((cp = strchr(address,'@')) != NULL) {

#if	CF_DEBUGS
	            if (g.debuglevel > 0)
	                logfile_printf(&g.eh,"putinmbag: at sign fixup\n") ;
#endif

	            up = address ;
	            ulen = cp - address ;
	            hp = cp + 1 ;
	            *cp = '\0' ;

	        } else if ((cp = strchr(address,'!')) != NULL) {

#if	CF_DEBUGS
	            if (g.debuglevel > 0)
	                logfile_printf(&g.eh,"putinmbag: bang sign fixup\n") ;
#endif

	            hlen = cp - address ;
	            *cp = '\0' ;
	            hp = address ;
	            up = cp + 1 ;

	        }

	        if (*hp == '.') hp += 1 ;

/* first check if we are in the AT&T domain */

#if	CF_DEBUGS
	            if (g.debuglevel > 0)
	                logfile_printf(&g.eh,"putinmbag: company domain check\n") ;
#endif

	        if (strcmp(hp,p.orgdomain) == 0) {

#if	CF_DEBUGS
	            if (g.debuglevel > 0)
	                logfile_printf(&g.eh,"putinmbag: straight AT&T domain\n") ;
#endif

	            f_companydomain = TRUE ;
	            dp = hp ;
	            hp = NULL ;
	            hlen = 0 ;

	        } else {

#if	CF_DEBUGS
	            if (g.debuglevel > 0)
	                logfile_printf(&g.eh,"putinmbag: general domain search\n") ;
#endif

/* is there a domain ? */

	            if ((cp = strchr(hp,'.')) != NULL) {

#if	CF_DEBUGS
	                if (g.debuglevel > 0)
	                    logfile_printf(&g.eh,
				"putinmbag: break the address up\n") ;
#endif

	                *cp = '\0' ;
	                hlen = cp - hp ;
	                if (cp[1] != '\0') dp = cp + 1 ;

	            }

	            if (dp != NULL) dlen = strlen(dp) ;

/* check again if we are in the AT&T domain */

#if	CF_DEBUGS
	            if (g.debuglevel > 0) logfile_printf(&g.eh,
			"putinmbag: about to check for AT&T again\n") ;
#endif

/* put any SubDomain length in variable 'sdlen' */

	            sdlen = 0 ;
	            if ((dlen > 0) &&
	                ((i = substring(dp,dlen,p.orgdomain)) >= 0) &&
	                (dlen == (i + strlen(p.orgdomain)))) {

#if	CF_DEBUGS
	                if (g.debuglevel > 0)
	                    logfile_printf(&g.eh,"putinmbag: got an AT&T string\n") ;
#endif

	                if (i == 0) 
				f_companydomain = TRUE ;

	                else if (dp[i - 1] == '.') {

	                    f_companydomain = TRUE ;
	                    sdlen = i - 1 ;
	                }
	            }

#if	CF_DEBUGS
	            if (g.debuglevel > 0)
	                logfile_printf(&g.eh,
				"putinmbag: AT&T check resulted in %d\n",
	                    f_companydomain) ;
#endif

	        } /* end if */

/* remember that there may not be ANY host at this point !!! */

	        if (dp != NULL) dlen = strlen(dp) ;

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,
			"putinmbag: about to make some booleans\n") ;
#endif

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"putinmbag: native domain \"%s\"\n",
			u.domainname) ;
#endif

#if	CF_DEBUGS
	        if (g.debuglevel > 0) {

	            if (dp != NULL)
	                logfile_printf(&g.eh,"putinmbag: domain=\"%s\"\n",dp) ;

	            else
	                logfile_printf(&g.eh,
				"putinmbag: no domain extracted\n",dp) ;

	        }
#endif

#if	CF_DEBUGS
	        if (g.debuglevel > 0) {

	            logfile_printf(&g.eh,"putinmbag: la=%d\n",
	                (f_companydomain && (sdlen == 0) && (hp != NULL))) ;

	            logfile_printf(&g.eh,"putinmbag: lb=%d\n",
	                ((dp != NULL) && (strcmp(dp,u.domainname) == 0))) ;

	            logfile_printf(&g.eh,"putinmbag: lc=%d\n",
	                (dlen <= 0)) ;

	        }
#endif

	        f_domainname = 
	            ((dp != NULL) && (strcmp(dp,u.domainname) == 0)) ;

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"putinmbag: sdlen=%d f_cd=%d f_ld=%d\n",
	                sdlen,f_companydomain,f_domainname) ;
#endif

	        f_straightuser = (strpbrk(up,".! \"") == 0) ;

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"putinmbag: straightuser=%d\n",
	                f_straightuser) ;
#endif

/* look for the local mailhost (IF WE EVEN HAVE A HOST in 'hp' variable) */

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,
			"putinmbag: about to check for hard local\n") ;
#endif

	        if ((hp != NULL) &&
			((strcmp(hp,u.nodename) == 0) ||
	            ((strcmp(hp,p.mailnode) == 0) && 
			(strcmp(u.domainname,p.maildomain) == 0)) ||
	            (strcmp(hp,p.mailhost) == 0))) {

#if	CF_DEBUGS
	            if (g.debuglevel > 0)
	                logfile_printf(&g.eh,
			"putinmbag: intermediate local check\n") ;
#endif

	            if (f_straightuser && (getpwnam(up) != NULL)) {

#if	CF_DEBUGS
	                if (g.debuglevel > 0)
	                    logfile_printf(&g.eh,"putinmbag: found a hard local\n") ;
#endif

	                strcpy(sort,"LOCAL") ;

	                strcpy(mp,up) ;

	            }

	        } else if (f_companydomain && (hp != NULL)) {

#if	CF_DEBUGS
	            if (g.debuglevel > 0)
	                logfile_printf(&g.eh,
			"putinmbag: miscellaneous AT&T check \n") ;
#endif

	            if (sdlen > 0)
	                bufprintf(sort,MAXPATHLEN,
				"%s.%t",hp,dp,sdlen) ;

	            else
	                bufprintf(sort,MAXPATHLEN,
				"%s",hp) ;

	            bufprintf(mp,BUFLEN,
			"%s.%s!%s",hp,dp,up) ;

	        }

#if	CF_DEBUGS
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"putinmbag: done w/ hard checks\n") ;
#endif

	    } /* end if */

	} /* end if (normal mail address) */

#if	CF_DEBUGS
	if (g.debuglevel > 0)
	    logfile_printf(&g.eh,"putinmbag: name=\"%s\" ema=\"%s\" mail=%d\n",
	        aname->realname,aname->mailaddress,aname->mail) ;
#endif

	bld_link(aname,sort) ;

}
/* end subroutine (put_in_mbag) */



