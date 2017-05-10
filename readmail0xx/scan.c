/* scan */


#define	DEBUG	0


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
 *									

 ***********************************************************************/



#include	<string.h>
#include	<stdio.h>

#include	"defs.h"



/* external subroutines */

extern char	*strshrink() ;



/* provides a brief synopsis of the message,
  giving  number, sender, subject, date, length .  
  currently assumes line is formatted into 80 characters (longscan)
    or 50 characters (shortscan).
  returns 0 if successful, 1 if error (not scanned).
  the "all" option causes deleted messages to be printed as well.
*/

char  blanks[] = "                         " ;




/* print heading if first message, then call scan routine */

scan(opts,messnum)
char opts[] ;
int messnum ;
{	

	if (firstmatch == 1)
	    if ((messdel[messord[messnum]] == 0) || 
		(strcmp(opts,"all") == 0)) {	

/* undeleted or all specified so print heading */

	        sfield_iden ();		/* identify scanline fields */
	        firstmatch = 2;		/* past firstmatch */

	    } else {

/* message marked for deletion && all not specified */

	        firstmatch = 0;		/* continue to look */
	        return 1 ;		/* don't scan */
	    }

/* scan the message */

	if (profile("long_scan"))
	    return longscan(opts,messnum) ;

	else 
		return shortscan(opts,messnum) ;

}
/* end subroutine (scan) */


longscan(opts,messnum)
char opts[] ;
int messnum ;
{
	int extmessnum ;
	int length = 0 ;
	int	i, l ;

	char string[100];	/* getfield returns values here	*/
	char scanline[100] ;
	char	temp[LINELEN + 1] ;
	char	num[40] ;
	char	*cp ;


	extmessnum = messnum;		/* preserve external number */
	messnum = messord[messnum];	/* convert to internal number */

	if (messdel[messnum] == 1) {	

/*  message marked for deletion  */

/* preface with "d" */

	    if (strcmp(opts,"all") == 0)
	        strcpy(scanline,"d") ;

	    else 
		return 1 ;

	} else	
		strcpy(scanline," ") ;

/* build rest of scanline */

	sprintf(num,"%3d",extmessnum) ;

	strcat(scanline,num);	/* stick in message number */

	strncat(scanline,blanks,2) ;

	fetchfield(messnum,"FROM",string,50) ;

	if (strncmp(string,blanks,3) == 0)
	    fetchfrom (messnum,string,15);   /* sent by UNIX mail */

	else
	    pickname(string, string, 0, 15) ;

#if	DEBUG
debugprintf("here 1\n") ;
#endif

	cp = strshrink(string) ;

#if	DEBUG
debugprintf("here 2\n") ;
#endif

	if ((l = strlen(cp)) < 15) {

		strcpy(temp,cp) ;

		for (i = 0 ; i < (15 - l) ; i += 1) strcat(temp," ") ;

		cp = temp ;
	}

#if	DEBUG
debugprintf("here 3\n") ;
#endif

	strcat(scanline,cp) ;

	strncat(scanline,blanks,3) ;

	fetchfield(messnum,"SUBJECT",string,27) ;

	strcat(scanline,string) ;

	strncat(scanline,blanks,3) ;

	fetchfield(messnum,"DATE",string,18) ;

	strcat(scanline, string) ;

	strncat(scanline,blanks,3) ;

/* length of whole message (should remove headers, esp internal ones
	   for accurate count of body only) */

	fseek(curr.fp,messbeg[messnum],0) ;

	while (ftell(curr.fp) < messend[messnum]) {

	    fgets(temp,100,curr.fp) ;

	    if ((strncasecmp(temp,"From ",5) == 0) ||
	        (strncasecmp(temp,">From",5) == 0) ||
	        (strncasecmp(temp,"FROM",4) == 0) ||
	        (strncasecmp(temp,"TO",2) == 0) ||
	        (strncasecmp(temp,"CC",2) == 0) ||
	        (strncasecmp(temp,"DATE",4) == 0) ||
	        (strncasecmp(temp,"SUBJECT",7) == 0)) continue ;

	    length += 1 ;

	}

	sprintf(num,"%4ld",length) ;

	strcat(scanline,num) ;

	printf("%s\n",scanline); /*carriage return is already there*/

	return 0 ;
}
/* end subroutine (longscan) */


/* short version of scan line */

shortscan(opts,messnum)
char opts[] ;
int messnum ;
{
	int extmessnum ;
	int length = 0 ;

	char string[100];	/* getfield returns values here	*/
	char scanline [100] ;
	char temp [LINELEN] ;
	char num[40] ;

	extmessnum = messnum;		/* preserve external number */
	messnum = messord[messnum];	/* find internal number */

	if (messdel[messnum] == 1) {	/*  message marked for deletion  */

	    if (strcmp(opts,"all") == 0)
/* preface with "d" */
	        strcpy (scanline,"d") ;
/* don't scan this message */

	    else 
	        return(1) ;

	} else	
	    strcpy (scanline," ") ;

/* build rest of scanline */

	sprintf(num,"%2d",extmessnum) ;
	strcat(scanline,num);	/* stick in message number */
	strncat(scanline,blanks,2) ;
	fetchfield (messnum,"FROM",string,50) ;

	if (strncmp(string,blanks,3) == 0)
	    fetchfrom (messnum,string,20);   /* sent by UNIX mail */
	else
	    pickname(string, string, 0, 20) ;

	namecompress (string,10) ;
	strcat (scanline,string) ;
	strncat (scanline,blanks,2) ;
	fetchfield (messnum,"SUBJECT",string,20) ;

	strcat (scanline,string) ;
	strncat (scanline,blanks,2) ;
	fetchfield (messnum,"DATE",string,18) ;

	if (strncmp(string,blanks,3) == 0)
	    fetchfield (messnum,"Date",string,10) ;

	datecompress (string,6) ;
	strcat (scanline,string) ;
	strncat (scanline,blanks,1) ;

/* length of whole message (should remove headers, esp internal ones
	   for accurate count of body only) */

	fseek(curr.fp,messbeg[messnum],0) ;

	while(ftell(curr.fp)<messend[messnum]) {

	    fgets(temp,LINELEN,curr.fp) ;

	    if ((strncasecmp(temp,"From ",5) == 0) ||
	        (strncasecmp(temp,">From",5) == 0) ||
	        (strncasecmp(temp,"FROM",4) == 0) ||
	        (strncasecmp(temp,"TO",2) == 0) ||
	        (strncasecmp(temp,"CC",2) == 0) ||
	        (strncasecmp(temp,"DATE",4) == 0) ||
	        (strncasecmp(temp,"SUBJECT",7) == 0)) continue ;

	    length += 1 ;

	}

	sprintf(num,"%4ld",length) ;

	strcat(scanline,num) ;

	printf("%s\n",scanline); /*carriage return is already there*/

	return 0 ;
}
/* end subroutine (shortscan) */


/* 
	fetches value of specified header field from specified message
          and places into "str" left adjusted and padded with blanks until
          "len" long.  (removes leading and adds trailing blanks)
	  assumes "str" can hold something that long.    
*/

fetchfield(messnum,hfield,str,len)
char hfield[],str[];  
int messnum,len ;
{
	int i ;

	char temp[LINELEN + 1], *sp ;


	str[0] = '\0' ;
	strcpy(temp,"") ;

	getfield(messnum,hfield,temp) ;

	sp = temp ;
	while (ISWHITE(*sp)) sp += 1 ;

/* remove previous text */

	str[0] = '\0' ;
	strncat(str,sp,len) ;

	for (i=strlen(str); i<len; i++)  str[i] = ' ' ;

	str[len] = '\0' ;
}
/* end subroutine (fetchfield) */


/* 
	find "FROM" field when message has been generated by UNIX mail
	   i.e. look for "From" or ">From" (for remote mail). 
*/

fetchfrom(messnum,str,len)
int messnum,len;   
char str[] ;
{
	int i ;
	int inheader ;

	char temp[LINELEN],last[LINELEN],sender[50], *word ;


/* get the sender's name */

	str[0] = '\0' ;
	if ((messnum < 1) || (messnum > nummess)) return 1 ;

	fseek(curr.fp,messbeg[messnum],0) ;

	temp[0] = '\0' ;
	fgets(temp,100,curr.fp) ;

	inheader = 1 ;
	last[0] = '\0' ;
	while ((ftell(curr.fp) < messend[messnum]) && inheader) {

		temp[0] = '\0' ;
	    fgets(temp,100,curr.fp) ;

/* Try to find a From: line */

	    if (strncasecmp(temp,"From:",5) == 0) {

	        strcpy(last,temp) ;
	        break ;
	    }

	    if (strlen(temp) < 2) inheader=0 ;

	}

	if (strlen(last) > 2) {

	    pickname(last, str, 1, len) ;

	}

/* pick up address from the UUCP "from" header line */

	if (strlen(last) < 2)	{

	    fseek(curr.fp,messbeg[messnum],0) ;

		temp[0] = '\0' ;
	    fgets(temp,100,curr.fp) ;

	    strcpy(last,temp) ;

/* find last From line (original sender) */

	    while (ftell(curr.fp) < messend[messnum]) {

		temp[0] = '\0' ;
	        fgets(temp,100,curr.fp) ;

	        if ((strncasecmp(temp,"From",4) != 0)  &&
	            (strncasecmp(temp,">From",5) != 0))    break ;

	        strcpy (last,temp) ;

	    }

/* the "From" */

	    if (strtok(last," ") != NULL) {

	        char *c = strtok(0," ") ;


	        if ( c != NULL) {

	            strncpy (str, c, len);/* the sender login name */

/* find remote machine name if there is one */

	            while ((word = strtok(0," "))  !=  NULL) {

	                if (strcmp(word,"from") == 0) {

/* found "remote from" */
	                    strcpy (sender,str) ;
	                    *str = '\0' ;
	                    c = strtok(0," \n") ;

	                    if ( c != NULL )
	                        strcpy (str,c);/* machine */

	                    strcat (str, "!") ;
	                    strcat (str, sender) ;
	                    break ;
	                }
	            }
	        }
	    }
	}

/* pad to proper field length */
/*printf("scan.c: str is %s\n",str);*/

	for (i=strlen(str); i<len; i++)  str[i] = ' ' ;

	str[len] = '\0' ;
}
/* end subroutine (fetchfrom) */


/* compress name into last name only  & pad to right len.  */

namecompress (str,len)
char str[];  
int len ;
{
	int k ;

	char *name, *sp, tempstr[100], *tname, *strrchr() ;


/* eliminate initials */

	sp = strrchr(str,'.') ;
	if (sp == NULL)
	    name = str ;

	else
	    name = sp +1 ;

/* eliminate first name(s) */

	strcpy(tempstr,name) ;

	tname = tempstr ;
	sp = strtok (tempstr," ") ;

	while (sp != NULL) {

	    tname = sp ;
	    sp = strtok (0," ") ;
	}

/* pad */
	strcpy (str,tname) ;
	for (k=strlen(str); k<len; k++)  str[k] = ' ' ;

	str[len] = '\0' ;
}
/* end subroutine (namecompress) */


/* compress date into only mm/dd  if is in that form & pad to right len. */

datecompress (str,len)
char str[]; 
int len ;
{
	int k ;

	char *sp ;


	k=0 ;
	sp = str ;
	while (*sp != '\0') {

	    if (*sp == '/')
	        if (k == 1)
	            break ;
	        else	k++ ;
	    sp++ ;
	}

	for (k=sp-str; k<len; k++)   str[k] = ' ' ;

	str[len] = '\0' ;
}
/* end subroutine (datecompress) */


/* print a line identifying the fields in the scanline */

sfield_iden()
{
	if (profile("long_scan"))
	    printf ("\n%-7s%-18s%-36s%-14s%-5s\n",
	        "MSG","FROM","SUBJECT","DATE","LINES") ;
	else	printf ("\n%-5s%-13s%-21s%-8s%-5s\n",
	    "MSG","FROM","SUBJECT","DATE","LINES") ;
}


/* This procedure is an abomination of a fudge. It essentially got to
 * be so because the code was pulled out from one procedure to make it
 * accessible from another procedure. Thus the code had to be massaged
 * to make it amenable to both cases. The major massaging that was needed
 * was to make the call to this procedure of the form:
 *
 *	pickname( name, name, 0 )
 *
 * do the proper thing. This meant copying the input argument into a local
 * string. What this procedure does is, given a From line (flag = 1) or
 * an address (flag = 0) it determines whether the address is an internet
 * address or not. If so it checks for the existence of the real name of the
 * user in parens following the address. If one exists, it returns that. If
 * not, it returns the internet address. If the address is none of these 
 * then it returns a "" string in last. Phew..!!!
 * Also str is guaranteed to be of length len as long as len < LINELEN.
 * Ptherwise it will be of length LINELEN-1.
*/

pickname( last, str, flag, len )
char	*last, *str ;
int	flag, len ;
{
	char *cpt, *c ;
	char lstring[LINELEN] ;

	if( len >= LINELEN ) len = LINELEN-1 ;
	lstring[0] = '\0' ;
	if( flag )
	{
	    if(strtok(last," ") != NULL){	/* the From: */

/* de-null-pointerized
		    strncpy (lstring, strtok(0," "),len);	
/* the address */
	        c = strtok(0, " ") ;
	        if( c != NULL ) {
	            strncpy(lstring, c, len) ;
	        }
	    }
	}
	else
	{
	    c = strtok(last, " ") ;
	    if ( c != NULL)
	        strncpy( lstring, c, len ) ;
	}
	if((cpt=strchr(lstring,'\n'))!=NULL) *cpt = '\0' ;
	*last = '\0' ;
	if(strchr(lstring,'@')!=NULL)/*  an internet address*/
	{
	    cpt = strtok(0," ") ;
	    if(cpt != NULL && (cpt=strchr(cpt, '(')) != NULL)
	    {/* pick up an user name if one can be found */
	        c = strtok(++cpt,") ") ;
	        if( c != NULL )
	            strncpy(last,c,len) ;
	        strcpy( lstring, last ) ;
	    }
	    else
	        strcpy( last, lstring ) ;
	}
	for( cpt= &lstring[strlen(lstring)]; cpt< &lstring[len]; cpt++)
	    *cpt=' ' ;
	lstring[len] = '\0' ;
	strcpy(str, lstring) ;
}
/* end subroutine (pickname) */


