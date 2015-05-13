#lab4 report
##练习1：分配并初始化一个进程控制块
```
  调用kmalloc给新进程控制块分配一块空间
  初始化进程控制块，
  state = PROC_UNINIT
  id = -1; runs = kstack = need_resched = flags = 0;
  parent = mm = tf = null;
  cr3 = boot_cr3
  调用memset清空context和name
  返回该控制块的地址
```

- 请说明proc_struct中struct context context和struct trapframe *tf成员变量含义和在本实验中的作用？
```
context 保存的是进程上次执行结束时各寄存器的值(eax除外，eax用作返回值)(即上下文)，在进程再次执行时会取出context的内容，即恢复上次执行现场
tf对应的trapframe的成员变量保存的是执行中断时的寄存器值，通过修改tf中的某些值可以实现用户态与内核之间的切换
```


##练习2：为新创建的内核线程分配资源
1. 调用alloc_proc()分配新进程proc
2. 设置新进程的parent = current;
3. 为调用setup_kstack函数分配一块空间作为proc的栈
4. 根据clone_flags决定是否拷贝proc的内存管理块mm
5. 调用copy_thread拷贝父进程的tf的部分内容给proc
6. 为Proc分配pid，并将proc添加到进程队列
7. 调用wakeup_proc使能proc，即将proc的state设为RUNNABLE

- 请说明ucore是否做到给每个新fork的线程一个唯一的id？请说明你的分析和理由.
```
是，分配时会查找进程队列的每个进程的id。
get_pid还做了适当的优化，使用两个静态变量来记录last_pid和next_safe，
当last_pid + 1 < 4096 * 2 && < next_safe时不必再查找整个队列。
get_pid写的有点混乱，个人觉得用散列的话会更好。
```

##练习3：阅读代码，理解 proc_run 函数和它调用的函数如何完成进程切换的。
1. 让current = next proc
2. 设置任务状态段ts中特权态0下的栈顶指针esp0为next的内核栈的栈顶，即next->kstack + KSTACKSIZE;
3. 设置cr3寄存器的值为next->cr3，即切换页目录
4. 调用switch_to，保存前个进程的现场并恢复下个进程的执行现场

- 在本实验的执行过程中，创建且运行了几个内核线程？
```
两个，idleproc 和 initproc
```
- 语句local_intr_save(intr_flag);....local_intr_restore(intr_flag);在这里有何作用?请说明理由
```
local_intr_save(intr_flag);  //关闭中断，并把中断前的状态保存到intr_flag
local_intr_restore(intr_flag); //恢复中断以及之前保存的状态
由于中间部分的操作需要连在一起执行，插入外部操作的话可能使得原有的操作不能正确执行
因此此时需要关闭中断
```
