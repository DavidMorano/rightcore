/* reply */

/* lets the user type a reply to the specified message */

/************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 *									

 ***********************************************************************/



#include	<string.h>
#include	<stdio.h>

#include	"localmisc.h"
#include	"defs.h"



#define TO_LEN		4096
#define SUBJ_LEN	4096



/* external subroutines */

extern char	*getenv() ;
extern char	*strshrink() ;
extern char	*ma_path() ;



/*************************************************************************

	Arguments:
	+ messnum		message_number
	+ f_conference		0=regular_reply, 1=conference_reply


Modified several times by J.Mukerji between 4.1.84 and 8.1.84 to do the 
 * following:								   
 *									   
 * (i)	Interface with PCSMAIL using a file rather than the command line.   
 * (ii)	Pass a copy of the original message to nsmail so that it is avail- 
 *	able for displaying in the +edit mode				   
 *	This is done only if the editor specified in the environment	   
 *	variable ED can make good use of it.				   


***************************************************************************/

int reply(messnum, f_conference)
int	messnum ;
int	f_conference ;
{
	FILE	*tfp, *otfp ;

	int	smessnum ;

	char	to_buf[TO_LEN], *to = to_buf ;
	char	subj_buf[SUBJ_LEN], *subj = subj_buf ;
	char	oldsubj_buf[SUBJ_LEN], *oldsubj = oldsubj_buf ;
	char	topath_buf[TO_LEN], *topath = topath_buf ;
	char	ccto_buf[TO_LEN], *ccto = ccto_buf ;
	char	recipto_buf[TO_LEN], *recipto = recipto_buf ;
	char	messid_buf[TO_LEN], *messid = messid_buf ;
	char	reference_buf[SUBJ_LEN], *reference = reference_buf ;
	char	command[TO_LEN+SUBJ_LEN+12];
	char	tempfile[TO_LEN], otempfile[TO_LEN];
	char	*pathto, *toname, *name, *last = NULL, *comment ;
	char	*tmp1 ;
	char	*cp ;


	comment = NULL;
	smessnum = messnum;
	messnum = messord[messnum];        /* convert to internal number */

/* get recipient (= old from).  select out only the last 
	   name to insure proper translation */

	to[0] = '\0' ;
	getfield (messnum,"reply-to",to);

	to = strshrink(to) ;

	if (to[0] == '\0') {

	    getfield (messnum,"from",to);

	    to = strshrink(to) ;

	}

/* get some stuff just in case we need it */

	getfield (messnum,"PATH:",topath);

	getfield (messnum,"MESSAGE-ID:",messid);

	getfield (messnum,"REFERENCES:",reference);

/* get more stuff if we are conference replying */

	if (f_conference) {

	    getfield (messnum,"CC:",ccto);

	    getfield (messnum,"TO:",recipto);

	}

/* try to get a return path from the UNIX envelope header if necessary */

	topath = strshrink(topath) ;

	if (topath[0] == '\0') {

	    fetchfrom (messnum, topath, TO_LEN - 1);

	    topath = strshrink(topath) ;

	}

/* get the intended recipient from the return path if necessary */

	if (to[0] == '\0')
	    strcpy(to, topath) ;

/* start processing of addresses */


/* cleanup PATHTO */

	topath = ma_path(topath) ;

/* FROM: address (comment string) */

	if (strchr(toname,')') != NULL) {

	    last = strtok( toname, "(" ) ;

	    comment = strtok( 0, ")" ) ;

	} else
	    last = strtok(toname," ") ;

	if ( last == NULL ) last = "" ;


/* strip last of leading and trailing blanks */

	last = strshrink(last) ;

/* get subject */

	getfield (messnum,"SUBJECT:",oldsubj) ;

	tmp1 = oldsubj + strspn(oldsubj," ") ;

	if (strncmp ("re:", tmp1 ,3) != 0)
	    strcpy (subj,"re: ") ;

	else
	    strcpy (subj,"") ;		/* already has re: */

	strcat (subj, tmp1);

	strcpy(tempfile,"/tmp/replyXXXXXX");

	mktemp(tempfile);

	if ((tfp = fopen( tempfile, "w" )) == NULL) {

	    unlink(tempfile) ;

	    printf("readmail: unable to create reply message !\n");

	    return BAD ;
	}

	chown(tempfile, getuid(), MAILGROUP) ;

	chmod(tempfile, 0640 );


/* create file for passing the original message	and put original */

	otempfile[0] = '\0' ;

/* see if there are special editor requirements */

	if ((cp = getenv("ED")) == NULL) {

	    if ((cp = getenv("EDITOR")) == NULL) cp = "" ;

	}

	if (strncmp(cp, "gem", 3) == 0) {

	    strcpy(otempfile,"/tmp/originalXXXXXX");

	    mktemp(otempfile);

	    if ((otfp = fopen( otempfile, "w" )) == NULL) {

	        unlink(otempfile) ;

	        printf("readmail: unable to create reply message !\n");

	        return BAD ;
	    }

	    chown(otempfile, getuid(), MAILGROUP);

	    chmod(otempfile, 0400 );

	    display(smessnum, otfp, 0);

	    fclose(otfp);

	} /* end if (special processing for the GEM editor) */

/* put reply message template in reply file		*/

	if ( comment != NULL ) {

	    if (strpbrk(comment," \t") != NULL)
	        fprintf(tfp,"TO: \"%s\" <%s>\n",comment,last) ;

	    else
	        fprintf(tfp,"TO: %s <%s>\n",comment,last) ;

	} else
	    fprintf(tfp,"TO: %s\n",last);

	if (f_conference) {

	    if ( *ccto || *recipto ) fprintf(tfp,"CC: ");

	    if (*ccto) fprintf(tfp,"%s%s",ccto,*recipto?"":"\n");

	    if (*recipto) fprintf(tfp,"%s %s\n", *ccto?",":"",recipto);

	}

	if (subj[0] != '\0') fprintf(tfp,"SUBJECT: %s\n",subj);

	if ((reference[0] != '\0') || (messid[0] != '\0') )
	    fprintf(tfp,"REFERENCES: %s %s\n",reference, messid);

	fclose(tfp);

	sprintf(command,"%s f=%s re=%s %s%s",
	    SENDMAIL,tempfile, pathto,otempfile[0]?"or=":"",otempfile);

	ssystem (command) ;

	unlink(tempfile);

	unlink(otempfile);

	return OK ;
}
/* end subroutine (readmail) */


