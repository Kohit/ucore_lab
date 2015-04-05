#lab1 report
---
##练习1
- 操作系统镜像文件 ucore.img 是如何一步一步生成的?(需要比较详细地解释 Makefile 中每一条相关命令和命令参数的含义,以及说明命令导致的结果)
```
    1. 用gcc编译bootasm.S和bootmain.c，得到bootasm.o和bootmain.o
    
    > 156 bootfiles = $(call listf_cc,boot)
    > 157 $(foreach f,$(bootfiles),$(call cc_compile,$(f),$(CC),$(CFLAGS) -Os -nostdinc))
    实际上是
    gcc -Iboot/ -fno-builtin -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Os -nostdinc -c boot/bootasm.S -o obj/boot/bootasm.o
    gcc -Iboot/ -fno-builtin -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Os -nostdinc -c boot/bootmain.c -o obj/boot/bootmain.o

    2. 链接bootasm.o和bootmain.o，得到bootblock.o,并将其复制一份为bootblock.out（二进制）
    
    > 159 bootblock = $(call totarget,bootblock)
    > 161 $(bootblock): $(call toobj,$(bootfiles)) | $(call totarget,sign)
    > 162  @echo + ld $@
	> 163  $(V)$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 $^ -o $(call toobj,bootblock)
	> 164	@$(OBJDUMP) -S $(call objfile,bootblock) > $(call asmfile,bootblock)
	> 165   @$(OBJCOPY) -S -O binary $(call objfile,bootblock) $(call outfile,bootblock)
    ld -m    elf_i386 -nostdlib -N -e start -Ttext 0x7C00 obj/boot/bootasm.o obj/boot/bootmain.o -o obj/bootblock.o

    3. 利用sign（由sign.c生成）对bootblock.out进行检查，合格后在bin目录下生成*bootblock*

    > 173 $(call add_files_host,tools/sign.c,sign,sign)
    > 174 $(call create_target_host,sign,sign)

    4. 编译kern下的各代码，生成对应的.o文件,并将生成的.o文件链接起来(按照tools/kernel.ld脚本的规则)，生成*kernel*(line 141 - 151)
    5. 由*bootblock*和*kernel*生成ucore.img,其中分三步
    > 181 $(UCOREIMG): $(kernel) $(bootblock)
	   - $(V)dd if=/dev/zero of=$@ count=10000 // 生成10000个块的ucore.img，内容为空(0)
	   - $(V)dd if=$(bootblock) of=$@ conv=notrunc // 写入bootblock到ucore.img的第一块
	   - $(V)dd if=$(kernel) of=$@ seek=1 conv=notrunc // 从第二块开始写入kernel
```
- 一个被系统认为是符合规范的硬盘主引导扇区的特征是什么?
```
    一共512byte且第510byte的内容为0x55,第511byte的内容为0xAA
```
---
##练习2
- 从 CPU 加电后执行的第一条指令开始,单步跟踪 BIOS 的执行。
```
    在调用qemu时增加日志文件，记录开机后的汇编代码，具体是增加参数'-d in_asm -D q.log' // 参考result的report
```    
- 在初始化位置0x7c00 设置实地址断点,测试断点正常。
```
    修改gdbinit的内容为
    file obj/bootblock.o
    target remote :1234
    set architecture i8086
    b *0x7c00
    c
    x /2i $pc
    执行 make debug,结果为
        The target architecture is assumed to be i8086
        Breakpoint 1 at 0x7c00: file boot/bootasm.S, line 16.
        Breakpoint 1, start () at boot/bootasm.S:16
        => 0x7c00 <start>:      cli    
        0x7c01 <start+1>:    cld   
    断点正常
```
- 从0x7c00开始跟踪代码运行，将单步跟踪反汇编得到的代码与bootasm.S和bootblock.asm进行比较。
```
    由第一步得到的q.log可以找到0x7c00处开始的代码，其与上述文件汇编基本相同，但好像并不是AT&T格式的汇编
```
- 自己找一个bootloader或内核中的代码位置，设置断点并进行测试。
```
    在kern_init设置断点，测试发现了需要填写代码的地方。
``` 
---
##练习3
- 分析bootloader进入保护模式的过程
```
1. 为ds、es、ss寄存器赋初值0
2. 开启A20                        // 使能32位寻址
    - 将键盘控制器8042的P21端口置1  // 测试发现将P20和P21都置1才行
3. 加载GDT
4. 将CR0寄存器最低位置1             // 进入保护模式
5. 进入保护模式后设置段寄存器的初值后跳转到bootmain函数
```
---
##练习4
- 分析bootloader加载ELF格式的OS的过程
```
readsect函数实现了读取扇区的操作：
    1. 等待磁盘准备好
    2. 设置0x1f2 ~ 0x1f7的参数
    3. 等待磁盘准备好
    4. 将数据读到指定位置
readseg函数在readsect的基础上实现了读取任意长度的内容
有了readseg函数后,bootmain便可读取磁盘了
bootmain的操作如下：
    1. 读取ELF
    2. 检查ELF文件是否正常
    3. 将ELF中存储的描述表中的数据读进内存
    4. 根据ELF中的信息跳转到内核入口    //init函数
```
---
##练习5
- 实现函数调用堆栈跟踪函数，并解释最后一行各个数值的含义
```
ebp : 0x00007bf8     eip : 0x00007d68
args: [0xc031fcfa]	[0xc08ed88e]	[0x64e4d08e]	[0xfa7502a8]
    <unknow>: -- 0x00007d67 --
最底层堆栈，即是第一个函数所使用的堆栈，因此该部分对应bootmain函数
```
---
##练习6
- 完善中断初始化和处理，并回答：中断描述符表中一个表项占多少字节？其中哪几位代表中断处理代码的入口？
```
一个表项占8字节，其中第3-4字节是SELECTOR，1-2 & 7-8字节为OFFSET，二者构成入口。
```
---
##扩展练习Challange 1
- 增加syscall功能，即增加一用户态函数（可执行一特定系统调用：获得时钟计数值），
当内核初始完毕后，可从内核态返回到用户态的函数，而用户态的函数又通过系统调用得到内核态的服务
```
1. 开启从用户态切换到内核态的权限
2. 在trap_dispatch的case T_SWITCH_TOU中修改tf指向的cs、ds、es、ss以及eflags
   在case T_SWITCH_TOK中修改tf指向的cs、ds、es和eflags
3. init中的两个函数中分别调用中断，因为内核态的tf中没有esp和ss的信息，所以在调用前将esp - 8

ps：
用户态切换主要问题在于用户与内核的tf有所不同，内核的tf没有esp和ss。
参考答案采用分别另外生成对应结构的方法，个人在尝试多次不另外借助变量的方法，后来在init的两个调用函数
的内联汇编中加上sub $0x8, %%esp再调用中断后偶然发现能够实现直接切换，但是这里具体的机制不太了解。
在challange 2中也用这种方法切换用户态，可是切换到内核态后会出错&多次切换到用户态qemu会自动关闭&将那段
输出寄存器值的语句去掉后无法切换到用户态，原因不明。
challange 2中的中断向量表中内核切换的向量要设置为允许可屏蔽中断，否则从用户态切换后系统不会继续响应，原因不明。

```
