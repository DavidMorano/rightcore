
while read F ; do
  if [[ -f ${F} ]] ; then
    print -- $F
  fi
done < h.list > hh.list


