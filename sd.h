#ifndef _S3C2440_SD_H
#define _S3C2440_SD_H
#include "types.h"
#define DEBUG_FLAG

#ifdef DEBUG_FLAG
#define debug(fmt, args...) printf("[DEBUG] "fmt"\r\n", ##args);
#else
#define debug(fmt, args...) //udelay (2000);
#endif

#define SD_TYPE_UNKNOWN	(1<<0)
#define SD_TYPE_SD	(1<<1)
#define SD_TYPE_SDHC	(1<<2)
int sd_init (void);
void print_cid (u32 *reg);
int8_t sd_block_read (u32 src, u8 *des, u32 len);
int8_t sd_read (u32 src, u8 *des, u32 size);
int8_t sd_block_write (u8 *src, u32 des, u32 len);
int8_t sd_write (u8 *src, u32 des, u32 size);
//static u_int32 *sd_cmd (u_int8 cmd, u_int32 arg, u_int8 flag);
#endif /* end of _S3C2440_SD_H */