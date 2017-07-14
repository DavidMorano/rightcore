/* pdb */

/* printer database */


#ifndef	PDB_INCLUDE
#define	PDB_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<svcfile.h>
#include	<localmisc.h>


#define	PDB_MAGIC	0x77446329
#define	PDB		struct pdb_head
#define	PDB_DB		struct pdb_db


enum pdbs {
	pdb_local,
	pdb_system,
	pdb_overlast
} ;

struct pdb_db {
	SVCFILE		dbfile ;
	time_t		ti_find ;
	time_t		ti_check ;
	time_t		ti_open ;
	time_t		ti_mtime ;
	uint		f_open:1 ;
} ;

struct pdb_head {
	unsigned long	magic ;
	PDB_DB		dbs[pdb_overlast] ;
	time_t		dt ;
	cchar		*a ;		/* memory allocation */
	cchar		*pr ;		/* root program */
	cchar		*ur ;		/* root root */
	cchar		*uname ;	/* username */
	cchar		*fname ;
} ;


#if	(! defined(PDB_MASTER)) || (PDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	pdb_open(PDB *,cchar *,cchar *,cchar *,cchar *) ;
extern int	pdb_fetch(PDB *,char *,int,cchar *,cchar *) ;
extern int	pdb_check(PDB *,time_t) ;
extern int	pdb_close(PDB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PDB_MASTER */

#endif /* PDB_INCLUDE */


