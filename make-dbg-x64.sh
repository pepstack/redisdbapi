#!/usr/bin/bash
_file=$(readlink -f $0)
_cdir=$(dirname $_file)
_name=$(basename $_file)

set -x  # 显示执行的命令
#set -e  # 出错时立即退出

cd ${_cdir} && make clean && make BUILD=DEBUG all && make BUILD=DEBUG test
