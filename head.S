@******************************************************************************
@ File: head.S
@ ����: ����SDRAM�����������Ƶ�SDRAM��Ȼ������SDRAM����ִ��
@******************************************************************************       
   
.extern     main
.text 
.global _start 
_start:
@******************************************************************************       
@ �ж��������������У���Reset��HandleIRQ�⣬�����쳣��û��ʹ��
@******************************************************************************       
    b   Reset

@ 0x04: δ����ָ����ֹģʽ��������ַ
HandleUndef:
    b   HandleUndef 
 
@ 0x08: ����ģʽ��������ַ��ͨ��SWIָ��������ģʽ
HandleSWI:
    b   HandleSWI

@ 0x0c: ָ��Ԥȡ��ֹ���µ��쳣��������ַ
HandlePrefetchAbort:
    b   HandlePrefetchAbort

@ 0x10: ���ݷ�����ֹ���µ��쳣��������ַ
HandleDataAbort:
    b   HandleDataAbort

@ 0x14: ����
HandleNotUsed:
    b   HandleNotUsed

@ 0x18: �ж�ģʽ��������ַ
    b   HandleIRQ

@ 0x1c: ���ж�ģʽ��������ַ
HandleFIQ:
    b   HandleFIQ

Reset:                  
    ldr sp, =4096           @ ����ջָ�룬���¶���C����������ǰ��Ҫ����ջ
    bl  disable_watch_dog   @ �ر�WATCHDOG������CPU�᲻������
    bl  clock_init          @ ����MPLL���ı�FCLK��HCLK��PCLK
    bl  memsetup            @ ���ô洢��������ʹ��SDRAM
    bl  nand_init           @ ��ʼ��NAND Flash
    
    
                            @ ���ƴ��뵽SDRAM��
    ldr r0, =0x30000000     @ 1. Ŀ����ַ = 0x30000000������SDRAM����ʼ��ַ
    mov r1, #4096           @ 2. Դ��ַ   = 4096�����е�ַ��SDRAM�еĴ��뱣����NAND Flash 4096��ַ��ʼ��
    mov r2, #14*2048        @ 3. ���Ƴ��� = 16K�����ڱ�ʵ�飬�����㹻��
    bl  CopyCode2SDRAM      @ ����C����CopyCode2SDRAM
    
    bl  clean_bss           @ ����bss�Σ�δ��ʼ������ֵΪ0��ȫ��/��̬����������bss��

    msr cpsr_c, #0xd2       @ �����ж�ģʽ
    ldr sp, =0x32000000     @ �����ж�ģʽջָ��

    msr cpsr_c, #0xdf       @ ����ϵͳģʽ
    ldr sp, =0x34000000     @ ����ϵͳģʽջָ�룬

    ldr lr, =ret_initirq    @ ���÷��ص�ַ    
    ldr pc, =init_irq       @ �����жϳ�ʼ������
ret_initirq:
    msr cpsr_c, #0x5f       @ ����I-bit=0����IRQ�ж�

    ldr lr, =halt_loop      @ ���÷��ص�ַ
    ldr pc, =main           @ ����main����
halt_loop:
    b   halt_loop

HandleIRQ:
    sub lr, lr, #4                  @ ���㷵�ص�ַ
    stmdb   sp!,    { r0-r12,lr }   @ ����ʹ�õ��ļĴ���
                                    @ ע�⣬��ʱ��sp���ж�ģʽ��sp
                                    @ ��ʼֵ���������õ�4096
    
    ldr lr, =int_return             @ ���õ���IRQ_Handle�������ķ��ص�ַ  
    ldr pc, =IRQ_Handle             @ �����жϷַ���������interrupt.c��
int_return:
    ldmia   sp!,    { r0-r12,pc }^  @ �жϷ���, ^��ʾ��spsr��ֵ���Ƶ�cpsr
    
