#!/usr/bin/nawk -f
# PSBOXSIZE
#
#
# This is the same as the BOXSIZE program discussed in the
# AT&T DWB PostScript® Utilities manual.
#
# David Morano, 1998-10-21
#
#

function dimensions(file) {
  height = 3.0 * 72.0
  width = 6.0 * 72.0
  while (getline < file > 0) {
    if ($1 == "%%BoundingBox:") {
      height = $5-$3
      width = $4-$2
    }
  }
  close(file)
}

($1 == ".BP") && (($3 == -1) || ($4 == -1)) {

  argc = split($0,arg)

  dimensions(arg[2])

  if ((arg[3] < 0) && (arg[4] < 0)) {
    arg[3] = height/72.0
    arg[4] = width/72.0
  } else if (arg[3] < 0) {
    arg[3] = arg[4] * height / width
  } else {
    arg[4] = arg[3] * width / height
  }

  for (i = 1 ; i <= argc ; i += 1) {
    if (i > 1) printf " "
    printf "%s", arg[i]
  }

  printf "\n"
  next

}

{
  print $0
}



