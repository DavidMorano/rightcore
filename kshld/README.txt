KSHLD

This program hacks up the LD_LIBRARY_PATH so that KSH doesn't f**k up!

This is used as a way to set the LD_LIBRARY_PATH variable in order to properly
execute a KSH program that has a dependency on libraries such as:
	libcmd.so
	libast.so

