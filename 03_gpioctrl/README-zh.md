# 03 gpioctrl
```c
#define IO_LED 21
#define IO_BUTTON 20
#define IO_OFFSET 512
```
根据[pinout](https://pinout.xyz/)和[Linux Device Tutorial](https://github.com/Johannes4Linux/Linux_Driver_Tutorial/blob/main/03_gpioctrl/README.md)来看，使用到的是GPIO 21和GPIO 20。

但是这还需要加入偏移值。输入`gpiodetect`可以查看到有多少个GPIO控制器。
```bash
gpiochip0 [pinctrl-bcm2711] (58 lines)
gpiochip1 [raspberrypi-exp-gpio] (8 lines)
```
然后使用`gpioinfo`可以看到具体的情况
```bash
gpiochip0 - 58 lines:
        line   0:     "ID_SDA"       unused   input  active-high
        line   1:     "ID_SCL"       unused   input  active-high
        line   2:      "GPIO2"       unused   input  active-high
        line   3:      "GPIO3"       unused   input  active-high
        line   4:      "GPIO4"       unused   input  active-high
        line   5:      "GPIO5"       unused   input  active-high
        line   6:      "GPIO6"       unused   input  active-high
        line   7:      "GPIO7"       unused   input  active-high
        line   8:      "GPIO8"       unused   input  active-high
        line   9:      "GPIO9"       unused   input  active-high
        line  10:     "GPIO10"       unused   input  active-high
        line  11:     "GPIO11"       unused   input  active-high
        line  12:     "GPIO12"       unused   input  active-high
        line  13:     "GPIO13"       unused   input  active-high
        line  14:     "GPIO14"       unused   input  active-high
        line  15:     "GPIO15"       unused   input  active-high
        line  16:     "GPIO16"       unused   input  active-high
        line  17:     "GPIO17"       unused   input  active-high
        line  18:     "GPIO18"       unused   input  active-high
        line  19:     "GPIO19"       unused   input  active-high
        line  20:     "GPIO20"       unused   input  active-high
        line  21:     "GPIO21"       unused   input  active-high
        line  22:     "GPIO22"       unused   input  active-high
        line  23:     "GPIO23"       unused   input  active-high
        line  24:     "GPIO24"       unused   input  active-high
        line  25:     "GPIO25"       unused   input  active-high
        line  26:     "GPIO26"       unused   input  active-high
        line  27:     "GPIO27"       unused   input  active-high
        line  28: "RGMII_MDIO"       unused   input  active-high
        line  29:  "RGMIO_MDC"       unused   input  active-high
        line  30:       "CTS0"       unused   input  active-high
        line  31:       "RTS0"       unused   input  active-high
        line  32:       "TXD0"       unused   input  active-high
        line  33:       "RXD0"       unused   input  active-high
        line  34:    "SD1_CLK"       unused   input  active-high
        line  35:    "SD1_CMD"       unused   input  active-high
        line  36:  "SD1_DATA0"       unused   input  active-high
        line  37:  "SD1_DATA1"       unused   input  active-high
        line  38:  "SD1_DATA2"       unused   input  active-high
        line  39:  "SD1_DATA3"       unused   input  active-high
        line  40:  "PWM0_MISO"       unused   input  active-high
        line  41:  "PWM1_MOSI"       unused   input  active-high
        line  42: "STATUS_LED_G_CLK" "ACT" output active-high [used]
        line  43: "SPIFLASH_CE_N" unused input active-high
        line  44:       "SDA0"       unused   input  active-high
        line  45:       "SCL0"       unused   input  active-high
        line  46: "RGMII_RXCLK" unused input active-high
        line  47: "RGMII_RXCTL" unused input active-high
        line  48: "RGMII_RXD0"       unused   input  active-high
        line  49: "RGMII_RXD1"       unused   input  active-high
        line  50: "RGMII_RXD2"       unused   input  active-high
        line  51: "RGMII_RXD3"       unused   input  active-high
        line  52: "RGMII_TXCLK" unused input active-high
        line  53: "RGMII_TXCTL" unused input active-high
        line  54: "RGMII_TXD0"       unused   input  active-high
        line  55: "RGMII_TXD1"       unused   input  active-high
        line  56: "RGMII_TXD2"       unused   input  active-high
        line  57: "RGMII_TXD3"       unused   input  active-high
gpiochip1 - 8 lines:
        line   0:      "BT_ON"   "shutdown"  output  active-high [used]
        line   1:      "WL_ON"       unused  output  active-high
        line   2: "PWR_LED_OFF" "PWR" output active-low [used]
        line   3: "GLOBAL_RESET" unused output active-high
        line   4: "VDD_SD_IO_SEL" "vdd-sd-io" output active-high [used]
        line   5:   "CAM_GPIO" "regulator-cam1" output active-high [used]
        line   6:  "SD_PWR_ON" "regulator-sd-vcc" output active-high [used]
        line   7:    "SD_OC_N"       unused   input  active-high
```
像`GPIO <number>`这样的格式，它表示的并不是GPIO的全局标识，他只是相对于自己的GPIO控制器的基准位置的编号，所以还需要加入GPIO的`base`，也可以说是`offset`。
```bash
$ cat /sys/kernel/debug/gpio
gpiochip0: GPIOs 512-569, parent: platform/fe200000.gpio, pinctrl-bcm2711:
 gpio-512 (ID_SDA              )
 gpio-513 (ID_SCL              )
 gpio-514 (GPIO2               )
 gpio-515 (GPIO3               )
 gpio-516 (GPIO4               )
```
可以看到实际的全局标识是512开始的，所以要使用的GPIO应该是`GPIO (number + offset)`

---

## gpio_to_desc
`gpio_to_desc`内容如下
```c
struct gpio_desc *gpio_to_desc(unsigned gpio)
{
	struct gpio_device *gdev; // 用于遍历GPIO控制器（设备）的指针
	// scoped_guard(srcu, &gpio_devices_srcu) 这是一个RAII风格的宏，用于管理SRCU锁
        // srcu是一个class类型，gpio_devices_srcu则是这个class的一个对象，整个scoped_guard也就是为了自动化管理这个对象
	scoped_guard(srcu, &gpio_devices_srcu) {
		// list_for_each_entry_srcu 遍历一个SRC-protected链表
		// gdev: 当前遍历到的gpio_device指针
		// &gpio_devices: 要遍历的链表头，它是一个全局链表，存储了所有注册的GPIO控制器
		// list: gpio_device结构体中用于链表连接的成员名
		// srcu_read_lock_held(&gpio_devices_srcu): SRCU读取锁是否被持有的检查
		list_for_each_entry_srcu(gdev, &gpio_devices, list,
				srcu_read_lock_held(&gpio_devices_srcu)) {
			// 检查当前的GPIO编号是否落在当前gdev（GPIO控制器）管理的范围之内
			if (gdev->base <= gpio &&      // GPIO编号大于或等于控制器的起始编号
			    gdev->base + gdev->ngpio > gpio) // 并且GPIO编号小于控制器的结束编号 (base + ngpio)
				// 如果是，则计算出对应的gpio_desc并返回
				return &gdev->descs[gpio - gdev->base];
		}
	}
	// 如果遍历完所有GPIO控制器都没有找到对应的gpio_desc，则返回NULL
	return NULL;
}
```
`scpoed_guard`内容如下：
```c
#define CLASS(_name, var)						\
	class_##_name##_t var __cleanup(class_##_name##_destructor) =	\
		class_##_name##_constructor
...
#define __guard_ptr(_name) class_##_name##_lock_ptr
#define __is_cond_ptr(_name) class_##_name##_is_conditional
...
#define __scoped_guard(_name, _label, args...)				\
	for (CLASS(_name, scope)(args);					\
	     __guard_ptr(_name)(&scope) || !__is_cond_ptr(_name);	\
	     ({ goto _label; }))					\
		if (0) {						\
_label:									\
			break;						\
		} else

#define scoped_guard(_name, args...)	\
	__scoped_guard(_name, __UNIQUE_ID(label), args)
```
所以`scoped_guard`后面的括号等于是定义了`else`后面的内容。这里对for循环的使用，并不是为了迭代。而是利用for循环机制里面的定义，判断和迭代三部分进行作用域控制。
- 定义：`CLASS(_name, scope)(args)`：定义了一个对象，对象名为scope，这这里等于说是将`args`传递给了`class_##_name##_constructor`构造函数。然后同时`__cleanup`属性定义了一个`class_##_name##_destructor`析构函数。这个函数会在变量离开代码块的时候自动触发。
- 判断：`__guard_ptr`和`__is_cond_ptr`的作用都是定义了别名，所以这里实际上是在检查两个var是否为null
- 迭代：使用了 GNU C 的语法扩展 `({ ... })` 来执行 goto 语句，用于跳转到指定的标签 _label，然后退出 (break) for 循环

这里的if条件永远不会生效，所以这里实际上就是只会执行else的语句，也就是说`scoped_guard`的作用实际上就是为了管理`srcu`资源，保证运行后得到释放。

关于`CLASS`里面定义的`class_##_name##_constructor`从哪来这个问题，可以关注到：
```c
// /include/linux/srcu.h 
DEFINE_LOCK_GUARD_1(srcu, struct srcu_struct,
		    _T->idx = srcu_read_lock(_T->lock),
		    srcu_read_unlock(_T->lock, _T->idx),
		    int idx)
```

```c
// /include/linux/cleanup.h
#define __DEFINE_UNLOCK_GUARD(_name, _type, _unlock, ...)		\
typedef struct {							\
	_type *lock;							\
	__VA_ARGS__;							\
} class_##_name##_t;							\
									\
static inline void class_##_name##_destructor(class_##_name##_t *_T)	\
{									\
	if (_T->lock) { _unlock; }					\
}									\
									\
static inline void *class_##_name##_lock_ptr(class_##_name##_t *_T)	\
{									\
	return (void *)(__force unsigned long)_T->lock;			\
}


#define __DEFINE_LOCK_GUARD_1(_name, _type, _lock)			\
static inline class_##_name##_t class_##_name##_constructor(_type *l)	\
{									\
	class_##_name##_t _t = { .lock = l }, *_T = &_t;		\
	_lock;								\
	return _t;							\
}

#define __DEFINE_LOCK_GUARD_0(_name, _lock)				\
static inline class_##_name##_t class_##_name##_constructor(void)	\
{									\
	class_##_name##_t _t = { .lock = (void*)1 },			\
			 *_T __maybe_unused = &_t;			\
	_lock;								\
	return _t;							\
}

#define DEFINE_LOCK_GUARD_1(_name, _type, _lock, _unlock, ...)		\
__DEFINE_CLASS_IS_CONDITIONAL(_name, false);				\
__DEFINE_UNLOCK_GUARD(_name, _type, _unlock, __VA_ARGS__)		\
__DEFINE_LOCK_GUARD_1(_name, _type, _lock)

#define DEFINE_LOCK_GUARD_0(_name, _lock, _unlock, ...)			\
__DEFINE_CLASS_IS_CONDITIONAL(_name, false);				\
__DEFINE_UNLOCK_GUARD(_name, void, _unlock, __VA_ARGS__)		\
__DEFINE_LOCK_GUARD_0(_name, _lock)
```



