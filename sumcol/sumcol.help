SUMCOL

This little program reads the second (by default) of two or more columns (one
column if the specified column is column one) in each of the files specified.
The selected entry of each column is summed together for each line of the files.
All files should have the same number of lines for this to work properly. A new
output file is produced with the first column of the first file repeated, and
the second column consisting of the sum of all of the second columns from all
files. An option can have the sum divided by the number of files.

Integer and floating point numbers are automatically recognized and the output
file reflects the type of numbers from the input file.

Synopsis:
$ sumcol [<file(s)> ...] [-af <argfile>] [-d] [-c <factor>] [-V]

Arguments:
<file(s)>	files to sum columns from
-af <argfile>	a file with a list of files to sum
-<n>		specified column to sum up (default 2nd)
-c <factor>	compression factor
-d		divide the sum by the number of files (take an average)
-i		ignore non-numbers in other colums
-V		print program version to standard-error and then exit

