#!/bin/bash
if [ $# -ne 1 ]; then
	echo "please input a argument"
	exit -1
fi

SRC_PATH="/opt/watch/$1"
DES_PATH="/opt/linx_test/"
mv $SRC_PATH $DES_PATH
if [ $? -eq 0 ]; then
	echo all > $DES_PATH/LINX_MOD_LIST
	/opt/malware_sandbox_deploy/malware_app_env.sh -t 100 -p $DES_PATH/$1 -i 172.17.0.19 -o 8567
fi

rm $DES_PATH/$1 -rf


