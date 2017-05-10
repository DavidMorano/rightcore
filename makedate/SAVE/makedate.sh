 # <-- force CSh to use Bourne shell
# MAKEDATE


M=
if [ -n "${1}" ] ; then
  M="/${*}"
fi


date "+char	makedate[] = \"${M} %y/%m/%d %H:%M:%S\" ;"




