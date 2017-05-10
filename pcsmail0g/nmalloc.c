static char sccsid[] = "@(#)nmalloc.c	PCS 3.0";

/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
 *		T.S.Kennedy						*
 *		J.Mukerji						*
 *									*
 ************************************************************************/



#include	<stdio.h>

#include	"config.h"
#include	"smail.h"



struct table *nmalloc()
{
	struct table *p_table;


	if( name[tablelen] != NULL ) {	

/* an entry is already is allocated to this entry return it */

		p_table = name[tablelen++];
		p_table->syslink = NULL;
		return(p_table);
	}

	if ( tablelen >= NAMEDB ) return(NULL);

	p_table = (struct table *) malloc( sizeof (struct table) );

	p_table->syslink = NULL;
	name[tablelen++] = p_table;
	return(p_table);
}


nextentry()
{
	return( tablelen );
}


truncate( tabentry )
{
	tablelen = tabentry;
}


tcopy( entry2, entry1 )
{
	register struct table *ent1, *ent2;

	ent1 = name[entry1];
	ent2 = name[entry2];
	strcpy( ent2->realname, ent1->realname);

	strcpy( ent2->mailaddress, ent1->mailaddress) ;

	ent2->mail = ent1->mail;
	ent2->syslink = ent1->syslink;
}
/* end subroutine (tcopy) */


