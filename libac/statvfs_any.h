/* statvfs */



#ifndef STATVFS_INCLUDE
#define	STATVFS_INCLUDE		1


#if	(! defined(SYSHAS_STATVFS)) || (SYSHAS_STATVFS == 0)


#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Structure returned by statvfs(2).
 */

#ifndef	_FSTYPSZ
#define	_FSTYPSZ	16
#endif

#if !defined(_XPG4_2) || defined(__EXTENSIONS__)
#ifndef FSTYPSZ
#define	FSTYPSZ	_FSTYPSZ
#endif
#endif /* !defined(_XPG4_2) || defined(__EXTENSIONS__) */



#ifndef	_FILE_OFFSET_BITS
#define	_FILE_OFFSET_BITS	32
#endif


#if defined(_LP64) || (_FILE_OFFSET_BITS == 32)
typedef unsigned long		fsblkcnt_t;
typedef unsigned long		fsfilcnt_t;
#elif (_FILE_OFFSET_BITS == 64)
typedef unsigned long long	fsblkcnt_t;
typedef unsigned long long	fsfilcnt_t;
#endif

#if defined(_LARGEFILE64_SOURCE)
#ifdef _LP64
typedef	fsblkcnt_t		fsblkcnt64_t;
typedef	fsfilcnt_t		fsfilcnt64_t;
#else
typedef unsigned long long	fsblkcnt64_t;
typedef unsigned long long	fsfilcnt64_t;
#endif
#endif	/* _LARGEFILE64_SOURCE */




typedef struct statvfs {
	unsigned long	f_bsize;	/* fundamental file system block size */
	unsigned long	f_frsize;	/* fragment size */
	fsblkcnt_t	f_blocks;	/* total blocks of f_frsize on fs */
	fsblkcnt_t	f_bfree;	/* total free blocks of f_frsize */
	fsblkcnt_t	f_bavail;	/* free blocks avail to non-superuser */
	fsfilcnt_t	f_files;	/* total file nodes (inodes) */
	fsfilcnt_t	f_ffree;	/* total free file nodes */
	fsfilcnt_t	f_favail;	/* free nodes avail to non-superuser */
	unsigned long	f_fsid;		/* file system id (dev for now) */
	char		f_basetype[_FSTYPSZ];	/* target fs type name, */
						/* null-terminated */
	unsigned long	f_flag;		/* bit-mask of flags */
	unsigned long	f_namemax;	/* maximum file name length */
	char		f_fstr[32];	/* filesystem-specific string */
#if !defined(_LP64)
	unsigned long 	f_filler[16];	/* reserved for future expansion */
#endif
} statvfs_t;



#define	ST_RDONLY	0x01	/* read-only file system */
#define	ST_NOSUID	0x02	/* does not support setuid/setgid semantics */
#define	ST_NOTRUNC	0x04	/* does not truncate long file names */



 

#if defined(__STDC__)

int statvfs(const char *, statvfs_t *);
int fstatvfs(int, statvfs_t *);

/* transitional large file interface versions */
#if	defined(_LARGEFILE64_SOURCE) && !((_FILE_OFFSET_BITS == 64) && \
	    !defined(__PRAGMA_REDEFINE_EXTNAME))
int statvfs64(const char *, statvfs64_t *);
int fstatvfs64(int, statvfs64_t *);
#endif	/* _LARGEFILE64_SOURCE... */
#endif	/* defined(__STDC__) */


#ifdef	__cplusplus
}
#endif


#endif /* (! defined(SYSHAS_STATVFS)) || (SYSHAS_STATVFS == 0) */


#endif	/* _SYS_STATVFS_H */


