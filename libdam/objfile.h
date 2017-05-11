/* objfile */

/* simulated program mapping manager */


/* revision history:

	= 2000-07-26, David A­D­ Morano
	This code was started.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	OBJFILE_INCLUDE
#define	OBJFILE_INCLUDE		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<elf.h>

#include	<vecobj.h>
#include	<hdb.h>
#include	<localmisc.h>


#define	OBJFILE_MAGIC		0x14253592
#define	OBJFILE			struct objfile_head
#define	OBJFILE_SYMTAB		struct objfile_symtab
#define	OBJFILE_SYMBOL		struct objfile_symbol
#define	OBJFILE_FL		struct objfile_flags
#define	OBJFILE_SNCUR		HDB_CUR


/* a single humble symbol */
struct objfile_symbol {
	Elf32_Sym	*ep ;
	cchar		*name ;
} ;

struct objfile_hash {
	caddr_t		pa_hash ;
	caddr_t		pa_dynsym ;
	caddr_t		pa_dynstr ;
	Elf32_Sym	*symtab ;	/* the symbol table */
	uint		*hashtab ;
	char		*strings ;
	uint		maplen_hash ;
	uint		maplen_dynsym ;
	uint		maplen_strtab ;
} ;

/* this is a symbol table, there can be many per object file ! */
struct objfile_symtab {
	caddr_t		pa_symtab ;	/* base address */
	caddr_t		pa_strings ;	/* base address */
	Elf32_Sym	*symtab ;	/* the symbol table */
	char		*strings ;	/* the string table */
	uint		maplen_symtab ;
	uint		maplen_strings ;
	uint		entsize ;	/* entry size */
	uint		nsyms ;		/* number of symbols */
	uint		type ;
} ;

struct objfile_flags {
	uint		symtab:1 ;	/* there were symbol tables */
	uint		symbols:1 ;	/* there are indexed symbols */
	uint		bss:1 ;		/* there was a BSS section */
	uint		hash:1 ;
	uint		dynsym:1 ;
} ;

struct objfile_head {
	uint		magic ;
	OBJFILE_FL	f ;
	OBJFILE_FL	h ;
	HDB		symbols ;	/* fast symbol access */
	vecobj		symtabs ;	/* symbol tables */
	cchar		*fname ;	/* ELF program file name */
	Elf32_Shdr	*sheads  ;
	time_t		lastaccess ;	/* last access time (informational) */
	ulong		pagealign ;	/* virtual page alignment */
	ulong		mapalign ;	/* alignment for mapping */
	ulong		progentry ;	/* program entry address */
	int		nsheads ;	/* number of sections */
	int		ofd ;		/* object file descriptor */
} ;


#if	(! defined(OBJFILE_MASTER)) || (OBJFILE_MASTER == 1)

#ifdef	__cplusplus
extern "C" {
#endif

extern int objfile_open(OBJFILE *,cchar *) ;
extern int objfile_getentry(OBJFILE *,uint *) ;
extern int objfile_getpagesize(OBJFILE *,uint *) ;
extern int objfile_getsym(OBJFILE *,cchar *,Elf32_Sym **) ;
extern int objfile_sncurbegin(OBJFILE *,OBJFILE_SNCUR *) ;
extern int objfile_sncurend(OBJFILE *,OBJFILE_SNCUR *) ;
extern int objfile_fetchsym(OBJFILE *,cchar *,OBJFILE_SNCUR *,Elf32_Sym **) ;
extern int objfile_enumsym(OBJFILE *,OBJFILE_SNCUR *,cchar **,Elf32_Sym **) ;
extern int objfile_getsec(OBJFILE *,int,Elf32_Shdr **) ;
extern int objfile_close(OBJFILE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(OBJFILE_MASTER)) || (OBJFILE_MASTER == 1) */

#endif /* OBJFILE_INCLUDE */


