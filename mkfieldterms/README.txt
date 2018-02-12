MKFIELDTERMS

This program is used to make a terminating character array for inclusion (as
source) into C-language code.

Synopsis:
$ mkfieldterms <character(s)> [-lc] [-uc] [-V]

Example:
$ mkfieldterms $'\n /+' [-lc] [-uc] [-V]

Arguments:
<character(s)>	the characters to serve as field terminators
-lc		add all lowercase characters to the output
-uc		add all uppercase characters to the output
-V		print program version and then exit

