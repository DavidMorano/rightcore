/* last modified %G% version %I% */

/* DiSCOfunc (local SYSCAD SCHEMA CI functions for DiSCO) */

/*
	David A.D. Morano
	December 1992
*/

/**********************************************************************

	These SYSCAD SCHEMA CI functions are especially designed
	for use on the DiSCO project.


***********************************************************************/


#define		F_DEBUG		0



#include	"schema.h"



#define		LINELEN		120
#define		NAMELEN		100

#define		OK		0
#define		BAD		-1

#define		TRUE		1
#define		FALSE		0



/* local static data */

static DATA	dummy[1] ;


/* audit power/ground IOPART symbols for proper attached net names */

static char	*iopartname[] = {
	"IOPARTPB.E",
	"IOPARTGB.E",
	"IOPARTPS.E",
	"IOPARTGS.E",
#ifdef	COMMENT
	"IOPARTGEB.E",
#endif
	NULL
} ;

#define		SKIP		-1
#define		IOPARTPB	0
#define		IOPARTGB	1
#define		IOPARTGEB	2




/* Audit Auto Voltage */
int audit_av(argc,argv)
int	argc ;
char	*argv[] ;
{
	DATA	ap[1], gp[1], dap[1], pap[1], sap[1], wap[1] ;

	COORD	xy[256] ;

	int	i, j, k ;
	int	type ;
	int	ncoord, kind ;
	int	f_wire ;

	char	curgrp[GRP_NM_SZ + 1] ;
	char	grpname[GRP_NM_SZ + 1] ;
	char	wirename[NAMELEN + 1], valuename[NAMELEN + 1] ;
#ifdef	COMMENT
	char	*answer ;
#endif


	if (s_curgroup(curgrp) != 0) {

	    printf(NEED_PAG_SYM) ;

	    return BAD ;
	}

	if (s_getgroup(gp,curgrp,0) != 0) return BAD ;

/* loop through all of the atoms in this group for IOPARTs */

	for (i = 1 ; s_getatom(ap,gp,i) == 0 ; i += 1) {

	    if (s_inquire(ap,"def_group_name",grpname) != 0) continue ;

/* find which type of IOPART that we may have */

	    for (type = 0 ; iopartname[type] != NULL ; type += 1) {

	        if (strcmp(iopartname[type],grpname) == 0) break ;

	    }

/* continue looping if we do not have a part ? */

	    if ((iopartname[type] == NULL) ||
	        (s_inquire(ap,"def_group",dap) != 0) ||
	        (s_inquire(ap,"pla_ext",xy) != 0)) continue ;

/* find the terminal-pin in the IOPART symbol */

	    f_wire = FALSE ;
	    for (j = 1 ; s_getatom(pap,dap,j) == 0 ; j += 1) {

	        if ((s_inquire(pap,"kind",&kind) != 0) ||
	            (kind != PIN) ||
	            (s_inquire(pap,"pla_coord",xy,&ncoord) != 0)
	            ) continue ;

/* look for a subnet connected to the terminal-pin */

	        if (s_getatomat(sap,xy,SUBNET,gp) == 0) {

#ifdef	F_DEBUG
	printf("audit: got a SUBNET\n") ;
#endif

	            for (k = 1 ; (! f_wire) && (! s_getatom(wap,sap,k)) ; 
			k += 1) {

	                if (s_inquire(wap,"kind",&kind) || (kind != WIRE)) 
			    continue ;

	                if (s_inquire(wap,"NAME",wirename) != 0)
	                    continue ;

	                f_wire = TRUE ;
	                break ;
	            }

	            if (! f_wire) printf(
	                "ERROR: %s unnamed subnet @ %4.0f,%4.0f\n",
	                grpname,xy[0].x,xy[0].y) ;

	            break ;

	        } else {

	            printf(
	                "ERROR: %s has no wire @ %4.0f,%4.0f\n",
	                grpname,xy[0].x,xy[0].y) ;

	            break ;

	        }

	    } /* end for */

	    type = (f_wire) ? type : SKIP ;
	    switch (type) {

	    case IOPARTPB:
	        if (s_inquire(ap,"VALUE",valuename) == 0) {

	            if (strcmp(wirename,valuename) != 0) {

	                printf(
	                    "ERROR: %s mismatch - v %s - n %s",
	                    grpname,valuename,wirename) ;

	                printf(" @ %4.0f,%4.0f\n",
	                    xy[0].x,xy[0].y) ;

	            }

	        } else {

	            printf("ERROR: %s has no value @ %4.0f,%4.0f\n",
	                grpname,xy[0].x,xy[0].y) ;

	        }
	        break ;

	    case IOPARTGB:
	        if ((strcmp(wirename,"GRD") != 0) &&
	            (strcmp(wirename,"GND") != 0)) {

	            printf(
	                "ERROR: %s does not have ground net @ %4.0f,%4.0f\n",
	                grpname,xy[0].x,xy[0].y) ;

	        }
	        break ;

	    case IOPARTGEB:
	        break ;

	    default:
	        printf("ERROR: default on switch - %d %s\n",
	            type,grpname) ;

	    case SKIP:
	        break ;

	    } /* end switch */

	} /* end for */

	return E_OK ;
}
/* end subroutine (audit_av) */



