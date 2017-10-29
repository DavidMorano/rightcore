/* checkname */


#define	CF_DEBUG	0


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
 *		J.Mukerji						
		David A.D. Morano
 *									
*									
*	FUNCTIONAL DESCRIPTION:						
*	'Checkname' checks all entries in the table for mailing.	
*									
*	PARAMETERS:							
*	mailaddress -	a string containing a name			
*	flag	 -	an integer specifying the mode in which 	
*			checkname is called
*									
*	RETURNED VALUE:							
*									
*	SUBROUTINES CALLED:						
*									
*	parsename,	checkname					
*									
*	GLOBAL VARIABLES USED:						
*									
*	selectname,	curaddr						
*									
*	CAUTION!!! THIS CODE IS MORE COMPLICATED THAN YOU THINK IT IS!	
*	Lists are processed recursively while some info about them is	
*	saved as side effect in global variable. You MUST BE CAREFUL	
*	about restoring the value of global variables before exiting	
*	a recursive invocation OR ELSE ****				
*									

***********************************************************************/



#include	<string.h>
#include	<pwd.h>
#include	<stdio.h>

#include	<baops.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* external subroutines */

extern int	mkmailname(char *,int,const char *,int) ;

extern struct table	*on_nmalloc() ;

extern char		*parsename() ;


/* external variables */

extern struct global	g ;

extern int		*cl_va ;


/* forward references */

char		*namefromgecos() ;


/* local subroutines */

static int	add_to_translation() ;

static		checknet() ;
static		disambiguate() ;


/* local data */

static struct table	*foundnames[500] ;	/* list of ambiguous names */
static struct table	**foundpt ;



/* check names for validity */

int checkname(mailaddress,flag)
char	*mailaddress ;
int	flag ;
{
	FILE *fopen(), *fp ;

	struct table *aname ;

	int i,j,k,mark ;
	int ngroup ;
	int oflag ;
	int	type ;
	int	f_once ;
	int	f_first ;

	char group[BUFSIZE];		/*JM030185*/
	char s[BUFSIZE] ;
	char ss[BUFSIZE] ;
	char lastname[BUFSIZE] ;
	char	*cp, *cp2 ;


#if	CF_DEBUG
	if (g.debuglevel > 0) {
	    logfile_printf(&g.eh,"checkname: called w/ mode_flag=%d\n",flag) ;

	    logfile_printf(&g.eh,"checkname: called w/ MA=\"%s\"\n",
	        mailaddress) ;

	}
#endif

#if	CF_DEBUG
	if (cl_va != NULL)
	    if (g.debuglevel > 0) logfile_printf(&g.eh,
	        "checkname: v=%d\n",*cl_va) ;
#endif

	mailaddress = parsename(mailaddress) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    logfile_printf(&g.eh,"checkname: selectname=\"%s\"\n",
	        selectname) ;
#endif

#if	CF_DEBUG
	if (cl_va != NULL)
	    if (g.debuglevel > 0) logfile_printf(&g.eh,
	        "checkname: v=%d\n",*cl_va) ;
#endif

	strcpy(ss, selectname) ;

	strcat(ss,":") ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    logfile_printf(&g.eh,"checkname: curaddr=\"%s\"\n",
	        curaddr) ;
#endif

#if	CF_DEBUG
	if (cl_va != NULL)
	    if (g.debuglevel > 0) logfile_printf(&g.eh,
	        "checkname: v=%d\n",*cl_va) ;
#endif

	if (*curaddr != '\0') {

#if	CF_DEBUG
	    if (g.debuglevel > 0) logfile_printf(&g.eh,
	        "checkname: non-null curaddr - checking for mailing list\n") ;
#endif

#if	CF_DEBUG
	    if (cl_va != NULL)
	        if (g.debuglevel > 0) logfile_printf(&g.eh,
	            "checkname: v=%d\n",*cl_va) ;
#endif

	    if ((selectname[0] != ALIAS) &&
	        (selectname[0] != STANLIST) &&
	        (selectname[0] != ALTLIST)) {

#if	CF_DEBUG
	        if (g.debuglevel > 0) logfile_printf(&g.eh,
	            "checkname: did NOT get a mailing list or alias \"%s\"\n",
	            selectname) ;

	        if (g.debuglevel > 0) logfile_printf(&g.eh,
	            "checkname: - checking 'net'\n") ;
#endif

#if	CF_DEBUG
	        if (cl_va != NULL)
	            if (g.debuglevel > 0) logfile_printf(&g.eh,
	                "checkname: v=%d\n",*cl_va) ;
#endif

	        checknet(selectname, curaddr, &mark, &oflag, &flag, ss) ;

#if	CF_DEBUG
	        if (cl_va != NULL)
	            if (g.debuglevel > 0) logfile_printf(&g.eh,
	                "checkname: v=%d\n",*cl_va) ;
#endif

	    } else fprintf(stdout,"%-15s%s\t%s\n", 
	        selectname, curaddr,
	        "list or alias can't have associated address") ;

#if	CF_DEBUG
	    if (g.debuglevel > 0) logfile_printf(&g.eh,
	        "checkname: returning because of non-null curaddr \n") ;
#endif

#if	CF_DEBUG
	    if (cl_va != NULL)
	        if (g.debuglevel > 0) logfile_printf(&g.eh,
	            "checkname: v=%d\n",*cl_va) ;
#endif

	    return mark ;

	} /* end if */

#if	CF_DEBUG
	if (cl_va != NULL)
	    if (g.debuglevel > 0) logfile_printf(&g.eh,
	        "checkname: v=%d\n",*cl_va) ;
#endif

	oflag = 0 ;

/* check for groupname */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    logfile_printf(&g.eh,"checkname: about to check for mailing list\n") ;
#endif

	if ((*mailaddress == STANLIST) || (*mailaddress == ALTLIST)) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        logfile_printf(&g.eh,"checkname: got a mailgroup name=\"%s\"\n",
	            mailaddress) ;
#endif

#if	CF_DEBUG
	    if (cl_va != NULL)
	        if (g.debuglevel > 0) logfile_printf(&g.eh,
	            "checkname: v=%d\n",*cl_va) ;
#endif

	    mailaddress++ ;

/* open mailing list */

/*JM030185*/
	    if ((ngroup = genname(mailaddress,group,sizeof(group))) == 0) {

	        if (flag == 1) fprintf(stdout,
	            "%-15s (** unknown mailing list **)\n",ss) ;

	        else if (flag == 2) fprintf(stdout,
	            "%-15s~%s (** unknown mailing list **)\n",
	            "",mailaddress) ;

	        else if(flag == 0) fprintf(stdout,
	            "warning: '%s' is an unknown mailing list\n",
	            mailaddress) ;

	        return -1 ;

	    } else if (ngroup > 1) {

	        if (flag == 1) fprintf(stdout,
	            "%-15sambiguous mailing list matches:\n%15s(",
	            ss,"") ;

	        else if (flag == 2) fprintf(stdout,
	            "%-15s~%s (ambiguous mailing list matches)\n%15s(",
	            "",mailaddress,"") ;

	        else if (flag == 0) fprintf(stdout,
	            "warning: '%s' is an ambiguous mailing list\n",
	            mailaddress) ;

	        if ((flag == 1) || (flag == 2)) {

	            i = 0 ;
	            while (group[i] != '\0') {

	                if (group[i] == ':')
	                    fprintf(stdout,")\n%15s(","") ;

	                else putc(group[i],stdout) ;

	                i++ ;
	            }
	            fprintf(stdout,")\n") ;
	        }
	        return -1 ;
	    }

	    if ((fp = fopen(group,"r")) == NULL) {

	        fprintf(stdout,
	            "warning: '%s' cannnot be read\n",group) ;

	        return -1 ;
	    }

	    if (flag == 1) fprintf(stdout,
	        "%-15smailing list (%s)\n",ss,group) ;

	    else if (flag == 2) fprintf(stdout,
	        "%-15s~%s mailing list (%s)\n",
	        "",mailaddress,group) ;

#if	CF_DEBUG
	    logfile_printf(&g.eh,"checkname: i'm here 1\n") ;
#endif

	    if (flag == -1) fprintf(stdout,"%s\n",group) ;

/* read the mail list group file */

	    f_first = FALSE ;
	    if (flag > 0) while (fgetline(fp,s,BUFSIZE) > 0) {

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
	            if (strcmp(cp,mailaddress) == 0) continue ;

	        }

	        if (*cp == '\0') continue ;

	        checkname(cp,2) ;

	    }

	    fclose (fp) ;

#if	CF_DEBUG
	    logfile_printf(&g.eh,"checkname: i'm here 2\n") ;
#endif

/* restore state of global variables after recursive call */

	    strcpy(selectname, --mailaddress) ;
	    *curaddr = '\0' ;

	    return 1 ;

	} /* end if (mailing list) */

#if	CF_DEBUG
	if (cl_va != NULL)
	    if (g.debuglevel > 0) logfile_printf(&g.eh,
	        "checkname: v=%d\n",*cl_va) ;
#endif

#if	CF_DEBUG
	logfile_printf(&g.eh,"checkname: i'm here 3\n") ;
#endif

/* translate name if can */
normal:
	mark = 0 ;
	foundpt = foundnames ;
	if (strpbrk(mailaddress,"!@/:%") == NULL) {

#if	CF_DEBUG
	    logfile_printf(&g.eh,"checkname: i'm here 3a\n") ;
#endif

	    f_once = FALSE ;
	    while (! f_once) {

	        f_once = TRUE ;

/* look in the user's local name translation database */

#if	CF_DEBUG
	        logfile_printf(&g.eh,"checking user's translations\n") ;
#endif

	        type = TT_USER ;
	        i = 0 ;
	        while (TRUE) {

#if	CF_DEBUG
	            logfile_printf(&g.eh,"checkname: i'm here 3b\n") ;
#endif

	            j = findname(mailaddress,&i,type) ;

	            if (j < 0) break ;

	            aname = name[i] ;
	            if (aname->mailaddress[0] == ALIAS) k = 1 ;

	            else k = 0 ;

/* expand to network address for all addresses, even aliases.  PAS 2/15/85 */

/* save the alias for later use	PAS 2/25/85 */

/*		if ( k == 0 ) mark++;*/

#if	CF_DEBUG
	            logfile_printf(&g.eh,"checkname: i'm here 3c\n") ;
#endif

	            mark++; 	/* set mark even if alias PAS-JM 2/12/85 */
	            sprintf(lastname,"(%s)",&aname->mailaddress[k]) ;

#if	CF_DEBUG
	            logfile_printf(&g.eh,"checkname: i'm here 3d\n") ;
#endif

	            if (strncmp(aname->mailaddress, "NO", 2 ) == 0 ) {

	                fprintf(stdout,"%-15s	(unknown address)\n",
	                    aname->realname) ;

	                error++ ;

	            } else if (flag == 1) {

	                fprintf(stdout,"%-15s%-20s%-30s\n",
	                    ss,aname->realname,lastname) ;

	                *ss = '\0' ;

	            } else if (flag > 0) {

	                fprintf(stdout,"%-15s%-20s%-30s",
	                    "",aname->realname,lastname) ;

	                fprintf(stdout,"\n") ;

	            } else if (flag == -1)
	                fprintf(stdout,"%s\n",aname->realname) ;

	            if (mark < 20) *foundpt++ = aname ;

	            if (j == 0) break ;

	            i += 1 ;

	        } /* end while */

	        if (mark != 0) break ;

/* check for a local username FIRST !! */

#if	CF_DEBUG
	        logfile_printf(&g.eh,"checking \'%s\" for a user name\n",
	            mailaddress) ;
#endif

	        if ((mark = lookup_passwd(mailaddress,flag)) != 0) break ;

/* convert any "all initials" (format 'd.a.m.') name into the old format */

	        cp = mailaddress ;
	        if ((cp[1] == '.') && (cp[3] == '.')) {

	            if (cp[4] == '\0') {

	                cp[1] = cp[2] ;
	                cp[2] = '\0' ;

	            } else if ((cp[5] == '.') && (cp[6] == '\0')) {

	                cp[1] = cp[2] ;
	                cp[2] = cp[4] ;
	                cp[3] = '\0' ;

	            }
	        }

/* look in the system-wide name translation database */

#if	CF_DEBUG
	        logfile_printf(&g.eh,"checking \"%s\" for a system name\n",
	            mailaddress) ;
#endif

	        type = TT_SYSTEM ;
	        i = 0 ;
	        while (1) {

#if	CF_DEBUG
	            logfile_printf(&g.eh,"checkname: i'm here 3b\n") ;
#endif

	            j = findname(mailaddress,&i,type) ;

	            if (j < 0) break ;

#if	CF_DEBUG
	            logfile_printf(&g.eh,"found a match\n") ;
#endif

	            aname = name[i] ;
	            if (aname->mailaddress[0] == ALIAS) k = 1 ;

	            else k = 0 ;

/* expand to network address for all addresses, even aliases.  PAS 2/15/85 */

/* Save the alias for later use	PAS 2/25/85 */

/*		if ( k == 0 ) mark++;*/

#if	CF_DEBUG
	            logfile_printf(&g.eh,"checkname: i'm here 3c\n") ;
#endif

	            mark++; 	/* set mark even if alias PAS-JM 2/12/85 */
	            sprintf(lastname,"(%s)",&aname->mailaddress[k]) ;

#if	CF_DEBUG
	            logfile_printf(&g.eh,"checkname: i'm here 3d\n") ;
#endif

	            if (strncmp(aname->mailaddress, "NO", 2 ) == 0 ) {

	                fprintf(stdout,"%-15s	(unknown address)\n",
	                    aname->realname) ;

	                error++ ;

	            } else if (flag == 1) {

	                fprintf(stdout,"%-15s%-20s%-30s\n",
	                    ss,aname->realname,lastname) ;

	                *ss = '\0' ;

	            } else if (flag > 0) {

	                fprintf(stdout,"%-15s%-20s%-30s",
	                    "",aname->realname,lastname) ;

	                fprintf(stdout,"\n") ;

	            } else if (flag == -1)
	                fprintf(stdout,"%s\n",aname->realname) ;

	            if (mark < 20) *foundpt++ = aname ;

	            if (j == 0) break ;

	            i += 1 ;

	        } /* end while */

	        if (mark != 0) break ;

#ifdef	LOOKUP_POST
/* check the corporate POST database */

	        mark = lookup_post(mailaddress,buf) ;

	        if (mark > 0) break ;
#endif

#if	CF_DEBUG
	        logfile_printf(&g.eh,"checking for a user name again\n") ;
#endif

	        mark = lookup_passwd(mailaddress,flag) ;

	    } /* end while */

#if	CF_DEBUG
	    logfile_printf(&g.eh,"checkname: i'm here 3ea\n") ;
#endif

	} /* end if (bare name lookup) */

#if	CF_DEBUG
	logfile_printf(&g.eh,"checkname: i'm here 3eb\n") ;
#endif

	if (mark == 0) {

#if	CF_DEBUG
	    logfile_printf(&g.eh,"checkname: i'm here 4\n") ;
#endif

	    checknet( "", mailaddress, &mark, &oflag, &flag, ss) ;

	}

#if	CF_DEBUG
	logfile_printf(&g.eh,"checkname: i'm here 4a\n") ;
#endif

	if (mark == 0 && flag == 1 && oflag == 0)
	    fprintf(stdout,"%-15sunknown name\n",ss) ;

	else if (mark == 0 && flag == 2) fprintf(stdout,
	    "%-15s%-20s%-30s\n",
	    "",mailaddress,"(** unknown name **)") ;

	else if (mark == 0 && flag == 0) fprintf(stdout,
	    "warning: '%s' is an unknown name\n",mailaddress) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"checkname: i'm here 5\n") ;
#endif

	if ((flag == 0) && (mark > 1) && g.f.interactive) {

	    fprintf(stdout,
	        "\nwarning: '%s' is an ambiguous name\n",mailaddress) ;

	    disambiguate(&mark) ;

	}

	return mark ;
}
/* end subroutine (checkname) */


/* 
 * checknet() checks the validity of an absolute address and passes judgement
 * on it through the parameters mark, flag and oflag
 *
 * This is where code should be added to tie in any feedback from any delivery
 * system.
*/

static checknet( name, addr, mark, oflag, flag, ss )
char *name, *addr, *ss ;
int *mark, *oflag, *flag ;
{
	char	*netaddr ;
	char	lastname[SYSLEN + 1] ;
	char	locaddr[SYSLEN + 1] ;
	char	*mailaddressj = NULL ;
	char	fixedbuf[BUFSIZE + 1] ;


	*mark = 1 ;
	if (strpbrk( addr, "@:!./%" ) == NULL ) {

/* this is probably a local login name try it as such */

	    if ((mailaddressj = namefromgecos(addr,fixedbuf)) == NULL)
	        netaddr = "NOUSER" ;

	    else {
	        sprintf(locaddr, "!%s", addr) ;

	        netaddr = locaddr ;
	        ss = mailaddressj ;
	    }

	} else {

/* Here is where tie in with the delivery system for
	    address verification should go */

	    netaddr = addr ;
	}

	sprintf(lastname,"(%s)",netaddr) ;

	if (!strcmp(netaddr,"NOMAIL")) {

	    if (*flag == 1)
	        fprintf(stdout,"%-15s%-20s\n",
	            ss," unknown address") ;

	    *mark = 0 ;
	    *oflag = 1 ;

	} else if (!strcmp(netaddr,"NOUSER")) {

	    if (*flag == 1) fprintf(stdout,"%-15s%-20s\n",
	        ss," unknown user") ;

	    *mark = 0 ;
	    *oflag = 1 ;

	} else if (!strcmp(netaddr,"NODOM")) {

	    if (*flag == 1) fprintf(stdout,"%-15s%-20s\n",
	        ss," unknown domain and address") ;

	    *mark = 0 ;
	    *oflag = 1 ;

	} else if ( *flag == 1 ) fprintf(stdout,"%-15s%-20s%-30s\n",
	    ss,name,lastname) ;

	else if ( *flag > 0 ) {

	    fprintf(stdout,"%-15s%-20s%-30s",
	        "", addr , lastname) ;

	    fprintf(stdout,"\n") ;

	}

	if (*mark && *curaddr != '\0') {

	    strcpy( curaddr, netaddr ) ;

	    *realto = '\0' ;
	    ambiguous = 1 ;
	}
}
/* end subroutine (checknet) */


/*
 * disambiguate() is an integral part of checkname. It should not be moved 
 * out of this file. disambiguate first prints out a list of names that 
 * matched the given name and then asks the user to choose one. It then 
 * returns the chosen name to the user in the global variable mailaddress
 * it also returns 1 if a name is selcted and 0 if none is selcted in
 * the paramater *mark
*/

static disambiguate(markp)
int	*markp ;
{
	struct table **iptr ;

	int  i, choice, k;	/* New variable k for handling aliases.
				 * This function wasn't being called when
				 * ambiguous name included an alias. The
				 * ambiguity wasn't even being flagged.
				 * Now that it is being flagged, it must be
				 * handled here. - PAS-JM 2/12/85
										 */
	char answer[20] ;


	printf("it matched the following names:\n") ;

again:
	iptr = foundnames ;
	i = 1 ;
	while (iptr < foundpt) {

	    k = 0;					/* PAS-JM 2/12/85 */
	    if ((*iptr)->mailaddress[0] == ALIAS)
	        k = 1 ;				/* PAS-JM 2/12/85 */

	    printf("%2d\t%15s\t%15s\n", i, (*iptr)->realname, 
	        (*iptr)->mailaddress + k) ; /* PAS-JM 2/12/85 */

	    iptr++ ;
	    i++ ;
	}

getans:
	printf("\nPlease select one (? for help, ") ;

	for ( i=1; i <= *markp ; i++) printf("%2d, ", i) ;

	printf(" none) [none]: ") ;

	fgets(answer,20,stdin) ;

	if ((*answer == '\0' ) || (*answer == 'n')) {

/* none selected so return "" in selectname */

	    printf("\nNone selected\n") ;

	    strcpy( selectname, "  " ) ;

	    *curaddr = '\0' ;
	    *markp = 0 ;
	    ambiguous = 1 ;
	    return ;
	}

	if ((*answer == '?' ) || (sscanf(answer,"%d",&choice) == 0)) {

	    help(6) ;

	    printf("\n") ;

	    goto again ;
	}

	if ((choice > *markp) || (choice < 1)) {

	    printf("%d is out of range\n",choice) ;

	    goto getans ;
	}

	choice-- ;
	k = 0;						/* PAS-JM 2/12/85 */
	if (foundnames[choice]->mailaddress[0] == ALIAS)
	    k = 1 ; /* PAS-JM 2/12/85 */

	printf("the name you selected is: %s <%s>\n",
	    foundnames[choice]->realname, foundnames[choice]->mailaddress + k) ;

/* PAS-JM 2/12/85 */

	printf("is that the one you wanted ? (yes, no) [yes] : ") ;

	fgets(answer, 20, stdin) ;

	if ( *answer == 'n' ) goto again ;

	if ( strncmp( foundnames[choice]->mailaddress + k, "NO", 2 ) == 0 ) {

	    printf(
	        "\nthe name that you have chosen has an unknown address") ;

	    printf(
	        "\nplease make another selection\n") ;

	    goto again ;
	}

	strcpy(selectname, foundnames[choice]->realname) ;

/* PAS-JM 2/12/85 */

	strcpy(curaddr, foundnames[choice]->mailaddress + k) ;

	error = 0;					/* JM 2/22/85 */
	ambiguous = *markp = 1 ;
}
/* end subroutine (disambiguate) */


int lookup_passwd(username,flag)
char	*username ;
int	flag ;
{
	int	nmatch = 0 ;

	char	gecosnamebuf[200] ;
	char	mailnamebuf[200] ;


#if	CF_DEBUG
	logfile_printf(&g.eh,"you are here in 'lookup_passwd' 1\n") ;
#endif

	if (mkgetgecos(gecosnamebuf,200,username) < 0) 
		return 0 ;

	if (mkmailname(mailnamebuf,200,gecosnamebuf,-1) < 0) 
		return 0 ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"you are here in 'lookup_passwd' 3\n") ;
#endif

	nmatch += add_to_translation(mailnamebuf,username,flag) ;

#if	CF_DEBUG
	logfile_printf(&g.eh,"you are here in 'lookup_passwd' 4\n") ;
#endif

	return nmatch ;
}
/* end subroutine (lookup_passwd) */


char *namefromgecos(username,fixedname)
char	*username ;
char	fixedname[] ;
{


#ifndef	COMMENT
	struct passwd	*pwent ;

	int i ;

	char *c, *d, *t ;


	for (i = 0; i < 30; i++)
	    fixedname[i] = '\0' ;

	if ((pwent = getpwnam(username)) == NULL) return NULL ;

	c = pwent->pw_gecos ;
	d = fixedname ;
	if ((t = strchr(c,'-')) != NULL) {

	    c = t += 1 ;
	    if ((t = strchr(c,0x28)) != NULL) *t = '\0' ;

	}

	while (*c != '\0') *d++ = *c++ ;
#else
	fixedname[0] = '\0' ;
	getgecos(username,fixedname,BUFSIZE) ;
#endif

	return fixedname ;
}
/* end subroutine (namefromgecos) */


static int add_to_translation(realname,mailaddress,flag )
char	*realname, *mailaddress ;
int	flag ;
{
	struct table	*tname ;

	char		locadr[300] ;


/* check for no space left in translation table */

	if ((tname = on_nmalloc()) == NULL) return 0 ;

/*
	save the name in the entry and put a pointer
	to the entry in the 'foundnames' list ;
	skip over the name that went in as input 
*/

	strcpy(tname->realname,realname) ;

	strcpy(tname->mailaddress,mailaddress) ;

	sprintf(locadr,"(%s)",mailaddress) ;

	if (flag == 1) {

	    fprintf(stdout,"%-15s%-20s%-30s[LOCAL]\n",
	        realname,tname->realname,locadr) ;

	} else if (flag > 0) 
		fprintf(stdout,"%-15s%-20s%-30s%s\n",
	    "",tname->realname,locadr,verbose?"[LOCAL]":"") ;

	else if (flag == -1) 
		fprintf(stdout,"%s\n",tname->realname) ;

	strcpy(selectname,tname->realname) ;

	strcpy(curaddr,tname->mailaddress) ;

	ambiguous = 1 ;
	return 1 ;
}
/* end subroutine (add_to_translation) */


