# 01_hello
This subproject is simple, so in this blog (I just name it readme) I will pay attention to vscode c/c++ configuration.

Firstly, part of the project's makefile as follows:
```Makefile
make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
```
`-C` means switching to this dir and excute make command. You can cd into this directory to see that its makefile includes other makefiles. In my case, it shows:
```Makefile
include /usr/src/linux-headers-6.12.34+rpt-common-rpi/Makefile
```
> M=$(PWD) here fills in the value of M in /usr/src/linux-headers-6.12.34+rpt-common-rpi/Makefile.

Now, if we go into `/usr/src/linux-headers-6.12.34+rpt-common-rpi/Makefile`, we can see:
```Makefile
# Use USERINCLUDE when you must reference the UAPI directories only. 
USERINCLUDE := \ -I$(srctree)/arch/$(SRCARCH)/include/uapi \ -I$(objtree)/arch/$(SRCARCH)/include/generated/uapi \ -I$(srctree)/include/uapi \ -I$(objtree)/include/generated/uapi \ 
# Use LINUXINCLUDE when you must reference the include/ directory. 
LINUXINCLUDE := \ -I$(srctree)/arch/$(SRCARCH)/include \ -I$(objtree)/arch/$(SRCARCH)/include/generated \ $(if $(building_out_of_srctree),-I$(srctree)/include) \ -I$(objtree)/include \ $(USERINCLUDE)
```
If you're not sure about what these variables contain, you can add to `/usr/src/linux-headers-6.12.34+rpt-common-rpi/Makefile`:
```Makefile
print-vars:
    @echo "srctree=$(srctree)"
    @echo "objtree=$(realpath $(objtree))"
    @echo "SRCARCH=$(SRCARCH)"
```
At the same time, add to your own Makefile:
```Makefile
print:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) print-vars
```

Execute make print in the current directory to see the results.

Take the printed variables and incorporate them into your paths for use in `c_cpp_properties.json`. Note that the order needs to be correct to avoid file conflicts, such as the `module.h` file. Therefore, ensure the inclusion order follows with USERINCLUDE at the end. My includePath is as follows:
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

## Makefile Execution Process
According to [kernel.org](https://www.kernel.org/doc/Documentation/kbuild/modules.txt), the `obj-m` variable here is a conventional variable. We use `obj-m+=hello.o`, which indicates that `hello.o` should be built as an external module.

```Makefile
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
```
This loop in the compilation command:
1. First, switch to the `/lib/modules/$(shell uname -r)/build` directory and execute its makefile.  As previously mentioned, this actually executes `/usr/src/linux-headers-6.12.34+rpt-common-rpi/Makefile`. 
2. Then we set `M=$(PWD)` to tell the build system that the external module is in our current project directory.
3. Based on `obj-m += hello.o`, the build system will compile `hello.c` as the module source code.
> The difference between +=, :=, and = doesn't matter much in this case as all would work.

## Excution
First, execute `make all`, then in two separate windows, execute the following commands:

Output the kernel ring buffer content:
```bash
sudo dmesg -W # print or control the kernel ring buffer
```
> The function of `printk` is reflected here. Since the kernel doesn't have what's called standard output in user space, `printk` essentially prints log messages to the kernel's buffer, which is the kernel ring buffer.

Use the following two commands to trigger the module loading and exit functions in `hello.c`:
```bash
sudo insmod hello.ko # install module
sudo rmmod hello.ko # remove module
```