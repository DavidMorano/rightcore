 # <-- force CSH to use Bourne shell
# STARTDAY


PATH=${PATH}:${HOME}/bin
export PATH

daytime -s -t 300 "${@}" &



