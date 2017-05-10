/* inter */


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
 *                      Bruce Schat, Jishnu Mukerji                     
 *									
 ***********************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <stdio.h>

#include	<logfile.h>
#include	<userinfo.h>

#include	"config.h"
#include	"defs.h"



/* the user interface (command interpreter) for rdmail */

#define IFNUMBER(NVAR)  ((NVAR>='0' && NVAR<='9') || (NVAR=='$'))
#define IFLETTER(LVAR)  ((LVAR>='a' && LVAR<='z') || (LVAR>='A' && LVAR<='Z')                            ||   (LVAR=='('))
#define CMP(A)		(strncmp(comtoks[1],(A),strlen(comtoks[1]))==0)
#define OLDBOX  "old"          /* name of default old mailbox */



/* external variables */

extern struct global	g ;

extern struct userinfo	u ;


/* forward references */

void	catchint() ;


/* local variables */

jmp_buf  env ;

int first_scan = 0 ;




int inter()
{
	int comargs ;
	int i,j ;
	int err ;

	char mbox[LINELEN + 1];       /* will hold the default savebox */
	char command[LINELEN + 1] ;
	char comtoks[20][COLS];  /* contains command line tokenized */
	char *sp ;


/* range & logexpr take functional args and evaluate them on messages
       matching the expression.  
       "range (func,arg1,exp)"  generates   "func (arg1,messnum)"
	    for messnum in the range exp (eg 1-5,7).
       "logexpr (func,arg1,exp)"  generates "func (arg1,messnum)"
            for messnum matching the logical exp (eg FROM:schatz&SUBJECT:mail).
         arg1 must be char; it must be passed in even if unnecessary in func.
	 the global "firstmatch" is set to 1 when a match is found and it was
	     0 previously.  Those called functions (scan,gsearch) that use 
	     it (to print headings) should set it to 2 after use.
	 the globals "lastmatch" and "lastundeleted" are set to the last 
	     matched message and the last matched not marked for deletion
	     prior to executing func.  They are used to adjust the 
	     current message pointer after func evaluation.
     */

/*  these called functions are passed external messnums which they
	convert (via messord array) to internal ones as necessary.
	all other functions (except for the message checker  messvalid)
	assume they are passed internal messnums.
     */

	int scan() ;
	int delete() ;
	int undelete() ;
	int save() ;
	int reply() ;
	int forward() ;
	int putfile() ;
	char dummy[100];	/* dummy arg1 */


/* only messages not marked for deletion are ever accessible, except
      for "scan all".  
    each command resets the current message pointer (curr.msgno) so that
      it never points to a deleted message.
 */

enter:	
	printf("\n") ;

	if (nummess > 0 ) { 	

/* setup:  identify, scan, msg pointer */

	    if (profile("all_scan") || (strcmp(u.username,"gerri") == 0)) {	

/* only if user oks */

	        printf("\n  mailbox \"%s\"\n", curr.mailbox) ;

	        first_scan = 1 ;

	    } else {	

/* if off, print out only number of messages */

	        printf("\n  mailbox \"%s\" :  %d message%c\n",
	            curr.mailbox,nummess,(nummess>1 ? 's' : ' ')) ;

	    }

	    curr.msgno = 1;			/* start with first message */

	} else {

	    printf("\n  no messages in mailbox \"%s\"\n",curr.mailbox) ;

	    curr.msgno=0 ;
	}

/* loop through accepting and executing commands */
top:
	setjmp(env) ;              /* interrupts return here for new command */

	if (first_scan) {

	    first_scan = 0 ;
	    signal(SIGINT,catchint) ;

	    firstmatch = 1 ;   	/* print heading */
	    for (j = 1 ; j <= nummess ; j += 1)
	        scan("",j) ;    /* scan all messages */

#if	CF_DEBUG
	debugprintf("inter: first scan ?\n") ;
#endif

#ifdef	COMMENT
	    printf("\n\n") ;
#endif

	}

/* set global message pointing variables */
	firstmatch = 0;		   /* no matches yet */
	lastmatch = curr.msgno ;
	lastundeleted = curr.msgno ;

/* print the prompt */

	printf("\n") ;


/* scan current mess before each prompt*/

		/* no scan if mailbox empty */

	if (curr.msgno > 0) {

	    if (profile("each_scan"))   
	        scan("all",curr.msgno) ;

	    else 
		printf("%d  ",curr.msgno) ;

	}

	printf("? ");		/* this is the prompt */

	signal (SIGINT,catchint);  /* interrupts longjmp to "top" */

/* get and parse command string into tokens. 
	   tokens are separated by blanks.
	    (thus no blanks allowed in ranges or logical expressions)
	   command name is token 1; args are 2-n .
         */
	gets(command) ;
	comargs=0;	/* first command argument will be comtoks[1] */
	sp = strtok(command," ") ;
	while (sp != NULL)
	{
	    strcpy (comtoks[++comargs],sp) ;
	    sp = strtok(0," ") ;
	}
	comtoks[comargs+1][0] = NULL;     /* useful as a stopping point */


/* interpret the command */


/* if a carriage return, display current message */
	if (comargs == 0)
	{
/* display next message */
	    if (messvalid(curr.msgno))
	        display (curr.msgno,stdout,1) ;
	    goto top ;
	}
/* if a "|" of ">", divert current message */
	if (comtoks[1][0] == '|' || comtoks[1][0] == '>')
	{
/* display next message */
	    if (messvalid(curr.msgno))
	    {
	        command[0] = '\0' ;
	        for(i = 1; i <= comargs; i++)
	            sprintf(command + strlen(command),"%s ",comtoks[i]) ;
	        output (curr.msgno,command) ;
	    }
	    goto top ;
	}
/* full call to sendmail */
	if (strcmp("sendmail",comtoks[1])  ==  0)
	{
	    strcpy (command,"sendmail") ;
	    for (i=2; i<=comargs; i++)
	    {
	        strcat (command," ") ;
	        strcat (command,comtoks[i]) ;
	    }
	    usystem(command) ;
	    goto top ;
	}
/* same as "change new".  assume new box always exists. */
	if (strcmp("new",comtoks[1])  ==  0)
	{
	    rdebugwrite() ;
	    setup_mailbox("new") ;
	    goto enter ;
	}

/* goto the given message number and display it */
	if IFNUMBER(comtoks[1][0])
	{
	    i = atoi(comtoks[1]) ;
	    if (messvalid(i))
	    {
	        curr.msgno = i;   /* set current message number*/
/* if a "|" of ">", divert current message */
	        if (comtoks[2][0] == '|' || comtoks[2][0] == '>')
	        {
/* divert output of next message */
	            command[0] = '\0' ;
	            for(i = 2; i <= comargs; i++)
	                sprintf(command + strlen(command),"%s ",comtoks[i]) ;
	            output (curr.msgno,command) ;
	        }
/* else display message */
	        else display (i,stdout,1) ;
	    }
	    goto top ;
	}
/* display previous (first undeleted previous) message */
	if CMP("prev")
	{
	    if (messnp(-1) == 0)
	        display (curr.msgno,stdout,1) ;
	    goto top ;
	}
/* display next message */
	if CMP("next")
	{
	    if (messnp(1)  ==  0)
	        display (curr.msgno,stdout,1) ;
	    goto top ;
	}

/* print scanline of specified messages.  arg1 = options (eg "all").
	   when done, curr.msgno will be at last undeleted message scanned.
	 */
	if (CMP("scan") || CMP("header")) {

/* scan all undeleted messages */

	    if (comargs == 1) {

/* keep trying to print heading */

	        for (i = 1 ; i <= nummess ; i += 1) {

	            firstmatch = 1 ;
	            if (scan("",i) == 0) break ;

	        }
	        if (i > nummess) goto top ;    /*no undeleted mess*/

	        lastundeleted = i ;

/* rest of messages */
	        for (j = i + 1 ; j <= nummess ; j += 1)
	            if (scan("",j) == 0) lastundeleted = j ;

	    } else if (strcmp(comtoks[2],"all")  == 0) {

	/* scan all including deleted */
	        firstmatch = 1 ;
	        for (j=1; j<=nummess; j++)    /* rest of messages */
	        {
	            if (messdel[messord[j]] ==0) lastundeleted = j ;
	            scan("all",j) ;
	        }

	    } else if (strncmp(comtoks[2],"discussion",
		strlen(comtoks[2])) == 0) {

	/* scan all messages that have the same  */
/* message-id as the first reference in this message*/

	        char mess_id[LINELEN], *c ;
	        char messstr[80] ;


	        if( getfield( curr.msgno,"REFERENCES:",mess_id ) == 0)
	        { /* there is a references line */
/* now pick up the first reference number */
	            if(( c = (char *)strpbrk(mess_id,", \n")) == NULL )
	                *c = '\0' ;
	        }
	        for(c=mess_id; *c!='\0' && *c==' '; c++) ;
	        if( *c != '\0')
	        {
	            sprintf(messstr,"REFERENCES:%s|MESSAGE-ID:%s",
	                c,c) ;
	            logexpr (scan,"",messstr) ;
	        }
	        else
	        {
	            printf("\nNo previous message in discussion\n\n") ;
	        }
	    }
	    else if IFNUMBER(comtoks[2][0])
	        range (scan,comtoks[3],comtoks[2]) ;
	    else
	        logexpr (scan,comtoks[3],comtoks[2]) ;

	    curr.msgno = lastundeleted ;
	    goto top ;
	}

/* mark message(s) for deletion */
	if CMP("delete")
	{
	    if (comargs == 1)
	    {	/* delete current message */
	        delete(" ",curr.msgno) ;
	    }
	    else if IFNUMBER(comtoks[2][0])
	        range (delete,dummy,comtoks[2]) ;
	    else 
	        logexpr (delete,dummy,comtoks[2]) ;

/* set message pointer to next undeleted mess after last 
		  message just marked */
	    curr.msgno = lastundeleted ;
	    messadjust() ;

	    goto top ;
	}

/* remove deletion mark on message(s) */
	if CMP("undelete")
	{
	    if (comargs == 1)
	        undelete(" ",curr.msgno) ;
	    else if IFNUMBER(comtoks[2][0])
	        range (undelete,dummy,comtoks[2]) ;
	    else
	        logexpr (undelete,dummy,comtoks[2]) ;

	    curr.msgno = lastmatch ;
	    goto top ;
	}

/* "save" command.  copy messages to specified mailboxes and mark for 
	  deletion in current mailbox. 
	  when done, curr.msgno will point after the last moved message.
	 */
	if CMP("move")
	{
	    full_boxname (mbox,OLDBOX);	/* default save box */
	    err = 0 ;
	    if (comargs == 1)
	    {	/* no arguments.  save current mess into default box */
	        err =  save (mbox,curr.msgno) ;
	    }
	    else if IFNUMBER (comtoks[2][0])
	    {	/* range of messages */
	        if (comargs == 2)
	        {	/* save into default box */
	            range (save,mbox,comtoks[2]) ;
	        }
	        else
	        {	/* save into specified mailboxes */
	            for (j=3; j<=comargs; j++)
	            {
	                full_boxname (mbox,comtoks[j]) ;
	                range (save,mbox,comtoks[2]) ;
	            }
	        }
	    }
	    else if ((findheader(comtoks[2])  > -1)  ||
	        (comtoks[2][0] == '('))
	    {	/* logical expression */
	        if (comargs == 2)
	        {	/* save into default box */
	            logexpr (save,mbox,comtoks[2]) ;
	        }
	        else
	        {	/* save into specified mailboxes */
	            for (j=3; j<=comargs; j++)
	            {
	                full_boxname (mbox,comtoks[j]) ;
	                logexpr (save,mbox,comtoks[2]) ;
	            }
	        }
	    }
	    else
	    {	/* only mailboxes listed. save curr mess into them. */
	        for (j=2; j<=comargs; j++)
	        {
	            full_boxname (mbox,comtoks[j]) ;
	            err =  save (mbox,curr.msgno) ;
	        }
	    }

/* set message pointer to next undeleted mess after last 
		  message just marked */
	    curr.msgno = lastundeleted ;
	    if (!err)     messadjust() ;

	    goto top ;
	}

/* change to new mailbox */
	if CMP("change")
	{
	    FILE *fd ;
	    full_boxname (mbox,comtoks[2]) ;
	    if (((fd=fopen(mbox,"r")) == NULL)   &&  
	        (strcmp(comtoks[2],"new") != 0))
	    {	/* don't do anything */
	        fclose(fd) ;
	        printf("\n non-existent mailbox \"%s\", ", comtoks[2]) ;
	        printf("please respecify.\n") ;
	        goto top ;
	    }
	    rdebugwrite() ;
	    setup_mailbox(comtoks[2]) ;
	    goto enter ;
	}
/* list all current mailboxes */
	if CMP("list")
	{
	    mlist() ;
	    goto top ;
	}
/* print current mailbox */
	if CMP("boxname")
	{
	    printf("\n  current mailbox is \"%s\" \n",curr.mailbox) ;
	    goto top ;
	}
/* do scan across all mailboxes */
	if CMP("gsearch")
	{
	    if (comargs == 1)
	        printf("\n no search expression, please respecify.\n") ;
	    else
	        gsearch (comtoks[2]) ;
	    goto top ;
	}

/* reply to specified message */
	if CMP("reply")
	{
	    if (comargs == 1)
	        reply (curr.msgno,0) ;
	    else 
	    {
	        for (i=0; i<strlen(comtoks[2]); i++)
	            if (! isdigit(comtoks[2][i]))
	            {
	                printf("\n  invalid message number, ") ;
	                printf("please respecify.\n") ;
	                goto top ;
	            }
	        if (messvalid (atoi(comtoks[2])))
	        {
	            curr.msgno = atoi (comtoks[2]) ;
	            reply (curr.msgno,0) ;
	        }
	    }
	    goto top ;
	}
	if CMP("Reply")
	{
	    if (comargs == 1)
	        reply (curr.msgno,1) ;
	    else 
	    {
	        for (i=0; i<strlen(comtoks[2]); i++)
	            if (! isdigit(comtoks[2][i]))
	            {
	                printf("\n  invalid message number, ") ;
	                printf("please respecify.\n") ;
	                goto top ;
	            }
	        if (messvalid (atoi(comtoks[2])))
	        {
	            curr.msgno = atoi (comtoks[2]) ;
	            reply (curr.msgno,1) ;
	        }
	    }
	    goto top ;
	}

/* append specified message onto mail-prepared message */
	if CMP("forward")
	{
	    if (comargs == 1)
	        forward (curr.msgno,0) ;
	    else 
	    {
	        for (i=0; i<strlen(comtoks[2]); i++)
	            if (! isdigit(comtoks[2][i]))
	            {
	                printf("\n  invalid message number, ") ;
	                printf("please respecify.\n") ;
	                goto top ;
	            }
	        if (messvalid (atoi(comtoks[2])))
	        {
	            curr.msgno = atoi (comtoks[2]) ;
	            forward (curr.msgno,0) ;
	        }
	    }
	    goto top ;
	}
	if CMP("editforward")
	{
	    if (comargs == 1)
	        forward (curr.msgno,1) ;
	    else 
	    {
	        for (i=0; i<strlen(comtoks[2]); i++)
	            if (! isdigit(comtoks[2][i]))
	            {
	                printf("\n  invalid message number, ") ;
	                printf("please respecify.\n") ;
	                goto top ;
	            }
	        if (messvalid (atoi(comtoks[2])))
	        {
	            curr.msgno = atoi (comtoks[2]) ;
	            forward (curr.msgno,1) ;
	        }
	    }
	    goto top ;
	}

/* appends message to specified file */

	if CMP("output") {

	    if (comargs == 1)
	        printf("\n  too few arguments, please respecify.\n") ;

	    else if IFNUMBER(comtoks[2][0])

	        if (comargs == 2) {

	            printf("\n  no filename given, please") ;
	            printf(" respecify.\n") ;

	        } else	
			range (putfile,comtoks[3],comtoks[2]) ;

	    else if ((findheader(comtoks[2]) > -1) || (comtoks[2][0] == '(' ))

	        if (comargs == 2) {

	            printf("\n  no filename given, please") ;
	            printf(" respecify.\n") ;

	        } else	
			logexpr (putfile,comtoks[3],comtoks[2]) ;

	    else
	        putfile (comtoks[2],curr.msgno) ;

	    curr.msgno = lastundeleted ;
	    goto top ;
	}

/* leave rdmail. make deletions if requested. */

	if CMP("quit") {

	    rdebugwrite() ;
	    return(0) ;
	}

/* escape to shell for one command */
	if (comtoks[1][0] == '!') {

	    strcpy (command,comtoks[1]+1) ;
	    for (i=2; i<=comargs; i++) {

	        strcat (command," ") ;

	        strcat (command,comtoks[i]) ;

	    }
	    usystem(command) ;

	    goto top ;
	}

/* print out the version */

	if CMP("V") {

	    printf("version %s/%s\n",
		VERSION,"X") ;

	    goto top ;
	}

/* help (short descriptions of all or specific commands) */

	if CMP("?") {

	    help (comtoks[2]) ;

	    goto top ;

	} else {

	    printf("\n illegal command, please respecify.\n") ;

	    goto top ;
	}

}
/* end subroutine (inter) */


/* catch interrupts, terminate command, and return for new command */
void catchint(n)
int	n ;
{	

	signal (SIGINT,SIG_IGN);   /* ignore interrupts until print prompt */
	longjmp (env,1) ;
}


