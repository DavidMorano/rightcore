/* reply */

/************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 * Authors of the contents of this file:                                
 *                                                                      
 *                      Jishnu Mukerji                                  
 *									
 ***********************************************************************/



#include <stdio.h>

#include	"config.h"
#include	"defs.h"



/* external subroutines */

extern char	*getenv() ;


/* external variables */

extern struct global	g ;




#define TO_LEN		100
#define SUBJ_LEN	256

/* lets the user type a reply to the specified message */
/* modified several times by J.Mukerji between 4.1.84 and 8.1.84 to do the *
 * following:								   *
 *									   *
 * (i)	Interface with nsmail using a file rather than the command line.   *
 * (ii)	Pass a copy of the original message to nsmail so that it is avail- *
 *	able for displaying in the +edit mode				   *
 *	This is done only if the editor specified in the environment	   *
 *	variable ED can make good use of it.				   */

int reply(messnum, mode)
int	messnum;
int	mode;
{
	FILE *tfd, *ofd;

	int smessnum;

	char to[TO_LEN],subj[SUBJ_LEN],oldsubj[SUBJ_LEN],topath[TO_LEN];
	char ccto[TO_LEN], recipto[TO_LEN];
	char messid[TO_LEN], reference[SUBJ_LEN];
	char command[TO_LEN+SUBJ_LEN+12];
	char tempfile[TO_LEN], otempfile[TO_LEN];
	char *pathto, *toname, *name,*last, *strchr(), *comment;
	char *tmp1;
	char	*cp ;


	comment = NULL;
	smessnum = messnum;
	messnum = messord[messnum];        /* convert to internal number */

	 /* get recipient (= old from).  select out only the last 
	   name to insure proper translation . */

	getfield (messnum,"FROM:",to);

	getfield (messnum,"PATH:",topath);

	getfield (messnum,"MESSAGE-ID:",messid);

	getfield (messnum,"REFERENCES:",reference);

	if ( mode ) {

		getfield (messnum,"CC:",ccto);

		getfield (messnum,"TO:",recipto);

	}

	if (*topath == NULL)
		fetchfrom (messnum, topath, TO_LEN - 1);
	if (*to == NULL)
		strcpy(to, topath);	/* non-sendmail From */
	for ( toname = to; *toname == ' '; toname++);
					/* get rid of leading spaces */

/* We have to deal with the new SMTP from line which is of the form
   From: some string <address>
   here.
*/
/* This is an SMTP mail header, so do the right thing for it */

	if ( strchr( toname, '<' ) != NULL ) { 

		comment = strtok( toname, "<");
		last = strtok( 0, ">");

	} else {	

/* FROM: address (comment string) */

		if (strchr(toname,')') != NULL) {

			last = strtok( toname, "(" );
			comment = strtok( 0, ")" );

		} else
			last = strtok(toname," ");

	}

	if ( last == NULL ) last = "";

	for ( name = &to[strlen(to)-1]; *name == ' '; *name-- = '\0'); 

					/* get rid of trailing blanks */

	for ( pathto = topath; *pathto == ' '; pathto++);

	for ( name = &topath[strlen(topath)-1]; 
		*name == ' ' || *name == '\n'; name--);

	*(++name) = '\0';

	/* strip last of leading and trailing blanks */
	for ( ; *last == ' '; last++ );

	for ( name = last; *name; *name++);

	for (name-- ; *name == ' '; name-- );

	*(++name) = '\0';

	 /* get subject */
	getfield(messnum,"SUBJECT:",oldsubj);

	tmp1 = oldsubj+strspn(oldsubj," ");

	if (strncmp ("re:", tmp1 ,3) != 0) 
		strcpy (subj,"re: ");

	else
		strcpy (subj,"");		/* already has re: */

	strcat (subj, tmp1);

	strcpy(tempfile,"/tmp/replyXXXXXX");

	mktemp(tempfile);

	if((tfd = fopen( tempfile, "w" )) == NULL) {

		printf("Unable to create reply message!\n");
		return 1 ;
	}

	chown(tempfile, getuid(), MAILGROUP);

	chmod(tempfile, 0640 );


	/* create file for passing the original message	and put original *
	/* message in it only if the user can make good use of it.	 */

	if ((cp = getenv("ED")) == NULL) cp = "vi" ;

	otempfile[0] = NULL;
	if (strncmp(cp, "gem", 3) == 0) {

		strcpy(otempfile,"/tmp/originalXXXXXX");

		mktemp(otempfile);

		if((ofd = fopen( otempfile, "w" )) == NULL) {

			printf("Unable to create reply message!\n");

			return 1 ;
		}

		chown(otempfile, getuid(), MAILGROUP);

		chmod(otempfile, 0400 );

		display(smessnum, ofd, 0);

		fclose(ofd);

	}

	/* put reply message template in reply file		*/
	if ( comment != NULL )
		fprintf(tfd,"TO: \"%s\" <%s>\n",comment,last);

	else
		fprintf(tfd,"TO: %s\n",last);

	if(mode) {

		if ( *ccto || *recipto ) fprintf(tfd,"CC: ");

		if (*ccto) fprintf(tfd,"%s%s",ccto,*recipto?"":"\n");

		if (*recipto) fprintf(tfd,"%s %s\n", *ccto?",":"",recipto);

	}

	if (*subj) fprintf(tfd,"SUBJECT: %s\n",subj);

	if ((*reference != NULL) || (*messid != NULL) ) 
		fprintf(tfd,"REFERENCES: %s %s\n",reference, messid) ;

	fclose(tfd) ;

	sprintf(command,"%s f=%s re=%s %s%s",
		g.prog_mailer,tempfile, pathto,
		otempfile[0]?"or=":"",otempfile) ;

	ssystem(command) ;

	unlink(tempfile) ;

	unlink(otempfile) ;

	return 0 ;
}
/* end subroutine (reply) */


