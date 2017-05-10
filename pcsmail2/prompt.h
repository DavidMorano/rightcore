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
 *									

 ************************************************************************/



#ifdef MASTER
#define VAR
#else
#define VAR extern
#endif

#define ALL	-1
#define SENDCOM	-2
#define SENDOPT	-3

#define PC_ALL		-1
#define PC_SENDCOM	-2
#define PC_SENDOPT	-3

VAR char command[BUFSIZE];
VAR char sendopt[BUFSIZE];



