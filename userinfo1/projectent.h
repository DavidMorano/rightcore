/* projectent INCLUDE */

/* subroutines for simple PROJECT object (from UNIX® library-3c) management */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PROJECTENT_INCLUDE
#define	PROJECTENT_INCLUDE	1


#include	<envstandards.h>
#include	<project.h>


#define	PROJECTENT	struct project


#ifdef	__cplusplus
extern "C" {
#endif

extern int projectent_load(struct project *,char *,int,const struct project *) ;
extern int projectent_parse(struct project *,char *,int,const char *,int) ;
extern int projectent_size(const struct project *) ;
extern int projectent_format(const struct project *,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* PROJECTENT_INCLUDE */


