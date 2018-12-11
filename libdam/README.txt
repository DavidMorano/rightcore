LIBDAM


module		description
-----------------------------------------------------------------------

vecstr		vector strings, stores the data for you
vecpstr		vector of packed strings
vecitem		variable sized items, stores the data for you
vecobj		fixed sized objects, stores the data for you
vechand		you have to store the data yourself 
vecint		integers, stores data
veclong		long integers
vecelem		vector of structures
recarr		record-arry (variable vector) of pointer handles

hdb		you have to store the data yourself
hdbstr		key-value strings, stores data for you
mapstrint	map of strings to integers, stores data
mapstrs		map of a string key with a string value
setint		set of integers
setstr		set (unique) of strings
osetstr		ordered set (unique) of string
vsetstr		another ordered set (unique) of strings

bits		dynamic (dynamically growable) bit array 

varray		dynamically populated array

fsdir		UNIX FS directory functions (object)
dirtree		UNIX FS directory walk functions using an object
wdt		UNIX FS directory walk function similar to 'ftw'

buffer		variable length object continuous buffer management
bufstr		variable length object continuous buffer management (cheapy?)
sbuf		fixed length object continuous buffer management
sbuf_xxx	specialty variants
strmgr		like SBUF but simpler (generally used internally)
storeitem	fixed length object buffer management of separate items
serialbuf	fixed length object buffer management of joined items
outbuf		some whacky buffer-storage thing!
outstore	this is similar (or essentially exactly the same as) BUFSTR
storebuf	fixed length non-object continuous buffer management

strtab		string-table generating object
strstore	string storage object
strpack		a simple string-packing object (lighter weight than STRSTORE)
strop		some (relatively) simple operations on counted strings

netorder	non-object specialized for individual network-order items
stdorder	non-object specialized for individual standard-order items

raqhand		Random-Access-Queue handler (see the code)
fifostr		FIFO object for strings, stores data
fsi		FIFO object for strings, stores data, thread-safe
fifoitem	FIFO object for variable sized items
q		self-relative Q, relocatable-head, thread-safe
piq		pointer Q, relocatable-head, count, magic, thread-safe
aiq		self-relative Q, thread-safe, async, magic, count
plainq		plain self-relative Q, count, and magic
cpq		circular pointer Q (huge in the old days w/ OS stuff!)
pq		pointer Q, relocatable-head, not-circular, count
cq		container Q, relocatable-head, count, magic
ciq		container Q, relocatable-head, count, magic, thread-safe
charq		character Q, relocatable-head, count
chariq		character Q, relocatable-head, count, thread-safe
intiq		integer Q, relocatable-head, count, thread-safe

fmq		file-message queue
pmq		POSIX message queue

psem		POSIX semaphore
csem		Counting-Semaphore (general counting semaphore)
ucsem		UNIX Counting-Semaphore

getenv2
getenv3
getourenv	get environment variable from local string-pointer array
getpwd
getpwusername	replacement for 'getpwnam'
getpwlogname	replacement for 'getpwnam'
getlogname	replacement for 'getlogin'
getgroupname	get curret user group-name
getutempent	UTMPX functions
getutemterm	UTMPX by terminal-name
getnodename	get the current node-name (will use environment)
getdomainname	get the INET domain-name (will use environment)
getnodedomain	get the node-name and the INET domain (will use environment)
getprojname	get user project-name
getclustername	get the cluster for the current node
getserial	get a serial number from a file-db
nisdomainname	get the NIS domain name

getax		access UNIX system account (PW, SH, GR, PJ) databases
gethe		access UNIX system HE (host-entry) database
getne		access UNIX system NE (network-entry) database
getpe		access UNIX system PE (protocol-entry) database
getse		access UNIX system SE (service-entry) database

getcanonical	get host canonical name
getchost
getchostname
getcname
getehostname	get externded host name

gethe1
getheaddr
gethostaddr

getfiledirs	find directories
findfile	find file
findfilepath	find file
findxfile	find eXexecuable file

mailmsgatt	mail message attachments
mailmsg		an object of the general message headers
mailmsgattent	mail-message attachment handing
mailmsgenv	mail-message environment handling
mailmsghdrfold	mail-message header folding
mailmsghdrs	mail-message header management
mailmsghdrval	mail-message header value management
mailmsgheadkey	mail-message header-key matching
mailmsgmatenv	mail-message environment matching
mailmsgmathdr	mail-message header matching
mailmsgstage	mail staging object
mailbox		mail-box object
mailalias	mail alias db access
msgheaders	a small object to do something on popular message headers

sigign		signal management object
sigblock	signal management object
sighand		signal management object
sigman		signal management object

streamsync	acquire stream character synchronization
dstr		string object

strcpylc	copy string to lower case
strcpyuc	copy string to upper case
strcpyfc	copy string to folded case

strncpylc	copy to lower case and flll out with NULs
strncpyuc	copy to upper case and fill out with NULs
strncpyfc	copy to folded case and fill out with NULs

strwcpy		copy a maxium length string to another
strwcpylc	to lower case
strwcpyuc	to upper case
strwcpyfc	to foled case
strwcpyrev	to reversed sequence of source

strljoin	join strings to a destination (w/ destination length)

strnchr		same as 'strchr()' but string has specified length
strnrchr	same as 'strrchr()' but string has specified length
strnpbrk	same as 'strpbrk()' but string has specified length
strwhite	same as 'strpbrk(s," \v\t\r\n")
strnncpy	special copy of strings with length and maxium
strsub		find a substring in a string
strtoken	reentrant version of 'strtok()' (UNIX has something now)

strnlen		get the length of a string w/ a specified maximum

sifield		string-index to field-separator or end
sichr		string-index to character
sisub		string-index to sub-string
sicasesub	string-index tp sub-string (case independent)
sibreak		string-index to break characters
sispan		string-index past spanning characters
sibasename	string-index to basename
siskipwhite	string-index skipping over white-space
sihyphen	string-index to a hyphen ('--')
sialnum		string-index to alpha-numeric character
sialpha		string-index to alpha character
sidquote	string-index to the end of double-quote string
sicite		string-index to a "citation" escape
silbrace	string-index to a left-brace character after white-space

wsichr		wide-string find character index
wsirchr		wide-string find reverse character index
wsinul		wide-string find NUL character index
wsnlen		wide-string length (bounded)
wslen		wide-string length
wscpynarrow	wide-string copy narrow string into it
wsncols		wide-string find columns in bounded string

sfnext		string-find next
sfword		string-find word
sfwhitedot	string-find white-space and dot
sfthing		string-find special token
strsubstance	string-find substantive core character sub-string
sfsub		string-find sub-string
sfstr		string-find ?
sfskipwhite	string-find skip white-space
sfshrink	string-find shrunken core sub-string
sfrootname	string-find root name
sfprogroot	string-find program root
sfnamecomp	string-find ?
sflast		string-find last
sfkey		string-find key
sfbasename	string-find base-name
sfdirname	string-find dir-name
sfbaselib	string-find base library name
sfdequote	string-find de-quoted sub-string
sfcookkey	string-find cook-key
sfcenter	string-find center ?
sfcasesub	string-find case-insensitive sub-string
sfbreak		string-find break character
sfbracketval	string-find backet value

wsfnext		wide-string find next field

strjoin		join two strings, returns pointer to next character position
strjoin2	join two strings
strjoin3	join three strings

strdcpy1	copy to a "destination" string (has address and length)
strdcpy2
strdcpy3

strshrink	shrink off white space from a string
strdirname	find dirname of a directory file path
strbasename	find basename of a directory file path
strdomain	INET domain thing
strftime	same as UNIX (now standard) ?

strkeycmp	compare key parts of two strings
strnkeycmp	compare key parts of strings
strnncmp	special compare of strings with length and maximum
strpcmp		prefix comparison of strings

strlead		compare leading parts of strings
strnlead	compare leading parts of strings

snwcpy
snwcpylc	to lower case
snwcpyul	to upper case
snwcpyfl	to folded case

sncpy1
sncpy2
sncpy3
sncpy4
sncpy5
sncpylc		to lower case
sncpyuc		to upper case
sncpyfc		to folded case

sncat1
sncat2

mkpath1
mkpath2
mkpath3
mkpath4
mkpath5

mkfname2

mkfnamesuf1
mkfnamesuf2

mkpr		make (find) a program root directory
mkbangname	make a "bang" ("n!u") name
mkbestname	make the best real name we can
mkmailname	make a name suitable for use as a mail-address (PCS facility)
mklogid		make a log-id (for logging)
mkplogid	make a log-id (with some differene than 'mklogid')
mkmsgid		make a MSG ID
mkutmpid	make an ID suitable for use in UTMP databases

mktmpfile	make a temporary file
mkjobfile	make a temporary file (suitable as a job-name)
mkdatefile	make a temporary file (suitable for a date thing of some sort)

randlc		Linear Congruent Random Nmmber Generator
random		UNIX® System |random(3c)| but made into an object
randomvar	a new high-randomness random number generator
randmwc		random number generator (Multiply W/ Carry)

mallocstr	malloc()'s a buffer the size of a supplied string
mallocstrn	malloc()'s a buffer the size of a supplied string of MAX length
mallocbuf	malloc()'s a buffer of the size specified

strlist		manage a STRLIST database file

svcfile		service-table file
kvsfile		key-value file
sysvar		constant database file
searchkey	object of search keys
schedvar	schedule variables

sesnotes	UNIX session notes
sesmsg		UNIX session messages

pcsconf		PCS constant database file
pcspoll		PCS user-mode polling manager
pcspolls	PCS poll loadable modules

sfisterm	safe-fast is-a-terminal


= C++ language (up to C++11) subroutines or objects

bellmanford1	Bellman-Ford algorithm-1
bellmanford2	Bellman-Ford algorithm-2
bfs1		breadth first search
bstree		Binary-Search-Tree object (slightly more useful than the STL)
ctwords.hh	Convert-to-Words (also see program NUMCVT)
dfs1.hh		Depth-First-Search algorithm-1
dfs2.hh		Depth-First-Search algorithm-2
dijkstra1.hh	Dijkstra shorted-path-in-graph, algorithm-1
dijkstra2.hh	Dijkstra shorted-path-in-graph, algorithm-2
graph.hh	some graph thing?
minmaxelem.hh	find maximum and minimum in a range over a list-like object
obuf.hh		ourput-buffer (used interanlly in some point solution things)
returnstatus.hh	manage return status (I guess for some point soluations
singlist.hh	a single-link list (not brain-damaged and better than STL)
sort_insertion.hh	an insertion sort (for C++)
sort_merge.hh		a merge sort (for C++)
willAddOver.hh		??


