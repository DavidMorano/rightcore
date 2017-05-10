REFERM


Synopsis :

	referm [-f argfile] [-p database1[,database2]] [file(s) [...]]

Options:
	-p dbfile[,dbfile]	bibliographical files
	-f argfile		list of files to process


Usage In Document :

.[[
> key words
what is this
%A other author
.]]


.RS
.[[ some key words
> key words
> more key words
%A other author
.]]
regular text added to the citation
.RF

.RS
.[[ key words
.]]
.RF



Revision History :


= Dave Morano, 
  97/01/10, version 0

1) I started looking at this version 1 series again.

2) I qualified alot of debugging printouts with the
   debug flag optionally specified on the command invocation.


= Dave Morano, 
  98/09/10, version 1a

** THE PLAN **

1) This version has been enhanced to work with the GNU version of
   the 'lookbib' program.  Also, it has been enhanced to also handle
   multiple identical responses from the Sun Solaris version of
   'lookbib'.




