/*  hp.c  This file contains the extra support code required to run the */
/*        batchd under hp-ux.  It is mainly comprised of code which watches */
/*        the execution time of a set of processes on the system and detects */
/*        when one of batch jobs exceeds its aloted time and kills it. */
/* history:                                                            */
/*         Written by G. R. Strachan November 1991                  */
/*         Added code to keep track of data size Feb 1992   */
/*         Corrected bug in data size code to remove space of dead children */
/*         Corrected PruneProcs code if number of procs > HASHSIZE Jan 1993 */


/*  Copyright University of Waterloo 1992 */
/*  This code is provided as is, neither the University of Waterloo nor */
/*  the author is liable for any damage caused by the use or misuse of this */
/*  code.  */

/* Permission if granted to copy, use and modify this code provided it is */
/* not sold for profit and the above copyright remains in tack. */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/pstat.h>
#include <sys/resource.h>
#include <signal.h>
#include <stdio.h>

#include "bat_common.h"
#include "hp.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define HASHSIZE 512
#define NEW   0
#define ME    1
#define CHILD 2

#define muser1(who,fmt,a1)	{ sprintf(errstr,fmt,a1); muser(who, errstr);}
#define muser2(who,fmt,a1,a2) \
	{ sprintf(errstr,fmt,a1,a2); muser(who, errstr); }
#define muser3(who,fmt,a1,a2,a3) \
	{ sprintf(errstr,fmt,a1,a2,a3); muser(who, errstr); }
#define muser4(who,fmt,a1,a2,a3,a4) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4); muser(who, errstr); }
#define muser5(who,fmt,a1,a2,a3,a4,a5) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4,a5); muser(who, errstr); }
#define muser6(who,fmt,a1,a2,a3,a4,a5,a6) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4,a5,a6); muser(who, errstr); }
#define muser7(who,fmt,a1,a2,a3,a4,a5,a6,a7) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4,a5,a6,a7); muser(who, errstr); }
#define mdebug(str)	      { if (debug) muser(debugfile,str); }
#define mdebug1(str,a1)	      { if (debug) muser1(debugfile,str,a1); }
#define mdebug2(str,a1,a2)    { if (debug) muser2(debugfile,str,a1,a2); }
#define mdebug3(str,a1,a2,a3) { if (debug) muser3(debugfile,str,a1,a2,a3); }
#define mdebug4(str,a1,a2,a3,a4) \
	{ if (debug) muser4(debugfile,str,a1,a2,a3,a4); }
#define mdebug5(str,a1,a2,a3,a4,a5) \
	{ if (debug) muser5(debugfile,str,a1,a2,a3,a4,a5); }

struct WatchList
{
 int pid;                       /* the process ID of the process to watch */
 struct rlimit limits[RLIM_NLIMITS];  /* resource limits for the process */
 struct WatchList *next;        /* next element in the list */
};

struct ProcessTime
{
 struct ProcessTime *next;     /* pointer to next in hash chain */
 int Pid;                      /* its process id */
 int PPid;                     /* its parent pid */
 long ExecutionTime;           /* total execution for this process */
 long ChildTime;               /* total execution time for all its children */
 long DataSize;                /* size of this processes data segment */
 long ChildDSize;              /* total size of data segment for all children*/
 int WhoBuilt;                 /* whether node built by self or child */
 int LastSeen;                 /* when we last modified this process */
};

extern char *mymalloc();
extern muser();

extern debug;
char errstr[2048];
extern char *debugfile;

struct ProcessTime *FetchProcess();

struct WatchList *Watching = NULL;   /* the list of processes we are watching*/
int MaxProcs;          /* kernel limit for number of processs */
struct pst_status *Procs;   /* the currently running processes */
struct ProcessTime *HashTable[HASHSIZE];
int Round = 0;
long PageSize = 0;

/* AddHPWatch:  This function takes the process id of a child process and */
/*  the resource limits assigned to it.  If there are any resource limits */
/*  set, it adds it to its list of processes to monitor */

AddHPWatch(pid,limits)

int pid;
struct rlimit *limits;

{
 int i,j;
 struct WatchList *dummy;

 /* first check if there are any limits set on this process */
 mdebug1("In AddHPWatch for pid %d\n", pid);
 for(j = 0; j < RLIM_NLIMITS; j++)
  {
   mdebug2(" limits[%d].rlim_cur is %d\n", j, limits[j].rlim_cur);
   mdebug2(" limits[%d].rlim_max is %d\n", j, limits[j].rlim_max);
  }
 for(i = 0; i < RLIM_NLIMITS; i++)
  {
   if(((limits[i].rlim_cur != 0) && (limits[i].rlim_cur != -1) &&
    (limits[i].rlim_cur != RLIM_INFINITY)) ||
    ((limits[i].rlim_max != 0) && (limits[i].rlim_max != -1) &&
    (limits[i].rlim_max != RLIM_INFINITY)))
    {
     dummy = (struct WatchList *)mymalloc(sizeof(struct WatchList));
     dummy->pid = pid;
     for(j = 0; j < RLIM_NLIMITS; j++)
      {
       dummy->limits[j] = limits[j];
      }
     dummy->next = Watching;
     Watching = dummy;
     mdebug1("Watching is %8.8x\n", Watching);
     mdebug1("Added pid %d to Watchlist\n", Watching->pid);
     mdebug2("      limits[%d].rlim_cur was %d\n", i, limits[i].rlim_cur);
     mdebug2("      limits[%d].rlim_max was %d\n", i, limits[i].rlim_max);
     return;
    }
  }
 mdebug("No non-zero limits set for this pid - not added to Watchlist\n");
 mdebug1("Watching is %8.8x\n", Watching);
}

/* RmHPWatch:  This function takes the process id of a child process and */
/*  removes it from the list of processes it is monitoring */

RmHPWatch(pid)

int pid;

{
 struct WatchList *dummy = Watching;
 struct WatchList *previous = NULL;

 mdebug1("In RmHPWatch, Watching is %8.8x\n", Watching);
 mdebug1("              scanning for pid %d\n", pid);
 while(dummy != NULL)
  {
   if(dummy->pid == pid)
    {
     if(dummy == Watching)
      Watching = dummy->next;
     else
      previous->next = dummy->next;
     free(dummy);
     mdebug1("After list cleanup, Watching is %8.8x\n", Watching);
     return;
    }
   previous = dummy;
   dummy = dummy->next;
  }
 mdebug("RmHPWatch tried to remove a pid not in its watch list\n");
 mdebug1("Watching is %8.8x\n", Watching);
}

/* CheckHPRunTimes:  This function is the heart of this file.  It calls pstat*/
/*   to obtain a list of all currently running processes.  It then builds a */
/*   table of run times for parents and all their children.  Finally, it  */
/*   compares the total run times for the process in the watch list against */
/*   the table it just built.  If the time is greater than the limit then */
/*   it send kill signals to the process.  */

CheckHPRunTimes()

{
 static int Started = 0;
 int NProcs;
 union pstun pstatbuf;

 mdebug1("In CheckHPRunTimes, Watching is %8.8x\n", Watching);
 mdebug1("                    Started is %d\n", Started);
 if(Watching == NULL)
  return;    /* if there are no children don't waste time here */

 if(!Started)
  {
   InitialRunTimes();
   Started = 1;
  }
 
 pstatbuf.pst_status = Procs;
 if((NProcs = pstat(PSTAT_PROC,pstatbuf,sizeof(struct pst_status),MaxProcs,0))
    <= 0)
  error("CheckHPRunTimes failed to get process status");
 mdebug1("CheckHPRunTimes found %d processes running\n",NProcs);

 UpdateTimes(Procs,NProcs);   /* update the run time list */
 PruneProcs();                /* look for stale processes and remove them */
 CheckAndKill(Watching);      /* go through the watch list + kill some jobs */

 Round = (Round + 1) & 0x1;
 mdebug1("After CheckHPRunTimes, Watching is %8.8x\n", Watching);
}

/* InitialRunTimes:  This function is called the first time the list of */
/*  running processes is examined.  It figures out static kernel information */
/*  that is needed later such as the maximumu number of processes that */
/*  might be running and the size of a text page. */

InitialRunTimes()

{
 union pstun pstatbuf;
 struct pst_static sinfo;
 int i;

/* first thing we determine how many processes we can run to determine table */
/* sizes */

 pstatbuf.pst_static = &sinfo;
 if(pstat(PSTAT_STATIC,pstatbuf,sizeof(sinfo),0,0) == -1)
  error("InitialRunTimes failed in call to pstat for static info");

 MaxProcs = sinfo.max_proc;
 PageSize = sinfo.page_size;
 mdebug1("Will watch %d processes\n",MaxProcs);

 Procs = (struct pst_status *) mymalloc(sizeof(struct pst_status)*
					(MaxProcs + 1));
 for(i = 0; i < HASHSIZE; i++)
  HashTable[i] = NULL;
}

/* UpdateTimes;  This function takes the information on the processes */
/*  currently running on the system and converts it into its internal */
/*  representation.  When a process is encountered, this function will */
/*  also update the information in the parent's pid structure.  This is done */
/*  recursively for all parents.  If the parent has not yet been seen, then */
/*  parent structure is created and a flag is set to indicate this.  Then, */
/*  when the parent is finally encountered, the parent's resource's plus its */
/*  children will be propagated back up the parent list. */

UpdateTimes(Procs,NProcs)

struct pst_status *Procs;
int NProcs;

{
 int i;
 struct ProcessTime *ProcTime;
 long ttime;
 long diff_time;
 long diff_size;
 
 for(i = 0; i < NProcs; i++)
  {
   ProcTime = FetchProcess(Procs[i].pst_pid,1);
   if(ProcTime->WhoBuilt == NEW)  /* if it is new then fill in other info */
    {
     ProcTime->WhoBuilt = ME;
     ProcTime->PPid = Procs[i].pst_ppid;
    }
   ttime = Procs[i].pst_utime + Procs[i].pst_stime; /* calc our runtime */
   diff_time = ttime - ProcTime->ExecutionTime;
   ProcTime->ExecutionTime = ttime;
   diff_size = Procs[i].pst_dsize * PageSize - ProcTime->DataSize;
   ProcTime->DataSize = Procs[i].pst_dsize * PageSize;
   if(ProcTime->WhoBuilt == CHILD)
    ProcTime->PPid = Procs[i].pst_ppid;
   /*if(ProcTime->PPid != Procs[i].pst_ppid)
    error("UpdateTimes found a process that changed its parent!!");*/

/* if we have a parent inform it about our run time.  If this is our first */
/* time then we have to tell the parent about our total runtime, otherwise */
/* just tell it about the new run time to add.                             */

   if(ProcTime->PPid > 0)
    {
     if(ProcTime->WhoBuilt == CHILD)
      AddtoParent(ProcTime->PPid,
		  ProcTime->ExecutionTime + ProcTime->ChildTime,
		  ProcTime->DataSize + ProcTime->ChildDSize);
     else
      {
       if((diff_time > 0) || (diff_size > 0))
	AddtoParent(ProcTime->PPid,diff_time,diff_size);
       else
	mdebug("Skipping parent update\n");
      }
    }
   ProcTime->WhoBuilt = ME;
   ProcTime->LastSeen = Round;
   mdebug4("Done pid %d ppid %d (%ld %ld)\n",ProcTime->Pid,ProcTime->PPid,
	   ProcTime->ExecutionTime,ProcTime->ChildTime);
  }
}   

/* Recursively walk up the list of parents adding the new child runtime */

AddtoParent(Pid,Time,Size)

int Pid;
long Time;
long Size;

{
 struct ProcessTime *Proc;

 Proc = FetchProcess(Pid,1);
 if(Proc->WhoBuilt == NEW)
  Proc->WhoBuilt = CHILD;
 
 Proc->ChildTime += Time;
 Proc->ChildDSize += Size;
 mdebug3("Updated parent %d %ld %ld\n",Proc->Pid,Proc->ExecutionTime,
	 Proc->ChildTime);
 if(Proc->PPid > 0)
  AddtoParent(Proc->PPid,Time,Size);
 Proc->LastSeen = Round;
}

/* Recursively walk up the list of parents subtracting the child memory size */

RemoveFromParent(Pid,Size)

int Pid;
long Size;

{
 struct ProcessTime *Proc;

 Proc = FetchProcess(Pid,1);
 Proc->ChildDSize -= Size;
 mdebug2("Updated parent %d after removal %ld\n",Proc->Pid,Size);
 if(Proc->PPid > 0)
  RemoveFromParent(Proc->PPid,Size);
}

/* FetchProcess:  Extract the process structure from the hash table.  If it */
/*  doesn't exist we may create it. */

struct ProcessTime *FetchProcess(Pid,CreateFlag)

int Pid;
int CreateFlag;

{
 int Offset;
 struct ProcessTime *dummy = NULL;

 Offset = Pid % HASHSIZE;
 /*Offset = Pid & 0x0ff;*/
 dummy = HashTable[Offset];

 while(dummy != NULL)
  if(dummy->Pid == Pid)
   return(dummy);
  else
   dummy = dummy->next;

/* didn't find it, see if we should create one */

 if(CreateFlag)
  {
   dummy = (struct ProcessTime *) mymalloc(sizeof(struct ProcessTime));
   dummy->Pid = Pid;
   dummy->PPid = 0;
   dummy->ExecutionTime = dummy->ChildTime = 0;
   dummy->DataSize = dummy->ChildDSize = 0;
   dummy->LastSeen = Round;
   dummy->WhoBuilt = NEW;
   dummy->next = HashTable[Offset];
   HashTable[Offset] = dummy;
   mdebug2("Created hashtable entry for %d at %d\n",Pid,Offset);
  }
 return(dummy);
}

/* CheckAndKill: We walk through our list of processes we are watching and */
/*  check to see if they have exhausted their allowable resources.  If they */
/*  have then we send a SIGKILL to the entire process group.  Batchd will */
/*  then take care of cleaning up the process and mailing the results back */
/*  to the user. */
 
CheckAndKill(List)

struct WatchList *List;

{
 struct ProcessTime *Proc;
 long RunningTime;
 long MaxTime;
 long MaxSize;
 long TotalSize;
 while(List != NULL)
  {
   if((Proc = FetchProcess(List->pid,0)) != NULL) /* process might had died */
    {
     RunningTime = (Proc->ExecutionTime + Proc->ChildTime);
     MaxTime = MIN(List->limits[RLIMIT_CPU].rlim_cur,
		   List->limits[RLIMIT_CPU].rlim_max);
     mdebug3("Comparing process %d runtime %ld < %ld\n",List->pid,
	    RunningTime,MaxTime);
     if((MaxTime < RunningTime) && (MaxTime != -1))
      {
       mdebug1("Going to terminate process %d (too much time)\n",List->pid);
       kill(-List->pid,SIGKILL);
      }

     TotalSize = Proc->DataSize + Proc->ChildDSize;
     MaxSize = MIN(List->limits[RLIMIT_DATA].rlim_cur,
		   List->limits[RLIMIT_DATA].rlim_max);
     mdebug3("Comparing process %d data size %ld < %ld\n",List->pid,
	     TotalSize,MaxSize);
     if((MaxSize < TotalSize) && (MaxSize != -1))
      {
       mdebug1("Going to terminate process %d (data segment too big)\n",
	       List->pid);
       kill(-List->pid,SIGKILL);
      }
    }
   List = List->next;
  }
}

/* PruneProcs:  In order to keep the hash table from filling up we search the */
/*  hashtable for processes which are no longer running on the system and */
/*  remove them. */

PruneProcs()

{
 struct ProcessTime *current;
 struct ProcessTime *previous;
 int i;

 for(i = 0; i <HASHSIZE; i++)
  {
   current = previous = HashTable[i];
   while(current != NULL)
    {
     if(current->LastSeen != Round)
      {
       mdebug1("Removing process %d\n",current->Pid);
       RemoveFromParent(current->PPid,current->DataSize + current->ChildDSize);
       if(current == HashTable[i])
	{
	 HashTable[i] = current->next;
	 free(current);
	 current = previous = HashTable[i];
	}
       else
	{
	 previous->next = current->next;
	 free(current);
	 current = previous->next;
	}
      }
     else
      {
       previous = current;
       current = previous->next;
      }
    }
  }
}

/*
 * getdtablesize ()
 *
 * Returns the maximum number of file descriptors allowed.
 */

#include <unistd.h>

	int
getdtablesize ()
{
	return(sysconf(_SC_OPEN_MAX));
}

/*
 * setlinebuf (FILE *fp)
 *
 * Routine to set line buffering on "fp".
 */

	int
setlinebuf (FILE *fp)
{
	(void) setvbuf (fp, NULL, _IOLBF, 0);
	return(0);
}
