# Linux 0.11

这是一个基于 <https://github.com/Wangzhike/HIT-Linux-0.11> 改装的 Linux 0.11 版
本，被本人用于北航 2017 级软件学院学生的操作系统课程。

这个版本目前在 WSL Ubuntu 18.04 下可以成功编译，并在 Windows 10 下使用 QEMU 运
行。

## 编译与运行

### 前期准备

- 按照 <https://github.com/Wangzhike/HIT-Linux-0.11> 中的方法安装相关依赖。

  编译 Linux 使用的是 `gcc-3.4`，编译 `build.c` 使用的是新版本的 `gcc` (此处成功
  编译时是 7.4.0).

- 将本项目克隆至自己认为合适的地方，最好在 wsl 环境下执行（以保证行尾是 `LF`）。
- 修改 `Makefile` 中的 `Extra` 部分至适合自己的状态。

  本项目中的默认步骤是将生成的磁盘镜像 `Image` 拷贝至 Windows 的 `D:\Study\os`
  目录下，以方便在 Windows 下运行和调试。请自行调整。如果不需要额外步骤请将其留
  空。

### 编译

直接运行 `make` 即可。清理目录使用 `make clean`。

### 运行

使用 QEMU 运行 `qemu-system-i386 -boot a -fda Image -hda .\hdc-0.11.img -m 16M`
即可。

## 与原先项目的区别

- 修改了 `build.c`。原先使用的是 8 字节的 `long`，改成了 `int32_t`。
- 修改了 `Makefile` 增加了可选的额外步骤。

## 课程内容

不同的课程目标已放到不同分支。
