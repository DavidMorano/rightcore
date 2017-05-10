 # <-- force CSH to use Bourne shell
# LS


: ${GNU:=/usr/add-on/gnu}
: ${LOCAL:=/usr/add-on/local}

LS=${GNU}/bin/ls
if [ ! -x $LS ] ; then
  LS=${LOCAL}/bin/ls
  if [ ! -x $LS ] ; then
    LS=/usr/5bin/ls
    if [ ! -x $LS ] ; then
      LS=/usr/bin/ls
    fi
  fi
fi

exec execname $LS ${0} -1 "${@}"



