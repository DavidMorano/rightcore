x-From: morano@acm.org (David Morano) 
From: morano@computer.org (David Morano) 
x-From: dmorano@ece.neu.edu (David Morano) 
x-reply-to: dam@rightcore.com
to: joe.meszaros@nii.net (Joe Meszaros)
x-cc: dell@nii.net (A. Morano) 
Subject: system call traces

For fun, I took some system call traces of two tiny programs.
The first program is :

=========================================================

#include	<stdio.h>


int main()
{


	fprintf(stdout,"hello world\n") ;

	return 0 ;
}

=========================================================

Incidentally, this is the mega-classic "hello world" program. :-)
The output of this program is :

hello world

I linked this program two ways.  The first way is compiled for only a
single possible thread of control in the program.  The system call
trace of this program compiled the first way (single thread only) is
included as attached file 'hello1.truss'.

I also linked it for possible multiple thread action.  The system
call trace for that is attached as file 'hello2.truss'.

I also did a second tiny program.  It is :

=========================================================

#include	<sys/types.h>
#include	<stdio.h>


extern int	timestr_log(time_t,char *) ;


int main()
{
	time_t	daytime ;

	char	timebuf[100] ;


	daytime = time(NULL) ;

	timestr_log(daytime,timebuf) ;

	fprintf(stdout,"%s\n",timebuf) ;

	return 0 ;
}

=========================================================

This is about the same as the classic "hello world" above but
includes the additional system call to get the time of day 'time()'
and also that subroutine 'timestr_log()' to convert it to an ASCII
string in the current time zone (not trivial by any means).
The output of this program is :

040114_1232:22

I linked this for multiple threads (the typical thing that is done)
and the system call trace is attached as file 'testdate.truss'.

Essentially in all of these system call traces, what we see is the
program boot loading itself for the most part.  Only the last two
or three system calls are what the programmer intended but the rest of
it is needed to get the program mapped into memory in the first place
and initialized to possibly split into multiple threads (unless it
was linked to forestall that -- single thread only).

FYI, some information on what some of the system calls do :

execve		system call used by parent program to create this one
stat		get status on a file
fstat		ditto
mmap		map part or all of a file into memory
munmap		unmap memory (from above)
open		open a file
close		close a file
time		get the system daytime in seconds since epoch
memctl		memory control
mprotect	setting type of memory protection (read, write, execute) 
lwp_XXX		various "light weight process" things (implements thread stuff)
sigXXX		setting various things with signals (process "interrupts")
door_XXX	communicating with other system daemon programs
brk		expanding heap memory space
sysconf		getting some system configuration information
exit		program exit


Except for 'lwp_XXX' and 'door_XXX' these are all standardized UNIX
system calls.  The 'lwp_XXX' is specific to how Solaris implements
their thread (multiple thread) stuff.  Also 'door_XXX' is specific (so
far) to how some system daemons communicate with each other and user
processes in Solaris.  The more standard way that most processes
communicate the sorts of things that 'door_XXX' now does in Solaris is
done using what is called an RPC (Remote Procedure Call -- also
developed by Sun but given to the world a long time ago) but done
locally only (which is optimized to be faster as expected).  But Sun
wanted something even faster than a local RPC for local communications
of this sort so they invented the "door."  I don't think that doors
have caught on with Linux yet (which is about the defining standard now
for true acceptance of a new thing ! :-).

Dave

