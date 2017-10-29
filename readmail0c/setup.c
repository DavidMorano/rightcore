/* setup */



#define	CF_DEBUG	0



/************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 * Authors of the contents of this file:                                
 *                                                                      
 *                      Bruce Schatz                                    
 *			Jishnu Mukerji					
 *									

 ***********************************************************************/

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<time.h>
#include	<string.h>
#include <errno.h>
#include <signal.h>

#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* external subroutines */

extern char	*strdirname() ;
extern char	*strtimelog() ;


/* external variables */


/* local variables */

static struct ustat errbuf ;

static int  ptord;	/* messord ptr used in match invocation of messplace*/
static int  tempmessord[MAXMESS+1];/*keeps new ordering while calling search*/
char frwrd[] = "Forward to " ;


/* sets up desired mailbox (calls sumessptr).  
  this initializes the globals: 
    messbeg,messend (byte beg/end of mess in file),  nummess,
    messord (external->internal messnum conversion (mess ordering)),
    curr.mailbox (external name), curr.fp (internal file pointer),
    messdel (delete markers).
  returns 0 if successful,  1 if can't be opened or other error.
*/

setup_mailbox (box)
char box[] ;
{
	FILE *fk ;

	int k,m,nexttry ;
	int messplace() ;

	char fullname[LINELEN + 1] ;
	char sexp[LINELEN + 1], dummy[LINELEN + 1] ;


	box= &box[strspn(box, " ")];		/* PAS-JM 2/8/85 */
/* add fresh arrivals */
	if (strcmp (box,"new")  == 0)
	    if (get_newmail()  ==  1)   return(1) ;

/* generate actual filename, then set up the mailbox */
	full_boxname (fullname,box) ;

/* setup. return if fail */
	if (sumessptr (fullname)  ==  1)    return(1) ;

/* record the new mailbox name */
	strcpy (curr.mailbox,box) ;

/* initial ordering of the messages */
	if (profile ("fifo"))
	    for (k=1; k<=nummess; k++)    messord[k] =  k ;
	else
	{	/* want Last In Fifo Out (reverse arrival) order */
	    for (k=1; k<=nummess; k++)
	        messord[k]  =   nummess - k + 1 ;
	}

/* see if wish to sort messages  (nosortall -> only if newbox) */
	if (profile ("sort_all")          ||
	    strcmp(box,"new") == 0 )
{	/* all message references will be done via "messord" array 
	          so change the message ordering in that.
		  logical expressions are contained in  .priority file.
		        */
	    full_boxname (fullname,".priority") ;
	    if ((fk = fopen(fullname,"r"))  !=  NULL)
	    {
	        ptord=0 ;
	        while (fgets(sexp,LINELEN,fk) != NULL)
	        {	/* matches of sexp get priority */
	            sexp[strlen(sexp)-1] = '\0' ;   /* rm linefeed*/
	            if (leparse (sexp) == 0)
	                search (messplace,dummy) ;
	        }
	        fclose(fk) ;
/* copy sorted messages into real messord */
	        for (k=1; k<=ptord; k++)
	            messord[k] = tempmessord[k] ;
/* unmatched mess go in FIFO order */
	        nexttry=1 ;
	        for (k=ptord+1; k<=nummess; k++)
	        {
	            for (m=nexttry; m<=nummess; m++)
	                if (! (numinarray (messord,ptord,m)))
	                {	/* found unused messnum */
	                    messord[k] = m ;
	                    break ;
	                }
	            nexttry = m+1 ;
	        }
	    }
	}
	return(0) ;
}



/* place messnum as the next message in expression-sorted order.
  this is called by search which uses messord so it will temporarily
  place messnums in tempmessord.
*/

messplace (dummy,messnum)
char dummy[] ;
int messnum ;
{
	if (! (numinarray (tempmessord,ptord,messnum)))
	{
	    ptord++ ;
	    tempmessord [ptord]  =  messnum ;
	}
	return(0) ;
}


/* searches for num in array. return 1 if found, 0 otherwise.  */

numinarray (array,len,num)
int array[] ;
int num ;
int len;	/*length of given array*/
{
	int i ;
	for(i=1; i<=len; i++)
	    if(array[i] == num)   return(1) ;
	return(0) ;
}

/* append /usr/mail/id to "new" box to get newly arrived mail */




int get_newmail()
{
	FILE *fi, *fn ;

	time_t		daytime ;

	struct ustat	sb ;

	int	locked,k,failed = 0 ;
	int	len ;

	char	command[LINELEN + 1], newbox[LINELEN + 1], inbox[LINELEN + 1] ;
	char	lockfile[LINELEN + 1], line[LINELEN + 1] ;
	char *userid ;
	char *mailfile;	/* For setting mail file (from MAIL) PAS-JM 2/8/85*/
	char frwrdbuf[256] ;
	char	spooldir[MAXPATHLEN + 1] ;
	char	buf[(2 * MAXPATHLEN) + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*cp ;


	full_boxname (newbox,"new") ;

/* if directory for mailboxes does not exist, create it and
	   create file for new box */

	if (stat(maildir(),&errbuf)  ==  -1) {	/*PAS-JM 2/8/85*/

	    if (errno == ENOENT) {

/* create the directory and "new" box */

	        strcpy (command,"mkdir ") ;

	        strcat (command,maildir()) ;

	        usystem (command) ;

	        chmod (maildir(),0700) ;

	        creat (newbox,0600) ;

	    } else {	/* can't get to it */

	        printf ("\n *** can't access/create mail directory, ") ;
	        printf (" please check permissions *** \n\n") ;
	        return(1) ;
	    }

	} else {

/* check that extant "mail" file is really a directory */

	    if ((errbuf.st_mode & 040000)  !=  040000)/*PAS-JM2/8/85*/
	    {	/* is there but as a file */
	        printf ("\n *** please delete the file \"mail\" in ") ;
	        printf ("HOME directory *** \n\n") ;
	        return(1) ;

	    } else {

/* is there & is directory so insure "new" box extant*/

	        if (stat(newbox,&errbuf) == -1)/*PAS-JM2/8/85*/
	            if (errno == ENOENT)
	            {	/* make new box */
	                creat (newbox,0600) ;
	            }
	            else 
	            {
	                printf("\n *** can't access \"new\" box,  ") ;
	                printf("please check permissions *** \n\n") ;
	                return(1) ;
	            }
	    }
	}

/* lock /usr/mail/id from mail deliveries */
/* Allow MAIL variable to be used insted of always LOGNAME */
/* begin PAS-JM 2/8/85 */

	if (((mailfile = getenv("MAIL")) == NULL) || (*mailfile == '\0')) {

#if	CF_DEBUG
	debugprintf("setup: no MAIL environment variable\n") ;
#endif

	    strcpy(inbox,SPOOLDIR) ;

		strcat(inbox,"/") ;

	    userid = getenv("LOGNAME") ;

	    strcat(inbox,userid) ;

	} else
	    strcpy (inbox, mailfile) ;

/*
	 * Check accessibility of mail file.  This fixes a large security
	 * hole allowing someone to read someone else's mail by setting
	 * LOGNAME (or MAIL). - PAS
	 */

	if ((stat(inbox, &errbuf) == 0) && (access(inbox, A_READ) == -1)) {

	    printf(" *** no permission to read mailbox for user %s\n", 
	        inbox) ;

	    return 1 ;
	}

/* end PAS-JM 2/8/85 */

/* check if the mail spool area is writeable (administrative problem) */

#if	CF_DEBUG
	printf("setup: about to check spool area, inbox=\"%s\"\n",inbox) ;
#endif

	strcpy(spooldir,inbox) ;

	cp = strdirname(spooldir) ;

	if (access(cp,W_OK) < 0) {

	    printf("*** mail spool area is not writable for locking\n") ;

	    printf("*** spooldir=\"%s\"\n",cp) ;

	    return 0 ;
	}

/* continue */

	strcpy(lockfile,inbox) ;

	strcat(lockfile,".lock");/* same convention as UNIX mail send */

/* locking and file copying must be indivisible */

	signal (SIGHUP,SIG_IGN) ;
	signal (SIGINT,SIG_IGN) ;
	signal (SIGQUIT,SIG_IGN) ;

	k=0 ;
	while (1) {	/* try repeatedly to lock */

	    locked = creat(lockfile,0444) ;
	    if (locked >= 0) break ;

/* we failed to capture the mail lock, is it too old ? */

#if	CF_DEBUG
	    printf("setup: failed to get lock file\n") ;
#endif

	    if (stat(lockfile,&sb) >= 0) {

	        daytime = time(NULL) ;

	        if (sb.st_mtime < (daytime + MAILLOCKAGE)) {

#if	CF_DEBUG
	            printf("setup: removing old lock file\n") ;
#endif

	            unlink(lockfile) ;

	        }
	    }

/* continue */

	    sleep(2) ;

	    k += 1 ;
	    if (k > 10) {

	        printf("*** delivery area busy, couldn't fetch") ;
	        printf(" new messages ***\n\n") ;

		goto earlyexit ;
	    }

	} /* end while */

/* write some useful information into the lock file */

#if	CF_DEBUG
	printf("setup: got the lock\n") ;
#endif

	daytime = time(NULL) ;

	len = sprintf(buf,"%s readmail pid=%d\n",
	    strtimelog(daytime,timebuf),getpid()) ;

	write(locked,buf,len) ;

/* continue */

/* append $SPOOL/id to new box  */

	fi = fopen(inbox,"r") ;
	fn = fopen(newbox,"a") ;
	failed=0 ;


/* check to see if messages are being forwarded, don't blank out
		inbox if so
	*/
	if ((fi != NULL) && (fn != NULL)) {

	    if (areforwarding(fi)) {

	        printf("Your mail is being forwarded to ") ;

	        fseek(fi, (long)(sizeof(frwrd) -1), 0) ;

	        fgets (frwrdbuf,sizeof(frwrdbuf),fi) ;

	        printf("%s",frwrdbuf) ;

	        if (getc(fi) != EOF)
	            printf(
	                "and your mailbox contains extra stuff\n") ;

	        fclose(fi);	/* was previously left open JM */

	        fclose(fn);	/* was previously left open JM */

	        close(locked) ;

	        unlink(lockfile) ;

	        return 1 ;
	    }
/* copy messages from delivery box into new box */
/* convert other (non-UNIX) message formats */
	    if (profile("convert"))
	        arpacopy (fi,fn) ;

/* just copy in the new mail */
	    else	while (fgets(line,LINELEN,fi)  !=  NULL)
	        if (fputs (line,fn) == EOF)
	        {
	            printf (
			"\n *** possible low filesystem space, exiting ***\n") ;
	            close (locked) ;
	            unlink (lockfile) ;
	            exit(-1) ;
	        }

	    fclose (fi) ;
	    fclose (fn) ;

/* blank out /usr/mail/id  (clear in-box) */
	    creat (inbox,0622) ;
	}
	else if (fn == NULL)
	{	/* no new box */
	    printf("\n *** cannot open \" %s",newbox) ;
	    printf("\", please check permissions. ***\n") ;
	    fclose(fi);	/* was previously left open! JM 4/19/85 */
	    failed=1 ;
	}
	else
	{	/* no delivery box (/usr/mail/id file).  just ignore. */
	    fclose(fn);	/* was previously left open JM	*/
	}

/* unlock /usr/mail/id file */
earlyexit:
	close (locked) ;

	unlink (lockfile) ;

/* turn interrupts, etc back on */

	signal (SIGHUP,SIG_DFL) ;
	signal (SIGINT,SIG_DFL) ;
	signal (SIGQUIT,SIG_DFL) ;

	return (failed) ;
}



/* conversion routines for changing other message formats into
   the standard UNIX message format,
   i.e. the delimeter is a line beginning "From " and all other Froms have
    a ">" preceding them.
 */


/* ARPAnet conversion.
    Must place ">" before the arpa "From:" (to distinguish it from the 
    UNIX generated Froms.  Also must switch Date: line to after From: line.
  */

arpacopy (fi,fo)
FILE *fi,*fo ;
{
	char line[200], from[200], newfrom[100] ;

	while (fgets(line,LINELEN,fi) != NULL)
	{
	    if (strncmp (line,"Date:",5) == 0)
	    {	/* reverse Date: and From: lines */
	        fgets(from,LINELEN,fi);  	/* From: line */
	        strcpy (newfrom,">") ;
	        strcat (newfrom,from) ;
	        fputs(newfrom,fo) ;
	    }
	    fputs (line,fo) ;
	}
}


areforwarding(fi)
FILE *fi ;
{
	int	rs = FALSE ;

	char	line[LINELEN] ;


	if (fgets(line,LINELEN,fi) != NULL) {

	    if (strncmp(line,frwrd, sizeof(frwrd) -1) == 0) {

	        rs = TRUE ;

	    }

	    fseek(fi,0L,0) ;

	}
	return rs ;
}


/* Set up the message pointers for the specified mailbox file.
   Function returns 0 if there was at least one message.
		    1 if the mailbox could not be opened.
		    2 if no messages are in the maibox.
   mailbox file is opened here and remains open until rdebugwrite.
*/

/* MR# gx84-34161 JM 2/7/85 *//*begin*/
#define DELIMITER "From "	/* inter message separator */
#define DELLEN 5		/* length of delimiter */
/* MR# gx84-34161 JM 2/7/85 *//*end*/

sumessptr (mailfile)
char mailfile[] ;
{
	long bytecnt;	/* keeps track of location in file  (fseek offset) */

	int j ;
	int  i;		/* index variable. temporarily holds num of msgs*/
	int  inheader;	/* 1 if in header of message, 0 otherwise	*/

	char temp[COLS];	/* temporary storage line read in. */


/* just in case curr.fp is open close it */
	fclose(curr.fp) ;

/* Close the b.p.lynch hacker's security hole		JM 5/8/85 */
	if (stat(mailfile, &errbuf) == 0 && access(mailfile, A_READ) == -1)
	{
	    printf(" *** No permission for %s!\n", mailfile) ;
	    return(1) ;
	}

/* open the mailbox. if it couldn't be opened return a 1 */
	if ((curr.fp = fopen(mailfile, "r")) == NULL)
	{	
	    printf("\n *** cannot open mailbox file \"%s\" *** \n",mailfile) ;
	    return(1) ;
	}
/* mailbox is there, go set it up. */
	bytecnt = ftell(curr.fp) ;

	inheader = 0;				/*jm*/
	i=1 ;
	while (fgets(temp,COLS,curr.fp) != NULL)
	{
	    if (strncmp (temp, DELIMITER, DELLEN) == 0)
	    {
	        if( inheader == 0 ){
	            messbeg[i] = bytecnt; /* record where mess begins*/
/*previous mess ends 1 byte before new mess begins*/
	            if (i > 1)      messend[i-1] = --bytecnt ;
	            inheader = 1 ;
	            i++;		/* get ready for next message */
	        }
	    }
	    if(strlen(temp) == 1){
	        inheader = 0 ;
	    }

/* find where ptr in file is positioned */
	    bytecnt = ftell(curr.fp) ;
	}
	messend[i-1] = bytecnt-1;	/* show where last message ends	*/

	nummess =  (--i);		/* record the number of messages */

	if (nummess > MAXMESS)
	{
	    printf("\n *** too many messages in mailbox, ") ;
	    printf("please split *** \n\n") ;
	    exit(1) ;
	}

	for (j=1; j<=nummess; j++)
	    messdel[j] = 0;		/* no messages marked for deletion*/

/* if there are no messages then return a 2 */
	if (nummess <= 0)
	    return(2);		/* no messages */
	else return(0); 		/* successful completion */
}
/* end subroutine (sumessptr) */


