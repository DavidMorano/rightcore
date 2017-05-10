 # <-- force CSH to use Bourne shell
# MSGSD


if [ -d /usr/sbin ] ; then
  MACH=`uname -n`
else
  MACH=`hostname`
fi

case $MACH in

mt* )
  : ${LOCAL:=/mt/mtgzfs8/hw/add-on/pcs}
  : ${MSGS_SPOOLDIR:=${PCS}/spool/boards/msgs/mt}
  export MSGS_SPOOLDIR
  ;;

hocp[a-d] | logicgate | nitrogen | nucleus )
  : ${PCS:=/home/gwbb/add-on/pcs}
  : ${TOOLS:=/home/gwbb/add-on/exptools}
  : ${MSGS_SPOOLDIR:=${PCS}/spool/boards/msgs/hocp}
  export MSGS_SPOOLDIR
  : ${NAME:="MSGSD - messages daemon"}
  export NAME
  ;;

esac


: ${LOCAL:=/usr/add-on/pcs}
: ${PCS:=/usr/add-on/pcs}
: ${TOOLS:=/usr/add-on/exptools}
export PCS LOCAL TOOLS


VERSION="0a"


PATH=${PCS}/bin:${PATH}
export PATH


U=$LOGNAME
if [ -n "${1}" ] ; then

  U=$1

fi

N=""
if [ -n "${2}" ] ; then
  N=`basename ${2} `
fi

pcsconf -l msgsd -p msgsd-${VERSION} -y "u=${U} j=${N}"

NAME="PCS MSGSD"
export NAME

msgs -s



