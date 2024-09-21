#!/bin/bash
# debub script

workspaceFolder=$(cd "$(dirname "$0")" && pwd)
echo "$workspaceFolder"

/usr/bin/valgrind --leak-check=full --show-leak-kinds=all ${workspaceFolder}/rdbtest
