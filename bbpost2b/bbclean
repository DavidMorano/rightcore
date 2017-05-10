 # <-- force CSH to use Bourne shell
# BBCLEAN
# program removes aged bulletins board by board

BRDS=/usr/add-on/pcs/lib/boards
PCSLIB=/usr/add-on/pcs/lib

[ ! -d $BRDS ] && echo Cannot find $BRDS && exit
cd $BRDS
find * -type f ! -name '*/.*' -ctime +21 -print | \
    egrep -v "^./.desc_|^./.pcsprofile|^./job-*post" 2> /dev/null | \
    xargs rm -f

## cd general
## find .  \( ! -name README ! -name .desc \) -mtime +14 -exec rm -f {} \;
## 
## cd ../att
## find .  \( ! -name README ! -name .desc \) -mtime +21 -exec rm -f {} \;
## 
## 
## 
## cd ../integration
## find . \( ! -name README ! -name .desc \) -mtime +21 -exec rm -f {} \;
## 
## cd ../junk
## find . \( ! -name README ! -name .desc \) -mtime +21 -exec rm -f {} \;
## 
## cd ../systems
## find . \( ! -name README ! -name .desc \) -mtime +21 -exec rm -f {} \;
## 
## 
## cd ../xtools
## find . ! -name ".desc" -mtime +28 -exec mv {} /usr/ncmp/pcs/lib/boards/old_programs/. \;
## 
## cd ../xOPinfo
## find . \( ! -name README ! -name .desc \) -mtime +21 -exec rm -f {} \;
## 
## cd ../xHO-LZnews
## find . \( ! -name README ! -name .desc \) -mtime +21 -exec rm -f {} \;
## 
## cd ../xr2aoi
## find . \( ! -name README ! -name .desc \) -mtime +21 -exec rm -f {} \;
## 
## cd ../xr2.config
## find . \( ! -name README ! -name .desc \) -mtime +21 -exec rm -f {} \;
## 
## cd ../xr2models
## find . ! -name ".desc" -mtime +28 -exec rm -f {} \;
## 
## cd ../events
## find . ! -name ".desc" -mtime +14 -exec rm -f {} \;
## 
## cd ../xdes_review
## find . ! -name ".desc" -mtime +21 -exec rm -f {} \;
## 
## cd ../models
## find . ! -name ".desc" -mtime +21 -exec rm -f {} \;
## cd ../xr2ss_mtgs
## find . ! -name ".desc" -mtime +21 -exec rm -f {} \;
## 
## 
## cd ../xnews75
## find . ! -name ".desc" -mtime +21 -exec rm -f {} \;
## 
## cd ../amdahl
## find . ! -name ".desc" -mtime +21 -exec rm -f {} \;
## 
## 
## cd ../programs
## find . ! -name ".desc" -mtime +28 -exec mv {} /usr/ncmp/pcs/lib/boards/old_programs \;

# Clean up the smaillog 
cp $PCSLIB/sendmail/smaillog $PCSLIB/sendmail/smaillog.bak
cat /dev/null > $PCSLIB/sendmail/smaillog


