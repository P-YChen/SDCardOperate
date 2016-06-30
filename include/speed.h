#ifndef SPEED_H
#define SPEED_H

typedef unsigned long ulong;
typedef unsigned int u32;

#define CONFIG_SYS_CLK_FREQ	12000000

void hang (void);
ulong get_FCLK(void);
ulong get_HCLK(void);
ulong get_PCLK(void);
ulong get_UCLK(void);
#endif /* end of TIMER_H */