#define VERSION  "3.0g"
/*
MODIFIED 19/10/92

	BWNFSD is an RPC daemon used in conjunction with BWNFS (a client
NFS for DOS based PCs. BWNFSD provides Authentication, Print Spooling,
DOS 3.1 Locking, DOS 3.1 Sharing, GID name mapping, UID name mapping
services for BWNFS and associated applications on the PC. BWNFSD is being
used also by Macintosh NFS clients.

	The BWNFSD code is originally copyright Beame & Whiteside Software
Ltd. and now is released to the Public Domain. The intent is that all server
vendors included a version of BWNFSD with their operating system. BWNFSD can
run simultanteously with PCNFSD, but provides more features.

	Please send modifications to:

		Beame & Whiteside Software Ltd.
		P.O. Box 8130
		Dundas, Ontario
		Canada L9H 5E7
		+1 (416) 765-0822

	Please modify by including "ifdefs" for your particular operating
system, with appropriate modifications to the "makefile".

BWLOCK.C provides the locking and sharing service for BWNFS. This should be
used if rpc.lockd does not include the extensions for DOS 3.1 locking or
sharing, or if your system has the regular distribution of the rpc.lockd as
the extensions most likely will not work.

One might want to make operating system calls to lock and unlock the files
so that locking occurs not only against other BWNFS clients, but native
clients as well.

MODIFICATION HISTORY
--------------------
31/07/92 fjw  Cleaned up the code a little
01/08/92 fjw  Merged in SVR4 code from Frank Glass (gatech!holos0!fsg)
19/10/92 fjw  Changed code to use some manifest constants
05/11/92 fjw  Punched up some of the debug output

*/

/*
 * HASH_LOCATION is a number between 0 and FHSIZE-1 inclusive. It should be
 * set to the position in the file handle FHSIZE byte structure which
 * changes the most between different files.
 */
#define HASH_LOCATION	17

#define LOCK_SHARE	20
#define LOCK_UNSHARE	21
#define LOCK_LOCK	22
#define LOCK_REMOVE_ALL 23
#define LOCK_UNLOCK	24
#define LOCK_BREAK_FH	101		/* break all locks held on FH */
#define LOCK_BREAK_IP	102		/* break all locks held by IP */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#if defined(SYSV32) || defined(SVR4)
#   define  memcmp(a,b,c) bcmp((b),(a),(c))
#   define  memcpy(a,b,c) bcopy((b),(a),(c))
#else
#   include <memory.h>
#endif

#ifdef INT_SIG
#define Void int
#define Return return(0)
#else
#define Void void
#define Return return
#endif

#define LOCK	'L'
#define SHARE	'S'
		      /*	      1st Open		   */
		      /* ALL	 DR	DW     DRW   COMP  */
		      /* I O B	I O B  I O B  I O B  I O B */
char shares[15][15] = { {1,1,1, 0,0,0, 1,1,1, 0,0,0, 0,0,0}, /* I */
			{1,1,1, 1,1,1, 0,0,0, 0,0,0, 0,0,0}, /* O  ALL */
			{1,1,1, 0,0,0, 0,0,0, 0,0,0, 0,0,0}, /* B */
			{0,1,0, 0,0,0, 0,1,0, 0,0,0, 0,0,0}, /* I */
			{0,1,0, 0,1,0, 0,0,0, 0,0,0, 0,0,0}, /* O  DR */
			{0,1,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0}, /* B */
			{1,0,0, 0,0,0, 1,0,0, 0,0,0, 0,0,0}, /* I */
			{1,0,0, 1,0,0, 0,0,0, 0,0,0, 0,0,0}, /* O DW 2nd Open */
			{1,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0}, /* B */
			{0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0}, /* I */
			{0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0}, /* O  DRW */
			{0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0}, /* B */
			{0,0,0, 0,0,0, 0,0,0, 0,0,0, 1,1,1}, /* I */
			{0,0,0, 0,0,0, 0,0,0, 0,0,0, 1,1,1}, /* O  COMP */
			{0,0,0, 0,0,0, 0,0,0, 0,0,0, 1,1,1}  /* B */
};

struct LK {
	char type;
	struct LK *nxt;
	struct LK *prv;
	struct LK *nxt_fh;
	struct LK *prv_fh;
	struct FH *fh;
	struct PC *pc;
	union {
		struct {
			char mode;
			char access;
		} s;
		struct {
			unsigned long offset;
			unsigned long length;
		} l;
	} lock;
};

#define FHSIZE	32
struct FH {
	char fh[FHSIZE];
	struct LK *lock;
	struct FH *nxt;
	struct FH *prv;
};

struct PC {
	char *owner;
	struct PC *nxt;
	struct PC *prv;
	struct LK *lock;
	unsigned long last_cookie;
	int last_result;
};

#define NUM_PCS 256
#define NUM_FHS NUM_PCS

struct PC *pcs[NUM_PCS];
struct FH *fhs[NUM_FHS];
extern int debugmode;

char *
requestname(req)
	int req;
{
static char buff[100];

	switch(req) {
	case LOCK_SHARE:
		strcpy(buff, "Share file");
		break;
	case LOCK_UNSHARE:
		strcpy(buff, "UnShare file");
		break;
	case LOCK_LOCK:
		strcpy(buff, "Lock file");
		break;
	case LOCK_REMOVE_ALL:
		strcpy(buff, "Remove all locks on file");
		break;
	case LOCK_UNLOCK:
		strcpy(buff, "UnLock file");
		break;
	case LOCK_BREAK_FH:
		strcpy(buff, "Break all Locks/Shares on file");
		break;
	case LOCK_BREAK_IP:
		strcpy(buff, "Break all Locks/Shares held by owner");
		break;
	default:
		sprintf(buff, "Unknown request - %d", req);
		break;
	}
	return(buff);
}

char *
accessname(acc)
	int acc;
{
static char buff[30];

	switch(acc) {
	case 1:
		strcpy(buff, "Input");
		break;
	case 2:
		strcpy(buff, "Output");
		break;
	case 3:
		strcpy(buff, "Input/Output");
		break;
	default:
		sprintf(buff, "Unknown access value - %d", acc);
		break;
	}
	return(buff);
}

char *
modename(mode)
	int mode;
{
static char buff[30];

	switch(mode) {
	case 0:
		strcpy(buff, "Deny None");
		break;
	case 1:
		strcpy(buff, "Deny Read");
		break;
	case 2:
		strcpy(buff, "Deny Write");
		break;
	case 3:
		strcpy(buff, "Deny Read/Write");
		break;
	case 4:
		strcpy(buff, "Compatibility");
		break;
	default:
		sprintf(buff, "Unknown mode value - %d", mode);
		break;
	}
	return(buff);
}

char
*my_strdup(p)
	char *p;
{
char *p1;

	if ((p1=(char *) malloc(strlen(p)+1)) == NULL)
		return(NULL);
	return(strcpy(p1,p));
}

Void
display_locks(unused)
	int unused;
{
int i, j;
struct PC *pc1;
struct LK *lk1;
struct FH *fh1;
FILE *lock_file;

#ifdef SIGUSR1
	(void) signal(SIGUSR1, display_locks);
#endif
	if ((lock_file=fopen("/etc/bwlocks.dmp","w")) == NULL)
		Return;
	for (i=0; i<NUM_PCS; i++) {
		pc1 = pcs[i];
		while (pc1 != NULL) {
			j = 0;
			fprintf(lock_file,"%3d %s -> ", i, pc1->owner);
			lk1 = pc1->lock;
			while (lk1 != NULL) {
				fprintf(lock_file, "%c/%3u/", lk1->type, (lk1->fh)->fh[HASH_LOCATION] & 0xff);
				if (lk1->type == SHARE) {
					fprintf(lock_file, "%1d -> ", lk1->lock.s.mode);
					j += 11;
				} else {
					fprintf(lock_file, "%3ld+%3ld -> ", lk1->lock.l.offset, lk1->lock.l.length);
					j += 17;
				}
				if (j > 70) {
					fprintf(lock_file, "\n                ");
					j = 0;
				}
				lk1 = lk1->nxt;
			}
			fprintf(lock_file, "NULL\n");
			pc1 = pc1->nxt;
		}
	}
	fprintf(lock_file, "File Handles\n");
	for (i=0; i<NUM_FHS; i++) {
		fh1 = fhs[i];
		while (fh1 != NULL) {
			fprintf(lock_file, "%3d ", i);
			for (j=0; j<FHSIZE; j++)
			       fprintf(lock_file, "%02X", fh1->fh[j] & 0xff);
			fprintf(lock_file, " -> ");
			j = 0;
			lk1 = fh1->lock;
			while (lk1 != NULL) {
				fprintf(lock_file, "%c/%3u/",lk1->type,*(lk1->pc)->owner & 0xff);
				if (lk1->type == SHARE) {
					fprintf(lock_file, "%1d -> ", lk1->lock.s.mode);
					j += 11;
				} else {
					fprintf(lock_file, "%3ld+%3ld -> ", lk1->lock.l.offset, lk1->lock.l.length);
					j += 17;
				}
				if (j > 70) {
					fprintf(lock_file, "\n                ");
					j = 0;
				}
				lk1 = lk1->nxt_fh;
			}
			fprintf(lock_file, "NULL\n");
			fh1 = fh1->nxt;
		}
	}
	fclose(lock_file);
	Return;
}

void
init_locks()
{
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [init_locks] called\n");
	for (i=0; i<NUM_PCS; i++)
		pcs[i] = NULL;
	for (i=0; i<NUM_FHS; i++)
		fhs[i] = NULL;
#ifdef	SIGUSR1
	(void) signal(SIGUSR1, display_locks);
#endif
}

#define TRUE  1
#define FALSE 0

static int pc_ptr, fh_ptr;

struct PC *
locate_by_owner(owner)
	char *owner;
{
struct PC *p;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [locate_by_owner] called for %s\n", owner);
	p = pcs[pc_ptr];
	while (p != NULL) {
		if (debugmode)
			fprintf(stdout, "bwnfsd: [locate_by_owner] owner is %s\n", p->owner);
		if (strcmp(p->owner,owner) == 0)
			return(p);
		else
			p = p->nxt;
	}
	return(p);
}

struct FH *
locate_by_handle(fh)
	char *fh;
{
struct FH *p;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [locate_by_handle] called\n");
	p = fhs[fh_ptr];
	while (p != NULL)
		if (memcmp(p->fh, fh, FHSIZE) == 0)
			return(p);
		else
			p = p->nxt;
	return(p);
}

int
process_lock_request(request, owner, fh, access, mode, offset, length, cookie)
	int request;
	char *fh, *owner;
	int access, mode;
	unsigned long length, offset;
	unsigned long cookie;
{
struct PC *pc1;
struct LK *lk1;
struct LK *lk2;
struct FH *fh1;
int j, len;

	if (debugmode) {
		fprintf(stdout, "bwnfsd: [process_lock_request] called with\n");
		fprintf(stdout, "bwnfsd: request     = %s\n", requestname(request));
		fprintf(stdout, "bwnfsd: owner       = %s\n", owner);
		fprintf(stdout, "bwnfsd: file_handle = ");
		for (j=0; j<FHSIZE; j++)
			fprintf(stdout, "%02X", fh[j] & 0xff);
		fprintf(stdout,"\n");
		fprintf(stdout, "bwnfsd: access      = %s\n", accessname(access));
		fprintf(stdout, "bwnfsd: mode        = %s\n", modename(mode));
		fprintf(stdout, "bwnfsd: offset      = %lu\n", offset);
		fprintf(stdout, "bwnfsd: length      = %lu\n", length);
		fprintf(stdout, "bwnfsd: cookie      = %lu\n", cookie);
	}
	if (owner == NULL)
		len = 0;
	else
		len = strlen(owner);
	if (len == 0)
		pc_ptr = 0;
	else
		if (len == 1)
			pc_ptr = *(owner+len-1) & 0xff;
		else
			pc_ptr = (*(owner+len-1) ^ *(owner+len-2)) & 0xff;

	if (request != LOCK_REMOVE_ALL) {
		if ((pc1=locate_by_owner(owner)) == NULL) {
			if ((pc1=(struct PC *) malloc(sizeof(struct PC))) == NULL)
				return(FALSE);
			pc1->owner = my_strdup(owner);
			pc1->lock = NULL;
			pc1->prv = NULL;
			pc1->nxt = pcs[pc_ptr];
			pc1->last_cookie = cookie-1;
			pcs[pc_ptr] = pc1;
			if ((pc1->nxt) != NULL)
				(pc1->nxt)->prv = pc1;
		}
		if (pc1->last_cookie == cookie)
			return(pc1->last_result);
		pc1->last_cookie = cookie;
	}
	switch(request) {
	case LOCK_SHARE:
		fh_ptr = fh[HASH_LOCATION] & 0xff;

		if ((fh1=locate_by_handle(fh)) == NULL) {
			if ((fh1=(struct FH *) malloc(sizeof(struct FH))) == NULL) {
				pc1->last_result = FALSE;
				return(FALSE);
			}
			(void) memcpy(fh1->fh,fh,FHSIZE);
			fh1->lock = NULL;
			fh1->prv = NULL;
			fh1->nxt = fhs[fh_ptr];
			fhs[fh_ptr] = fh1;
			if ((fh1->nxt) != NULL)
			       (fh1->nxt)->prv = fh1;
		}
		lk1 = fh1->lock;
		while (lk1 != NULL) {
			if (lk1->type == SHARE) {
				if (!shares[mode*3+access-1][lk1->lock.s.mode*3+lk1->lock.s.access-1]) {
					pc1->last_result = FALSE;
					return(FALSE);
				}
			}
			lk1 = lk1->nxt_fh;
		}
		if ((lk1=(struct LK *) malloc(sizeof(struct LK))) == NULL) {
			pc1->last_result = FALSE;
			return(FALSE);
		}
		lk1->fh = fh1;
		lk1->pc = pc1;
		lk1->type = SHARE;
		lk1->lock.s.mode = mode;
		lk1->lock.s.access = access;
		lk1->prv = NULL;
		lk1->prv_fh = NULL;
		if ((lk1->nxt=pc1->lock) != NULL)
			(lk1->nxt)->prv = lk1;
		pc1->lock = lk1;
		if ((lk1->nxt_fh=fh1->lock) != NULL)
			(lk1->nxt_fh)->prv_fh = lk1;
		fh1->lock = lk1;
		pc1->last_result = TRUE;
		return(TRUE);
	case LOCK_UNSHARE:
		fh_ptr = fh[HASH_LOCATION] & 0xff;
		if ((fh1=locate_by_handle(fh)) == NULL) {
			pc1->last_result = TRUE;
			return(TRUE);
		}
		lk1 = fh1->lock;
		while (lk1 != NULL) {
			if ((lk1->pc == pc1) &&
			    (lk1->type == SHARE) &&
			    (mode == lk1->lock.s.mode) &&
			    (access == lk1->lock.s.access))
			{
				if ((lk1->prv_fh) == NULL) {
					if ((fh1->lock=lk1->nxt_fh) == NULL) {
						if (fh1->prv == NULL) {
							if ((fhs[fh_ptr]=fh1->nxt) != NULL)
								(fh1->nxt)->prv = NULL;
						} else {
							if (((fh1->prv)->nxt=fh1->nxt) != NULL)
								(fh1->nxt)->prv = fh1->prv;
						}
						free(fh1);
					} else
						(fh1->lock)->prv_fh = NULL;
				} else {
					if (((lk1->prv_fh)->nxt_fh=lk1->nxt_fh) != NULL)
						(lk1->nxt_fh)->prv_fh = lk1->prv_fh;
				}
				if ((lk1->prv) == NULL) {
					if ((pc1->lock=lk1->nxt) == NULL) {
						if (pc1->prv == NULL) {
							if ((pcs[pc_ptr]=pc1->nxt) != NULL)
								(pc1->nxt)->prv = NULL;
						} else {
							if (((pc1->prv)->nxt=pc1->nxt) != NULL)
								(pc1->nxt)->prv = pc1->prv;
						}
						free(pc1->owner);
						free(pc1);
						pc1 = NULL;
					} else
						(pc1->lock)->prv = NULL;
				} else {
					if (((lk1->prv)->nxt=lk1->nxt) != NULL)
						(lk1->nxt)->prv = lk1->prv;
				}
				free(lk1);
				if (pc1 != NULL)
					pc1->last_result = TRUE;
				return(TRUE);
			}
			lk1 = lk1->nxt_fh;
		}
		if (pc1 != NULL)
			pc1->last_result = TRUE;
		return(TRUE);
	case LOCK_LOCK:
		fh_ptr = fh[HASH_LOCATION] & 0xff;
		if ((fh1=locate_by_handle(fh)) == NULL) {
			if ((fh1=(struct FH *) malloc(sizeof(struct FH))) == NULL) {
				pc1->last_result = FALSE;
				return(FALSE);
			}
			(void) memcpy(fh1->fh,fh,FHSIZE);
			fh1->lock = NULL;
			fh1->prv = NULL;
			fh1->nxt = fhs[fh_ptr];
			fhs[fh_ptr] = fh1;
			if ((fh1->nxt) != NULL)
				(fh1->nxt)->prv = fh1;
		}
		lk1 = fh1->lock;
		while (lk1 != NULL) {
			if (lk1->type == LOCK)
				if (((lk1->lock.l.offset >= offset) &&
				      (offset+length > lk1->lock.l.offset)) ||
				     ((lk1->lock.l.offset < offset) &&
				      (lk1->lock.l.offset + lk1->lock.l.length > offset)))
				{
					pc1->last_result = FALSE;
					return(FALSE);
				}
			lk1 = lk1->nxt_fh;
		}
		if ((lk1=(struct LK *) malloc(sizeof(struct LK))) == NULL) {
			pc1->last_result = FALSE;
			return(FALSE);
		}
		lk1->fh = fh1;
		lk1->pc = pc1;
		lk1->type = LOCK;
		lk1->lock.l.offset = offset;
		lk1->lock.l.length = length;
		lk1->prv = NULL;
		lk1->prv_fh = NULL;
		if ((lk1->nxt=pc1->lock) != NULL)
			(lk1->nxt)->prv = lk1;
		pc1->lock = lk1;
		if ((lk1->nxt_fh=fh1->lock) != NULL)
			(lk1->nxt_fh)->prv_fh = lk1;
		fh1->lock = lk1;
		pc1->last_result = TRUE;
		return(TRUE);
	case LOCK_REMOVE_ALL:
		if ((pc1=locate_by_owner(owner)) == NULL)
			return(TRUE);
		if (pc1->prv == NULL) {
			if ((pcs[pc_ptr]=pc1->nxt) != NULL)
				(pc1->nxt)->prv = NULL;
		} else {
			if (((pc1->prv)->nxt=pc1->nxt) != NULL)
				(pc1->nxt)->prv = pc1->prv;
		}
		lk1 = pc1->lock;
		while (lk1 != NULL) {
			if ((lk1->prv_fh) == NULL) {
				fh1 = lk1->fh;
				if ((fh1->lock=lk1->nxt_fh) == NULL) {
					if (fh1->prv == NULL) {
						fh_ptr = fh1->fh[HASH_LOCATION] & 0xff;
						if ((fhs[fh_ptr]=fh1->nxt) != NULL)
							(fh1->nxt)->prv = NULL;
					} else {
						if (((fh1->prv)->nxt=fh1->nxt) != NULL)
							(fh1->nxt)->prv = fh1->prv;
					}
					free(fh1);
				} else
					(fh1->lock)->prv_fh = NULL;
			} else
				if (((lk1->prv_fh)->nxt_fh=lk1->nxt_fh) != NULL)
					(lk1->nxt_fh)->prv_fh = lk1->prv_fh;
			lk2 = lk1->nxt;
			free(lk1);
			lk1 = lk2;
		}
		free(pc1->owner);
		free(pc1);
		return(TRUE);
	case LOCK_UNLOCK:
		fh_ptr = fh[HASH_LOCATION] & 0xff;
		if ((fh1=locate_by_handle(fh)) == NULL) {
			pc1->last_result = TRUE;
			return(TRUE);
		}
		lk1 = fh1->lock;
		while (lk1 != NULL) {
			if ((lk1->pc == pc1) &&
			    (lk1->type == LOCK) &&
			    (offset == lk1->lock.l.offset) &&
			    (length == lk1->lock.l.length))
			{
				if ((lk1->prv_fh) == NULL) {
					if ((fh1->lock=lk1->nxt_fh) == NULL) {
						if (fh1->prv == NULL) {
							if ((fhs[fh_ptr]=fh1->nxt) != NULL)
								(fh1->nxt)->prv = NULL;
						} else {
							if (((fh1->prv)->nxt=fh1->nxt) != NULL)
								(fh1->nxt)->prv = fh1->prv;
						}
						free(fh1);
					} else
						(fh1->lock)->prv_fh = NULL;
				} else {
					if (((lk1->prv_fh)->nxt_fh=lk1->nxt_fh) != NULL)
						(lk1->nxt_fh)->prv_fh = lk1->prv_fh;
				}
				if ((lk1->prv) == NULL) {
					if ((pc1->lock=lk1->nxt) == NULL) {
						if (pc1->prv == NULL) {
							if ((pcs[pc_ptr]=pc1->nxt) != NULL)
								(pc1->nxt)->prv = NULL;
						} else {
							if (((pc1->prv)->nxt=pc1->nxt) != NULL)
								(pc1->nxt)->prv = pc1->prv;
						}
						free(pc1->owner);
						free(pc1);
						pc1 = NULL;
					} else
						(pc1->lock)->prv = NULL;
				} else {
					if (((lk1->prv)->nxt=lk1->nxt) != NULL)
						(lk1->nxt)->prv = lk1->prv;
				}
				free(lk1);
				if (pc1 != NULL)
					pc1->last_result = TRUE;
				return(TRUE);
			}
			lk1 = lk1->nxt_fh;
		}
		if (pc1 != NULL)
			pc1->last_result = TRUE;
		return(TRUE);
	}
	if (pc1 != NULL)
		pc1->last_result = FALSE;
	return(FALSE);
}
