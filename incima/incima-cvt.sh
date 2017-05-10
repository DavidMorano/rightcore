 # <-- force CSH to use Bourne shell
# INCIMA-CVT (convert an image file into PostScript for 'incima')

#
#	This program will convert a given image format into PostScript.
#
#	Invoke as :
#
#		$ incima-cvt format infile > outfile
#
#	This program is generally only called by the 'incima' filter program.
#	The 'incima' program is a filter to handle the '.BG' (Begin Graphic)
#	macro invoked within AT&T DWB type documents.  This program
#	can also be called directly if this is what you want to do.
#
#

if [ $# -lt 2 ] ; then

  echo "${0}: not enough arguments supplied" >&2
  exit 1

fi


FORMAT=$1
FILE=$2


if [ ! -f "${FILE}" ] ; then

  echo "${0}: file \"${FILE}\" does not exist" >&2
  FILE=$3

fi

if [ ! -r "${FILE}" ] ; then

  echo "${0}: file \"${FILE}\" is not readable" >&2
  FILE=$4

fi


case $FORMAT in

pct | pict )
  picttoppm $FILE | pnmtops 2> /dev/null
  ;;

jpg | jpeg | jfi | jfif )
  djpeg -pnm -colors 256 | pnmtops 2> /dev/null
  ;;

gif* )
  giftoppm $FILE | pnmtops 2> /dev/null
  ;;

tif* )
  tiff2ps $FILE
  ;;

pnt | macp | mpt | mpnt )
  macptopbm $FILE | pnmtops 2> /dev/null
  ;;

icon )
  icontopbm $FILE | pnmtops 2> /dev/null
  ;;

xbm )
  xbmtopbm $FILE | pnmtops 2> /dev/null
  ;;

xpm )
  xpmtopbm $FILE | pnmtops 2> /dev/null
  ;;

xwd )
   xwd2ps $FILE
#  xwdtopbm $FILE | pnmtops 2> /dev/null
  ;;

ras )
  rasttopbm $FILE | pnmtops 2> /dev/null
  ;;

fs )
  fstopbm $FILE | pnmtops 2> /dev/null
  ;;

pic )
  pic $FILE | troff -Tpost | dpost -B
  ;;

gc* )
  gc2pic $FILE | pic | troff -Tpost | dpost -B
  ;;

text | simple | printer )
  postprint $FILE
  ;;

gplot | plot )
  plot2ps $FILE
  ;;

tek* )
  posttek $FILE
  ;;

dmd* )
  postdmd $FILE
  ;;

eps | ps | post* )
  cat $FILE
  ;;

* )
  echo "${0}: unknown format \"${FORMAT}\" on file \"${FILE}\"" >&2
  exit 1
  ;;

esac



