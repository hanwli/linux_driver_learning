# 02_better_hello
这里仍然是一个比较简单的项目，我这里将主要的叙述内容放在其中一些gcc扩展语法上。

## __init 和 __exit
这两个attribute都非必须的，但是可以提高可读性。他们分别表示
1. `__init`表示这个函数属于用来初始化的函数
2. `__exit`表示这个函数属于模块被卸载的时候执行的函数

## module_init
这个就比较绕了。他是存在动态和静态的两种定义，如下面的代码所示：
```c
#ifndef MODULE
...
#define module_init(x)	__initcall(x);
...
#else /* MODULE */
/* Each module must use one module_init(). */
#define module_init(initfn)					\
	static inline initcall_t __maybe_unused __inittest(void)		\
	{ return initfn; }					\
	int init_module(void) __copy(initfn)			\
		__attribute__((alias(#initfn)));		\
	___ADDRESSABLE(init_module, __initdata);
...
#endif
```

