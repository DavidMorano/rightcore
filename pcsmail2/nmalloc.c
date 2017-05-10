/* nmalloc */


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
 *		T.S.Kennedy						
 *		J.Mukerji						
 *									

 ************************************************************************/



#include	<sys/types.h>
#include	<stdio.h>

#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* forward references */



struct table *on_nmalloc()
{
	struct table *p_table;


	if ( name[tablelen] != NULL ) {	

/* an entry is already is allocated to this entry return it */

		p_table = name[tablelen++];
		p_table->syslink = NULL;
		return(p_table);
	}

	if ( tablelen >= NAMEDB ) return NULL ;

	p_table = (struct table *) malloc( sizeof (struct table) ) ;

	p_table->syslink = NULL ;
	name[tablelen++] = p_table ;
	return p_table ;
}
/* end subroutine (nmalloc) */


int on_nextentry()
{

	return tablelen ;
}


void on_truncate( tabentry )
{

	tablelen = tabentry;
}


void on_tcopy( entry2, entry1 )
{
	struct table *ent1, *ent2;


	ent1 = name[entry1];
	ent2 = name[entry2];
	strcpy( ent2->realname, ent1->realname);

	strcpy( ent2->mailaddress, ent1->mailaddress) ;

	ent2->mail = ent1->mail;
	ent2->syslink = ent1->syslink;
}
/* end subroutine (tcopy) */



