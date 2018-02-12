CEX

This program arranges to execute a specified program on another node in the
current machine cluster. A machine cluster consists of a subset of those
machines that share a common view of the entire filesystem space.

Synopsis:
$ cex <node> [-t <timeout>] [-n] [-V] <program> [<arg(s)>] 

Arguments:
<node>		name of another machine in current cluster
-t <timeout>	connection timeout
-n		do not read anything from the current standard input
-V		print program version to standard-error and then exit
<program>	file path of program to execute
<arg(s)>	optional arguments for the program to be executed

