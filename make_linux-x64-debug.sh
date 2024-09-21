#!/bin/bash
# debub script

workspaceFolder=$(cd "$(dirname "$0")" && pwd)
echo "$workspaceFolder"

cd ${workspaceFolder} && make clean && make BUILD=DEBUG && make BUILD=DEBUG test