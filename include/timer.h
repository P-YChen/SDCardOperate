#ifndef TIMER_H
#define TIMER_H

typedef unsigned long ulong;
#define CONFIG_SYS_HZ	1000

int timer_init(void);
void reset_timer(void);
ulong get_timer(ulong base);
void set_timer(ulong t);
void udelay(unsigned long usec);
void reset_timer_masked(void);
ulong get_timer_masked(void);
void udelay_masked(unsigned long usec);
unsigned long long get_ticks(void);
ulong get_tbclk(void);
void reset_cpu(ulong ignored);
#endif /* end of TIMER_H */