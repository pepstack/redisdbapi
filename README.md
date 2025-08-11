## redisdbapi

基于 hiredis 的操作 redis 集群的 C API。

当前仅支持：Linux

创建 redis 集群参考代码库:

    https://github.com/pepstack/redis_cluster_tools


deps.tar.gz 包含 hiredis 编译后的二进制文件，首先解压到工程目录：

    .vscode/
    deps/
       |
       +--- include/
       +--- lib/
    README.md


然后可以编译:

    make

将工程代码放于 Linux 服务器上，然后可以用 VSCode 远程调试。