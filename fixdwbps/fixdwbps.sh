 # force 'csh' to use Bourne shell
# FAKE_ADOBE
#
# This little program will take a AT&T DWB generated PostScript
# file, which does not claim to adhere to the Adobe PostScript
# document structuing standards, and change it into a file which
# claims to adhere to the standard even though it still may not.
# This allows some programs which check for Adobe adherence to be
# faked out and think that the file is Adobe adherent.  Many
# PostScript reading programs will not allow arbitrary page operations
# unless the document adheres to the Adobe structuring standards.
#
#

if [ $# -ge 1 ] ; then

  for F in "$@" ; do

    if [ -r $F ] ; then

      sed -e '1,10s/^%!PS$/%!PS-Adobe-/' $F

    fi

  done

else

  sed -e '1,10s/^%!PS$/%!PS-Adobe-/'

fi



