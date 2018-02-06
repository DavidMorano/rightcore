MAKENEWER

This program updates files in a target directory to be the same as the specified
files. It only uses the modification dates and the sizes of the files in order
to determine if an update is needed.

Synopsis:
$ makenewer <file(s)> [-af <afile>] {{-d <dstdir>}|<dstdir>} [-im]
	[-o <opt(s)>] [-z] [-r] [-t <touchfile>] [-m <o>=<n>] [-s <o>=<n>] [-V]

Arguments:
<file(s)>	files to process
-af <afile>	file listing names to process
-d <dstdir>	destination directory
-im		ignore missing files (good for Makefiles)
-o <opt(s)>	options: rmsuf, rmfile, younger, im
-z		ignore case when no files are specified
-m <o>=<n>	suffix-mapping specified names to source files
-s <o>=<n>	suffix-substitution source files to destination files
-r		remove target first (good for executable installations)
-t <touchfile>	file to touch at successful completion
-V		print command version to standard-error and then exit

