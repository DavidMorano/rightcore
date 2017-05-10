SUBMIT


The batch daemon can run several independent queues of batch jobs.  A
``batch job'' is a sequence of shell commands.  Users submit jobs with
the "batch" command.

Each batch queue has a set of attributes (defined in a profile file) to
control job processing, such as niceness, max concurrent jobs, etc.
Jobs can be stopped when the load average is too high, and restarted
when it falls.  Job execution can be controlled by time of day, or by
the exit status of a separate program.  Jobs can also be restarted
after a crash.  See the man pages for details.

Installation:

Edit Makefile, changing SPOOLDIR etc. to agree with your local arrangement.
The rest of these instructions use the symbolic names defined in Makefile.

To compile,
	make

Create SPOOLDIR, and set up whatever queues you need there.
You may want to use the sample queues in ./spool.example1 or
./spool.example2 at first:

	(cd spool.example1; tar cf - .) | (cd SPOOLDIR; tar xf -)

SPOOLDIR and its subdirectories must be owned by root and must not be
world-writeable.  The file SPOOLDIR/.README should contain a
description of the queues you have set up.

To install the binaries, do this as root:
	make install
To install the manual pages:
	make install-man

The queue logs will grow without bound, so you'll probably want to
run spool.example1/.cleanup once a week via cron.

Arrange that batchd is started at boot time, by
adding this to an appropriate place in /etc/rc.local or equivalent:

	if [ -d SPOOLDIR ]; then
		rm -f SPOOLDIR/*/tf* PIDFILE
		LIBDIR/batchd & (echo -n ' batch')		>/dev/console
	fi

Under certain error conditions, batchd sends mail to user "batch".
Add an appropriate alias, such as "batch:root",
to /usr/lib/aliases (or whatever) and run newaliases.

Start batchd (as root) and try it out:
	LIBDIR/batchd &
	sleep 5		# give batchd a chance to get going
	batch now -c "echo hello world"
	      ^^^ this is one of your queue names.

Under certain circumstances (unknown but probably related to problems
with 'exec'), batchd on Apollo systems will lose track of jobs that have
terminated. To recover from this situation without restarting batchd,
simply delete the input file (in CFDIR) and the output files from the
SPOOLDIR subdirectory for the affected queue. Batchd will then try to
re-sync itself internally next time it checks the queues.

Originally by Alex White, modified by lots of people at the
University of Waterloo.  Thanks to Mike Peterson for the Apollo support.
Thanks to Gordon Strachan for the HP-UX support.
This code is freely redistributable.

Bug reports to:
					Ken Lalonde
					Computer Science Dept.
					University of Toronto
					ken@cs.toronto.edu

* HP-UX notes from Gordon R. Strachan <gordon@maxwell.uwaterloo.ca>:

Some Unix operating systems do not have the ability to set an upper limit on
the run time of processes.  HP-UX is one of these.  This can make it difficult
to write a proper batch daemon.  The idea behind a batch daemon is to classify
jobs according to the resources they require so that they can be run when the
system can give them these resources.  However, if you can't enforce resource
limits on the jobs then you are left with hoping the users abide by the limits
you want them to use.  My experience that this doesn't work.

The solution I have come up with is to have the batch daemon actively monitor
the processes it is running to ensure they do not exceed the set resources.
Every few minutes the batch daemon will wake up and, if there are processes
running in the batch queue, it looks at every process that is running on the
system (the, at the moment, undocumented system call pstat is used for this).
The resources consumed by its process, and all its children are calculated.
Finally, the resources consumed by the batch processes are compared to the
limits set on them and, if they exceed the limits, they are terminated.

There are two drawbacks with this method.  Firstly, the batch daemon will
take up more runtime than it would otherwise.  However, on today's fast
computers, the extra runtime is hardly noticable.  The second problem is the
granularity of the checking.  By default, the batch daemon checks the system
every five minutes.  If a process running in the batch queue is creating a lot
of small child process which run quickly and exit, the batch daemon will miss
most of them and therefore get a incorrect estimate of the resource
consumption.  The solution is to use a finer granularity but this will increase
the load placed on the system by the batch daemon.

I have made one additional change to the batch system.  I have found that often
a person will submit many jobs at once to the batch queue, effectively hogging
the queue.  I modified the batch program to count the number of uncancelled
jobs a person has submitted into the queue and use that to reduce the
priority of the job.  Thus, more equitable access is given to the batch
queue.  However, there is one problem.  If a person submits a whole of jobs
at one time, the last jobs will have a low queue priority.  If then, another
person submits a job into the queue he will have a high priority and so will
run next.  If, as soon as his job finishes, the second person submits another
job it will again get a high priority and the lower priority jobs will not get
access to the CPU.  They will be starved.  I personally consider it
insentive to not submit lots of jobs at once but the proper solution is to
have either the batchd or batch process adjust the priorities of the jobs to
account for the aging of the job in the queue.

* Additional HP-UX notes by Mike Peterson <system@alchemy.chem.utoronto.ca>:

The C "system" call in HP-UX always returns status of -1 when used with
a SIGCHLD signal handler active (which is always the case for batchd!),
but an empirical test indicates that the real status of the process
run by "system" can be obtained from the "wait" system call - the
problem is that several child processes could be pending, and there is no
sure way to figure out which one belonged to the "system" call since
there is no way to know the pid(s) that were used. What I have done is
put some flags into batchd so that if a SIGCHLD is received after a
"system" call, and the pid doesn't belong to a job batchd thinks it is
running, the child exit status is returned as the "system" call status.
There are some checks so that only 1 extra pid will be accepted, and
only immediately after a "system" call, but this scheme could fail.


* Problem with RISCos and tcsh (Guntram Wolski):

	We found a weird one when running tcsh 6.00 on our MIPS boxes under
RISCos 4.52.  I have one user who tries the following:

% batch short
batch> echo hello
batch> ^d
% 

The jobs doesn't get run, but rather batch sends a message back that
free() had a problem with a badblock.memtop and some strange error
messages (I've since deleted the mail message).  Another user will
submit a job:

% batch short shellscript

and when they try to batchcancel the job only some of it is killed!
They are all in the same process group, but they don't seem to die...

Soooo, I changed batch.c ever so slightly around line 270:

	else if(strncmp(shell+strlen(shell)-3, BAT_CSHELL, 3) == 0) {
		shelltype = CSH;
		shell = "/bin/csh";  /* tcsh has problems sometimes, so just force it here */
	} else {

I didn't research this too far, I don't have the time to debug tcsh,
but at least for my environment I never have a problem.  You may or
may not wish to deal with this!  Just passing it on.

--G
--
Guntram Wolski                   gwolsk@sei.com
Silicon Engineering, Inc.        408-438-5331 x112
...!{sgiblab,ames}!seidc!gwolsk


* OSF/1 notes
	Edit batchqueue.sh, and make the changes
	indicated by the "OSF/1" comments.
	Thanks to Murray S. Kucherawy <mskucher@vertex.tor.hookup.net>
	for the OSF/1 port.
