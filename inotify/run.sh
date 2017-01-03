#!/bin/bash
#监视目录已经scp完成时启动沙箱
#Author:zldao
#Date:2016-12-27
if [ $# -ne 1 ]; then
	echo "please input a argument"
	exit -1
fi

SRC_PATH="/opt/input/"
DES_PATH="/opt/linx_test/"
RENAME=$(echo $RANDOM)
while true
do
	CMP1=$(du $SRC_PATH | tail -1 | cut -f 1)
	sleep 1
	CMP2=$(du $SRC_PATH | tail -1 | cut -f 1)
	if [ $CMP1 -eq $CMP2 ]; then
		break
	else
		continue
	fi
done
mv $SRC_PATH/$1 $SRC_PATH/$RENAME 
mv $SRC_PATH/$RENAME $DES_PATH
if [ $? -eq 0 ]; then
	echo all > $DES_PATH/$RENAME/LINX_MOD_LIST
	/opt/malware_sandbox_deploy/malware_app_env.sh -t 100 -p $DES_PATH/$RENAME -i 172.17.0.22 -o 8567
fi

rm $DES_PATH/$RENAME -rf


