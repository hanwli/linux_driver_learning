# 01_hello
## VScode配置代码提示
整体代码比较简单。这里主要说的是如何去配置VSCode里面的C/C++配置文件，让他们对头文件识别。

首先可以看到我们使用的Makefile文件
```Makefile
make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
```
`-C`切换目录执行makefile文件，这里你可以cd进去，可以看到里面的makefile引用了其他的makefile，我这里显示的：
```Makefile
include /usr/src/linux-headers-6.12.34+rpt-common-rpi/Makefile
```
> `M=$(PWD)` 这里是填补了`/usr/src/linux-headers-6.12.34+rpt-common-rpi/Makefile`里面M的值。

现在再进入`/usr/src/linux-headers-6.12.34+rpt-common-rpi/Makefile`,可以看到：
```Makefile
# Use USERINCLUDE when you must reference the UAPI directories only. 
USERINCLUDE := \ -I$(srctree)/arch/$(SRCARCH)/include/uapi \ -I$(objtree)/arch/$(SRCARCH)/include/generated/uapi \ -I$(srctree)/include/uapi \ -I$(objtree)/include/generated/uapi \ 
# Use LINUXINCLUDE when you must reference the include/ directory. 
LINUXINCLUDE := \ -I$(srctree)/arch/$(SRCARCH)/include \ -I$(objtree)/arch/$(SRCARCH)/include/generated \ $(if $(building_out_of_srctree),-I$(srctree)/include) \ -I$(objtree)/include \ $(USERINCLUDE)
```
如果你不知道这里面的是什么的话可以在`/usr/src/linux-headers-6.12.34+rpt-common-rpi/Makefile`里面加入：
```Makefile
print-vars:
    @echo "srctree=$(srctree)"
    @echo "objtree=$(realpath $(objtree))"
    @echo "SRCARCH=$(SRCARCH)"
```
同时在自己的Makefile里面加入:
```Makefile
print:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) print-vars
```
在当前目录执行`make print`即可看到结果。

打印出的变量带入路径里面就是你需要在`c_cpp_properties.json`里面使用的变量，注意这里的顺序需要保证是对的，因为会有文件冲突，比如`module.h`文件，所以确保引入顺序按照`USERINCLUDE`在最后面。我的`includePath`如下：
```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "/usr/src/linux-headers-6.12.34+rpt-common-rpi/arch/arm64/include",
                "/usr/src/linux-headers-6.12.34+rpt-rpi-v8/arch/arm64/include/generated",
                "/usr/src/linux-headers-6.12.34+rpt-common-rpi/include",
                "/usr/src/linux-headers-6.12.34+rpt-rpi-v8/include",
                "/usr/src/linux-headers-6.12.34+rpt-common-rpi/arch/arm64/include/uapi",
                "/usr/src/linux-headers-6.12.34+rpt-rpi-v8/arch/arm64/include/generated/uapi",
                "/usr/src/linux-headers-6.12.34+rpt-common-rpi/include/uapi",
                "/usr/src/linux-headers-6.12.34+rpt-rpi-v8/include/generated/uapi",                
                "/usr/include",
                "/usr/local/include",
                "/usr/include/aarch64-linux-gnu",
                "/lib/modules/6.12.34+rpt-rpi-v8/build/include"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c17",
            "cppStandard": "gnu++17",
            "intelliSenseMode": "linux-gcc-arm64"
        }
    ],
    "version": 4
}
```

## Makefile执行流程
根据[kernel.org](https://www.kernel.org/doc/Documentation/kbuild/modules.txt)的说法，这里的`obj-m`算是一个约定俗称的变量，这里我们使用了`obj-m+=hello.o`，表示`hello.o`的内容作为一个外部module.

```Makefile
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
```
则是进行编译的命令：
1. 首先切换到`/lib/modules/$(shell uname -r)/build`目录，执行里面的makefile，如我们之前所说，这里实际去执行的`/usr/src/linux-headers-6.12.34+rpt-common-rpi/Makefile`。
2. 然后我们设置`M=$(PWD)`告诉外部模块在当前的项目目录里面。
3. 然后根据`obj-m += hello.o`，这里会去编译`hello.c`作为模块源码。
> `+=`和`:=`还是`=`都不重要，都可以执行。

## 执行
首先执行`make all` 然后两个窗口分别执行下面的命令

输出kernel ring buffer的内容
```bash
sudo dmesg -W # print or control the kernel ring buffer
```
> `printk`的作用就体现在着，因为在内核没有用户空间所说的什么标准输出，所以这里`printk`等于说是将日志消息打印到内核的缓冲区，也就是kernel ring buffer


使用下面的两个命令分别触发`hello.c`里面的module加载和退出的函数
```bash
sudo insmod hello.ko # install module
sudo rmmod hello.ko # remove module
```
