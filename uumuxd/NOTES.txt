UUMUXD


The program invocation should look like :

$ uumuxd [-f from] [-j jobid] [-e[=type]] [-k=key] [-c[=type]] 
	[-s session] service [srvargs]


The format of an incoming UUMUX job has three parts, the file ID part
is first.  Then there are two (possibly repeating) other parts.  These
are an outer part and an inner part.  

The file ID part looks like :

UUMUX <version>


The outer part looks like :

session <session_key>
jobid <jobid>
from <fromaddress>
compressed gzip
encrypted descbc rca
267891
<outer data>


The inner (outer data) part looks like :

two 64 bit words of random noise
the decrypt/decompress verify data (two 64 bit words)
service [srvargs]\n
<inner data>


The inner data is the data for the specified service.




