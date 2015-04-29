void inline ex1(void){
<<<<<<< HEAD
        asm ("movl $0xffff, %%eax\n");
=======
        asm ("movl $0xffff, %eax\n");
>>>>>>> ce80172f55fa900a8687ebe7ca9c20f377514b31
}

void inline ex2(void){
        unsigned cr0;
        asm volatile ("movl %%cr0, %0\n" :"=r"(cr0));
        cr0 |= 0x80000000;
        asm volatile ("movl %0, %%cr0\n" ::"r"(cr0));
}

void inline ex3(void){
long __res, arg1 = 2, arg2 = 22, arg3 = 222, arg4 = 233;
__asm__ __volatile__("int $0x80"
   : "=a" (__res)
   : "0" (11),"b" (arg1),"c" (arg2),"d" (arg3),"S" (arg4));
 }
