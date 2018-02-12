LIBPRELOAD

This library is (meant to be) preloaded with all normal programs. It intercepts
(through interposition) the following LIBC subroutine calls:

confstr
sysinfo
gethostid
getpagesize


There are files that support these operations. These files are located
in '/etc/default' and are:

+ confstr
+ sysinfo

These files can be missing, in which case suitable defaults are employed.

Some sample versions of '/etc/default/' configuration files enclosed
in this directory as:

+ confstr.def
+ sysinfo.def

