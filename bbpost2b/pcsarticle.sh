 # <-- force CSH to use Bourne shell
# PCSARTICLE

#
#	This is pretty much the same as the old 'deposit' but
#	should be a little more "secure" !
#
#



: ${NEWSSPOOLDIR:=${PCS}/spool/boards}

F=$1
if [ -z "${F}" ] ; then exit 1 ; fi

DIR=`dirname ${F} `

if [ ! -d "${NEWSSPOOLDIR}/${DIR}" ] ; then exit 1 ; fi

if [ -d /usr/sbin ] ; then

  MACH=`uname -n`

else

  MACH=`hostname`

fi

  umask 002

case $MACH in

mtgzfs* | mtgbcs )
  TF=/tmp/df${$}
  cat > $TF
  rcp $TF mthost2:${NEWSSPOOLDIR}/${F}
  rm -f $TF
  ;;

* )
  cat > ${NEWSSPOOLDIR}/${F}
  ;;

esac



