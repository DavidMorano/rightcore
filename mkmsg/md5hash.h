/* md5hash */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MD5HASH_INCLUDE
#define	MD5HASH_INCLUDE		1


#include	<sys/types.h>
#include	<md5.h>

#include	"localmisc.h"


#define	MD5HASH		struct md5hash_head


struct md5hash_head {
	ulong		magic ;
	MD5_CTS		ctx ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	md5hash_init(MD5HASH *) ;
extern int	md5hash_add(MD5HASH *,void *,int) ;
extern int	md5hash_get(MD5HASH *,ULONG *) ;
extern int	md5hash_free(MD5HASH *) ;

#ifdef	__cplusplus
extern "C" {
#endif

#endif /* MD5HASH_INCLUDE */


