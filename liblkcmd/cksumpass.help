CKSUMPASS

This program creates a checksum of a data stream passed through it from input to
output. The checksum is written to standard error. The checksum is the POSIX
'cksum' type of CRC. Use this program in cases where you want to cksum a data
stream in progress such as :

Synopsis:
$ cksumpass -s summary.out < <infile> > <outfile>

Arguments:
<infile>	input file
<outfile>	output file

Example:
$ dd -if=/dev/tape | cksumpass -s summary.out | cpio -idmc

