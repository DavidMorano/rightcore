/* process */


#define	CF_DEBUG	1


/************************************************************************

	= David A­D­ Morano, 94/01/06
	This subroutine was adopted from the 'main'
	subroutine of the old PCSMAIL/SENDMAIL(PCS) program.


************************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<errno.h>
#include	<unistd.h>
#include	<string.h>
#include	<signal.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<logfile.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<userinfo.h>
#include	<pcsconf.h>
#include	<vecstr.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"prompt.h"
#include	"header.h"



/* local subroutine defines */

#define		MAXARGINDEX	100
#define		MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern char	*putheap() ;
extern char	*timestr_log() ;


/* external data */

extern struct global	g ;

extern struct pcsconf	p ;

extern struct userinfo	u ;


/* global data */


/* forward references */


/* local static data */





int process(rlp)
vecstr	*rlp ;
{
	bfile	msgfile, *mfp = &msgfile ;
	bfile	tmpfile, *tfp = &tmpfile ;
	bfile	*fpa[3] ;

	struct	stat	sb ;

	struct tm	*timep ;

	struct group	*gp ;

	struct address	*as_to = NULL ;
	struct address	*as_from = NULL ;
	struct address	*as_sender = NULL ;
	struct address	*as_replyto = NULL ;
	struct address	*as_cc = NULL ;
	struct address	*as_bcc = NULL ;

	offset_t		offset ;

	int		len, rs, status ;
	int		wstatus ;
	int		i, l ;
	int		pid_child ;

	char	argpresent[MAXARGGROUPS] ;
	char	pcsbuf[PCSCONF_LEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char		buf[BUFSIZE + 1], *bp ;
	char		linebuf[LINELEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char		*option, *tmp ;
	char		*namep, *cp, *cp2 ;
	char		*realname ;


	ex_mode = EM_PCSMAIL ;

/* initialize strings to ""*/

	*mess_id = *from = *sentby = *fromaddr = *reference = '\0' ;
	*keys = *subject = *moptions = *copyto = *bcopyto = *appfile = '\0' ;
	*received = *forwfile = *eforwfile = *retpath = *message = '\0' ;


	strcpy(mess_id,g.messageid) ;


/* continue with the rest of the program */

	iswait = FALSE ;
	isfile = FALSE ;		/* CAUTION: MUST be initialized here */
	isedit = TRUE ;
	isforward = FALSE ;
	iseforward = FALSE ;
	isappend = FALSE ;


	isedit = g.f.edit ;
	if (! g.f.interactive) isedit = FALSE ;

	iseforward = (g.arg_msgfname != NULL) ;
	isfile = iseforward ;


/* clean up recipient list */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: recipient so far :\n%s\n",recipient) ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: isfile=%d\n",isfile) ;
#endif

	tonames = getnames(recipient) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: tonames %d \n", tonames) ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: isfile=%d\n",isfile) ;
#endif

	ccnames = getnames(copyto) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: isfile=%d\n",isfile) ;
#endif

	bccnames = getnames(bcopyto) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: isfile=%d\n",isfile) ;
#endif

/* generate name for temporary message file */

	strcpy(tempfile,"/tmp/smailXXXXXX") ;   /* unique gensym name */

	mktemp(tempfile) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to create temporary message file\n") ;
#endif

/* create file for temporarily putting the message in */

	if ((rs = bopen(mfp,tempfile,"wct",0600)) < 0) {

	    bprintf(g.efp,
	        "%s: can't open necessary temporary file - system problem\n",
	        g.progname) ;

	    logfile_printf(&g.lh,"couldn't open TMP file \"%s\"\n",tempfile) ;

	    goto badret ;
	}

#ifdef	COMMENT

/* set group ownership */
/* to mailgroup, so that mail program can read it		*/
/* set protection to rw-r----- so that	*/
/* the owner can read/write and mail can only read		*/

	chown(tempfile, g.uid,g.gid_mail) ;

	chmod(tempfile,0640) ;

#endif

/* miscellaneous variables */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: misc variables isfile=%d\n",isfile) ;
#endif


#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to go and get names ? isfile=%d\n",
	        isfile) ;
#endif

/* go */

	if (isfile) {

#if	CF_DEBUG
	    if (g.f.debug) {

	        debugprintf("main: 'isfile' is ON \"%s\"\n",filename) ;

	        fileinfo(filename,"main got a ISFILE") ;

	    }
#endif

	    if (access(filename,R_OK) < 0) {

	        logfile_printf(&g.eh,"main: could not access FILENAME \"%s\"\n",
	            filename) ;

	    }

	    getheader(filename,u.mailname) ;

	    tonames = getnames(recipient) ;

	    bccnames = getnames(bcopyto) ;

	    ccnames = getnames(copyto) ;

/* prompt 5/2/85 (JM)		*/

/* if we have a return path, then we do not include the rest of the file */

#ifdef	COMMENT
	    if (isret) {

	        isfile = 0 ;	/* added to force reply message */
	        logfile_printf(&g.eh,"main: turned off 'isfile'\n") ;

	    }
#endif

	} /* end if (we have a file) */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to get the message contents\n") ;
#endif

/* the message itself */

	prompt(ALL) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to put the header\n") ;
#endif

/* write the headers that we have so far to the message file */

	writeheader(mfp,TRUE) ;

	bclose(mfp) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to execute non-PC code\n") ;
#endif

/* now we fork so that we can process the inputting of the message */

	fp = NULL ;
	pid_child = 0 ;
	if (ex_mode != EM_PC) {

/*
 This process collects the message from the terminal and	 
 stuffs it into tempfile while the main branch continues on 
 and reads in the humongously large translation table, which 
 takes anywhere from 4 to 8 seconds to read in! Then it waits
 for this process to terminate, and takes appropriate action 
 based on the termination status of this process. If this    
 process terminates normally (exit status 0) then processing 
 the message continues as usual. If terminal status is non-zero 
 then the main branch exits. If wait returns error then the  
 main branch saves the message in $HOME/dead.letter and exits 
*/

	    bflush(g.efp) ;

	    bflush(g.ofp) ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: about to execute the fork code\n") ;
#endif

	    pid_child = -1 ;
	    if ((! g.f.interactive) || iswait || 
	        ((pid_child = uc_fork()) <= 0)) {

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: forked pid=%d\n",pid_child) ;
#endif

/* if we forked, the child is executing the following code */

	        if ((rs = bopen(mfp,tempfile,"wca",0600)) < 0) {

	            bprintf(g.efp,"%s: could not open TMP file (rs %d)\n",
	                g.progname,rs) ;

	            logfile_printf(&g.lh,"could not open TMP file (rs %d)\n",
	                g.progname,rs) ;

	            goto badret ;
	        }

	        bseek(mfp,0L,SEEK_END) ;

#if	CF_DEBUG
	        if (g.f.debug) {

	            bflush(mfp) ;

	            fileinfo(tempfile,"after seek end") ;

	        }
#endif

	        if (ex_mode == EM_PCSMAIL) {

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: interactive program mode \n") ;
#endif

/* body of the message */

	            if (*message != '\0') {

#if	CF_DEBUG
	                if (g.f.debug)
	                    debugprintf("main: we have a message ? \"%s\"\n",
	                        message) ;
#endif

/* we were given the message on the command invocation line */

	                bprintf(mfp,"\n%s\n",message) ;

	            } else if (isfile) {

#if	CF_DEBUG
	                if (g.f.debug) {

	                    debugprintf("main: we have a file already \"%s\"\n",
	                        filename) ;

	                    fileinfo(filename,"main filename") ;

	                    fileinfo(tempfile,"before add on") ;

	                }
#endif

/* we were not given the message body on the command invocation line */

	                if ((rs = bopen(tfp,filename,"r",0600)) < 0) {

#if	CF_DEBUG
	                    if (g.f.debug)
	                        debugprintf("main: cannot open \"%s\" rs %d\n",
	                            filename,rs) ;
#endif

	                    bprintf(g.efp,
	                        "%s: ERROR cannot open \"%s\"\n",
	                        filename) ;

	                    logfile_printf(&g.eh,
	                        "cannot read file \"%s\" rs=%d\n",
	                        filename,rs) ;

	                    goto badret ;
	                }

/* copy the file to the mail file */

	                rs = 0 ;

/* skip any headers that may be in the file */

#ifdef	COMMENT
	                len = 0 ;
	                while ((l = breadline(tfp,linebuf,LINELEN)) > 0) {

	                    len += l ;
	                    if (linebuf[0] == '\n') break ;

	                }
#else
	                if ((l = skipheaders(tfp,linebuf,LINELEN)) > 0)
	                    bwrite(mfp,linebuf,l) ;
#endif

#if	CF_DEBUG
	                if (g.f.debug) {

	                    offset = bseek(tfp,0L,SEEK_CUR) ;

	                    debugprintf("main: skipped headers, off=%ld\n",
	                        offset) ;

	                    fileinfo(tempfile,"after skip headers") ;

	                }
#endif

/* copy the body of the message that was given us in the input file */

#if	CF_DEBUG
	                if (g.f.debug) {

	                    bflush(mfp) ;

	                    fileinfo(tempfile,"before copyblock") ;

	                    bflush(tfp) ;

	                    fileinfo(filename,"before copyblock") ;

	                }
#endif

	                while ((rs = bcopyblock(tfp,mfp,BUFLEN)) > 0)
	                    len += rs ;

#if	CF_DEBUG
	                if (g.f.debug) {

	                    bflush(tfp) ;

	                    bflush(mfp) ;

	                    fileinfo(tempfile,"after copyblock") ;

	                    fileinfo(filename,"after copyblock") ;

	                }
#endif

	                bclose(tfp) ;

#if	CF_DEBUG
	                if (g.f.debug)
	                    debugprintf("main: %d MSG body bytes copied\n",len) ;
#endif

	                if (rs < 0) {

	                    logfile_printf(&g.lh,
	                        "possible full filesystem (rs %d)\n",
	                        rs) ;

	                    bprintf(g.efp,
	                        "%s: possible full filesystem (rs %d)\n",
	                        g.progname,rs) ;

	                }

	            } /* end if (have a file given) */

	        } /* end if (regular mode) */

	        bflush(mfp) ;

#if	CF_DEBUG
	        if (g.f.debug)
	            fileinfo(tempfile,"before isedit") ;
#endif

	        if (isedit) {

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: we are working on an edit\n") ;
#endif

	            bclose(mfp) ;

	            if (iseforward) {

#if	CF_DEBUG
	                if (g.f.debug) {

	                    debugprintf("main: we are working on an eforward\n") ;

	                    fileinfo(eforwfile,"eforward") ;

	                }
#endif

	                fappend(eforwfile,tempfile) ;

	                iseforward = 0 ;

	            } /* end if (we have a forward file) */

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: about to edit it\n") ;
#endif

#if	CF_DEBUG
	            if (g.f.debug)
	                fileinfo(tempfile,"main before editit") ;
#endif

	            editit(tempfile) ;

#if	DEBUG && 0
	            sprintf(buf,"cp %s /home/dam/rje/edit2.out",tempfile) ;

	            system(buf) ;
#endif

#if	CF_DEBUG
	            if (g.f.debug)
	                fileinfo(tempfile,"main after editit") ;
#endif

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: edited it\n") ;
#endif

	        } else if ((! isfile) && (*message == '\0')) {

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: not file and not messages\n") ;
#endif

	            if (g.f.interactive) bprintf(g.ofp,
	                "enter message - terminate by period or EOF :\n") ;

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf(
	                    "main: about to read message that we got ??\n") ;
#endif

	            while (((len = breadline(g.ifp,linebuf,LINELEN)) > 0) && 
	                (strncmp(linebuf,".\n",2) != 0)) {

	                linebuf[len] = '\0' ;
	                if (g.f.interactive && (strcmp(linebuf,"?\n") == 0)) {

	                    help(4) ;

	                    continue ;
	                }

	                bwrite(mfp,linebuf,len) ;

	            } /* end while */

	            bclose(mfp) ;

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: read the message that we collected\n") ;
#endif

	        } else {

	            bclose(mfp) ;

	        } /* end if (gathering or editing the file) */

	        fp = NULL ;

/* exit only if fork did happen, otherwise just continue */

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: child about to possibly exit\n") ;
#endif

	        if (g.f.interactive && (! iswait) && (pid_child != -1))
	            exit(0) ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: child DID NOT exit\n") ;
#endif

	    } /* end if (of forked off code) */

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: end of forked off code \n") ;
#endif

/* while the other process is collecting input from the terminal */
/* the main process ignores any interrupts and continues with its*/
/* housekeeping chores, i.e. read translation table from the disk*/

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf(
	            "main: about to exit the 'not PC' if\n") ;
#endif

	} /* end if (not running as PCS PostCard) */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to read in translation table\n") ;
#endif

/* get last->myname conversion table */

	gettable(u.nodename) ;

	if ((tablelen == 0) && (namelist[0] != '\0'))
	    bprintf(g.efp,
	        "%s: warning - no entries in the translation table\n",
	        g.progname) ;


#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: we are about to attempt joining\n") ;
#endif

/* This is where the join happens 				*/
/* the join needs to be done only if the forking actually	*/
/* happened. pid_child has a positive non-zero value only if the	*/
/* forking actually happened					*/

	if (pid_child > 0) {

	    while ((wstatus = u_waitpid(pid_child,&status,0)) >= 0) {

	        if (wstatus == pid_child) break ;

	    }

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf(
	            "main: we're in the join code w/ wstatus=%02X status=%d\n",
	            wstatus,status) ;
#endif

	    if (wstatus == -1) {

	        goto badret ;
	    }

	    if (status != 0) goto goodret ;

	goto badret ;

	} /* end if */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf( "main: we joined \n") ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf( "main: about to get names again\n") ;
#endif

	if (pid_child > 0) {

/* scan message for changes */

	    if (isedit != 0) {

	        *subject = *recipient = '\0';			/* (11/30) */
	        *copyto  = *bcopyto = '\0' ;

	        isedit = 0 ;	/* MR# gw85-00801 2/7/85 (JM)*/

	    }

	    getheader(tempfile,syscom) ;	/* (11/30) */

	    if (isedit != 0) {

	        getfield(tempfile,HS_CC,copyto) ;

	        getfield(tempfile,HS_BCC,bcopyto) ;

	    }

	    tonames = getnames(recipient) ;

	    ccnames = getnames(copyto) ;

	    bccnames = getnames(bcopyto) ;

	    isedit = 0 ;

	} /* end if */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf(
	        "main: end of gettinu.names again\n") ;
#endif

/* convert user name (in 'username') to real name and put in variable 'from' */

#ifdef	COMMENT
	myname = getname(u.username,buf) ;

	if (myname == 0) u.name = putheap(buf) ;
#endif


#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to create the 'from' address\n") ;
#endif

/* create the 'fromaddr' header field */

	strcpy(fromaddr,p.fromnode) ;

	strcat(fromaddr,"!") ;

	strcat(fromaddr,u.username) ;


	namep = NULL ;
	if (f_name) {

	    if (u.mailname != NULL)
	        namep = u.mailname ;

	    else if (u.gecosname != NULL)
	        namep = u.gecosname ;

	    else if (u.name != NULL)
	        namep = u.name ;

	    else if (f_fullname && (u.fullname != NULL))
	        namep = u.fullname ;

	}

	if (namep != NULL) {

	    if (strchr(namep,' ') != NULL) {

	        if (f_internet) sprintf(from, "%s@%s (%s)", 
	            u.username,MAILNODE,namep) ;

	        else sprintf(from, "%s!%s (%s)", 
	            p.mailnode,u.username,namep) ;

	    } else {

	        if (f_internet) sprintf(from, "%s <%s@%s>", 
	            namep, u.username, MAILNODE) ;

	        else sprintf(from, "%s <%s!%s>", 
	            namep, p.mailnode,u.username) ;

	    }

	} else {

	    if (f_internet) sprintf(from, "%s@%s", 
	        u.username, MAILNODE) ;

	    else sprintf(from, "%s!%s", 
	        p.mailnode,u.username) ;

	}



#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to do some real work again\n") ;
#endif



	if (g.f.interactive)
	    inter() ;


/* 
	Fork off code to actually deliver the mail messages.
	  Then do own notification (since UNIX doesn't always),
	  once separately for each recipient (to avoid UNIX mail 'to' line).
	  Do the actual forking only if the user has not specified the '+wait'
	  option.  If 'ex_mode' is EM_PC, forking doesn't happen because
	  'iswait' is set to 1 in these modes.
*/

#ifdef COMMENT
	fclose(errlog) ;
#endif

/* fork off to UNIX top-level */

	if ((tonames > 0) && 
	    (iswait || ((pid_child = fork()) == 0) || (pid_child == -1))) {

#if	CF_DEBUG
	    if (g.f.debug)
	        fileinfo(tempfile,"main 1") ;
#endif

	    deliverem() ;

/* if we forked, the child exits here */

	rs = OK ;
	    goto goodret ;

	} /* end of forked off code */

/* no one to send to Sighhh...! */

	if (tonames == 0) {

	    if (access(tempfile,R_OK) < 0) {

	        debugprintf("main: no tempfile here\n") ;

	        sprintf(syscom,"cp /dev/null %s",tempfile) ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: SYSTEM> %s\n",syscom) ;
#endif

	        system(syscom) ;

	    }

	    sprintf(syscom, "cat %s >> %s/%s\n", tempfile,
	        u.homedname, DEADFILE) ;

	    system(syscom) ;

	    unlink(tempfile) ;

	    if (isforward > 0) unlink(forwfile) ;

	    if (g.f.interactive) {

	        bprintf(g.ofp,"\nno recipients were specified\n") ;

	        bprintf(g.ofp,"mail saved in file '%s'\n",DEADFILE) ;

	    }

	    goto badret ;
	}

	if (g.f.interactive) {

	    bprintf(g.ofp,"sending mail ") ;

	    bputc(g.ofp,C_LPAREN) ;

	    if (standard) bprintf(g.ofp,"standard") ;

	    if (verify) bprintf(g.ofp," verify") ;

	    if (copy) bprintf(g.ofp," filecopy") ;

	    if (f_notify) bprintf(g.ofp," notify") ;

	    bprintf(g.ofp,"%c\n",C_RPAREN) ;

	}

/* we are out of here ! */
done:
goodret:
	return OK ;

/* handle bad stuff */
badret:
	return rs ;

}
/* end subroutine (process) */



