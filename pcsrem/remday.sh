day=$1
case "$day" in
     T | t | S | s)
          echo ""$day" is ambiguous; must specify at least one more letter"
          exit
          ;;
     M* | m*)
          day=mon
          shift
          ;;
     W* | w*)
          day=wed
          shift
          ;;
     F* | f*)
          day=fri
          shift
          ;;
     TU* | Tu* | tu*)
          day=tue
          shift
           ;;
     TH* | Th* | th*)
          day=thu
          shift
          ;;
     SA* | Sa* | sa*)
          day=sat
          shift
          ;;
     SU* | Su* | su*)
          day=sun
          shift
          ;;
     *)
          echo "remday: usage day time 'message'"
          exit
          ;;
esac

rest=$*
set `date`
today=`echo $1| tr 'MTWFS' 'mtwfs'`
if test "$today" = "$day"
then rem $rest
else exit
fi
