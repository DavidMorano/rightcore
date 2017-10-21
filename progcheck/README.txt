PROGCHECK

We perform some source-code program checking.  We check for unbalanced:
	parentheses, quotes, literals, comments.

Synopsis:
$ progcheck <file(s)> [-o <opt(s)>] [-V]

Arguments:
<file(s)>	C-language file(s) to check for correctness
-o <opt(s)>	options: counts[={0|1}] (default is OFF)
-V		print program version to standard-error and then exit

