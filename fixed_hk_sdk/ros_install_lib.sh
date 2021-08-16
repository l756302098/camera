#!/bin/bash

#only root can install
if id | grep root > /dev/null; then
    echo " "
else
        echo "You must be root to install these SDK package."
    exit 1
fi

CDIR=`pwd`
INSTALLDIR=$CDIR/ros_needed_lib
dir_root=`echo $CDIR | cut -d "/" -f2`
echo $dir_root
dir_user=`echo $CDIR | cut -d "/" -f3`
echo $dir_user
if [ -d $INSTALLDIR ]; then
    rm -rf $INSTALLDIR
fi
mkdir $INSTALLDIR
export INSTALLDIR

# Find tar.gz line
ARCHIVE=`awk '/^__ARCHIVE_FOLLOWS__/ { print NR + 1; exit 0; }' $0`

# uzip file
tail -n +$ARCHIVE $0 > $INSTALLDIR/setup_20210205_lib.tar.gz
cd $INSTALLDIR
tar -zxvf setup_20210205_lib.tar.gz

dir_all="/usr/local/lib/thirdparty_lib"
dir_hk="/usr/local/lib/thirdparty_lib/HCNetSDKCom"
dir_ffmpeg="/usr/local/lib/thirdparty_lib/ffmpeg_lib"
dir_tb="/usr/local/lib/thirdparty_lib/tianboir"
cp thirdparty_lib /usr/local/lib -rf
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$dir_all:$dir_hk:$dir_ffmpeg:$dir_tb" >> /home/$dir_user/.bashrc
echo " " >> /home/$dir_user/.bashrc
echo " " >> /home/$dir_user/.bashrc


rm $INSTALLDIR/setup_20210205_lib.tar.gz
rm -rf $INSTALLDIR/thirdparty_lib
rm -rf $INSTALLDIR

echo " "
echo "********** Installation successful ! **********"
echo " "

exit 0
__ARCHIVE_FOLLOWS__
