PCSCONF

This program is used by administrators to verify configurations within the PCS
system. This program principally processes the generic configuration information
of the main PCS system-wide configuration database. Usually, parameters are
modified in the main configuration file and then the proper result is verified
by running this program and requesting the parameter in question. Unlike many
other similar information-query commands, this command requires that a query
name match exactly with a configuration parameter.

Synopsis:
$ pcsconf <query(s)> [-af <afile>] [-cf <cfile>] [-u <user>] [-s] [-l]
	[-df <dfile>] [-lf <lfile>] [-V]

Arguments:
<query(s)>	variable queries
-af <afile>	argument-list file
-cf <cfile>	configuration file
-df <dfile>	dump file (dumps all parameters to this file)
-lf <lfile>	log file
-u <user>	retrieve information as if alternative user
-l		list-mode (list all variables to STDOUT)
-s		force only showing special built-in parameters
-V		print program version to standard error and then exit

