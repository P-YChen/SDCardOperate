#include <stdio.h>
#include "sd.h"
#include "s3c24xx.h"
#include <string.h>
#include "timer.h"
#include "mmc.h"

static u8 SD_Type = SD_TYPE_UNKNOWN;
static u16 SD_RCAddr;
static u8 sd_buf[SD_BLOCK_SIZE];
static u32 *sd_cmd (u8 cmd, u32 arg, u8 flag);

static u32 *sd_cmd (u8 cmd, u32 arg, u8 flag)
{
	u32 SdiCmd = 0x3f & cmd;
	SdiCmd |= SDICMDCON_COMMAND_DEFAULT;
	SDICmdArg = arg;
	u32 SdiCmdState = (1<<11);
	u32 csta;
	static u32 Rsp[5];
	
	memset (Rsp, 0, sizeof(Rsp));
	
	debug ("cmd = 0x%08x", cmd);
	debug ("arg = 0x%08x", arg);
	debug ("flag = 0x%02x", flag);
	
	// 清除状态
	SDICmdSta = 0xffffffff;
	SDIDatSta = 0xffffffff;
	SDIFSTA = 0xffffffff;
	
	if (flag & SDI_FLAG_RSP)
	{
		SdiCmd |= SDICMDCON_COMMAND_WAITRSP;
		SdiCmdState = (1<<9);
	}
	
	if (flag & SDI_FLAG_RSP_LONG)
	{
		SdiCmd |= SDICMDCON_COMMAND_LONGRSP;
	}

	SDICmdCon = SdiCmd;
	
	while(1)
	{
		//udelay(2000);
		csta = SDICmdSta;
		debug ("csta=0x%08x", csta);
		udelay (2000);
		if (csta & SdiCmdState)
			break;
		
		if (csta & (1<<10)) //timeout
		{
			SDICmdSta = (1<<10);
			debug ("### SD Time OUT ###");
			return Rsp;
		}
	}
	
	SDICmdSta = csta;
	
	Rsp[0] = SDIRSP0;
	Rsp[1] = SDIRSP1;
	Rsp[2] = SDIRSP2;
	Rsp[3] = SDIRSP3;
	
	return Rsp;	
}

void print_cid (u32 *reg)
{
	struct sd_cid *cid = (struct sd_cid *)reg;
	
	printf ("### -SD Card- ###\r\n");
	printf ("Manufacturer ID:\t0x%02x\r\n", cid->mid);
	printf ("OEM/Application ID:\t%c%c\r\n", cid->oid_1, cid->oid_0);
	printf ("Product name:\t\t\"%c%c%c%c%c\"\r\n", cid->pnm_4, cid->pnm_3, \
												 cid->pnm_2, cid->pnm_1, cid->pnm_0);
	printf ("Product revision:\t%d.%d\r\n", ((cid->prv&0xf0)>>4), (cid->prv&0x0f));
	printf ("Product serial number:\t%u\r\n", (cid->psn_3<<24)|(cid->psn_2<<16)|(cid->psn_1<<8)|cid->psn_0);
	printf ("Manufacturing date:\t%4u.%02u\r\n", 2000+(((cid->mdt_1&0x0f)<<4) | (cid->mdt_0&0xf0)>>4), (cid->mdt_0&0x0f));
	printf ("CRC7 Check:\t\t0x%02x\r\n", cid->crc);	
}

int sd_init (void)
{
	u32 *res;
	u8 retry = 10;
	u8 is_finish = 0;
	int ret = -1;
	
	// 引脚设置
	GPECON |= (1<<11) | (1<<13) | (1<<15) | (1<<17) | (1<<19) | (1<<21);
	GPEUP = 0x07E0;
	// 使能时钟
	CLKCON |= (1<<9);
	
	SDIBSize = 512;
	SDIPRE = 0x01;
	
	SDIDTimer = 0x7fffff;
	//屏蔽中断
	SDIIntMsk = 0x0;
	
	//SDICON = (1<<5) | (1<<8) | (1<<0); //CLOCK TYPE=mmc
	SDICON = (1<<5) | (1<<0);
	udelay (1000); //wait 74 clocks
	
	//Send CMD0
	res = sd_cmd(MMC_CMD_GO_IDLE_STATE, 0, 0);
	// Send CMD8
	res = sd_cmd (SD_CMD_SEND_IF_COND, (1<<8) | 0xAA, SDI_FLAG_RSP);
	
	if (!res[0]){
		SD_Type = SD_TYPE_SD;
	}
	else {
		SD_Type = SD_TYPE_SDHC;
		// Valid Response
		if ((res[0] & 0xAA) == 0) {
			SD_Type = SD_TYPE_UNKNOWN;
			return ret;
		}
		// Card with compatible Voltage range
		if ((res[0] & (1<<8)) == 0) {
			printf ("SD card's voltage is uncompatible\r\n");
			return ret;
		}
	}
	debug ("CMD8 res[0]=0x%08x", res[0]);
	debug ("CMD8 res[1]=0x%08x", res[1]);
	udelay (5000);
	
	// try 10 times to get card status (busy)
	while (retry--)	{
		printf ("- %d times to try\r\n", 10-retry);
		udelay (5000);
		// Send CMD55
		res = sd_cmd (MMC_CMD_APP_CMD, 0, SDI_FLAG_RSP);
		udelay (5000);
		//debug ("CMD55 res[0]=0x%08x", res[0]);
		// Send ACMD41
		res = sd_cmd (SD_CMD_APP_SEND_OP_COND, \
					(SD_Type == SD_TYPE_SD) ? MMC_VDD_32_33 | MMC_VDD_33_34 :\
											MMC_VDD_32_33 | MMC_VDD_33_34 | OCR_HCS ,\
					SDI_FLAG_RSP);
		udelay (5000);
		//debug ("ACMD41 res[0] = 0x%08x", res[0]);
		//debug ("ACMD41 res[1] = 0x%08x", res[1]);
		
		if ((res[0] & (1<<31)) != 0){
			is_finish = 1;
			break;
		}
	}
	
	if ((retry == 0) && (!is_finish)){
		printf ("SD Card Init Time Out.\r\n");
		return ret;
	}
	
	if ((res[0] & (1<<30)) != 0)
		printf ("SD Type = SDHC\r\n");
	else
		printf ("SD Type = SD\r\n");
		
	// Send CMD2
	res = sd_cmd (MMC_CMD_ALL_SEND_CID, 0, SDI_FLAG_RSP|SDI_FLAG_RSP_LONG);
	debug ("CMD2 res[0] = 0x%08x", res[0]);
	debug ("CMD2 res[1] = 0x%08x", res[1]);
	debug ("CMD2 res[2] = 0x%08x", res[2]);
	debug ("CMD2 res[3] = 0x%08x", res[3]);
	udelay (5000);
	if (res[0] != 0){
		print_cid (res);
	} else {
		return ret;
	}
		
	// Send CMD3
	res = sd_cmd (SD_CMD_SEND_RELATIVE_ADDR, 0, SDI_FLAG_RSP);
	if (res[0] != 0){
		SD_RCAddr = (res[0] & 0xffff0000) >> 16;
		printf ("SD RCAddress:\t\t0x%04x\r\n", SD_RCAddr);
	}
	debug ("CMD3 res[0] = 0x%08x", res[0]);
	
	// Send CMD9 to get card capacity
	res = sd_cmd (MMC_CMD_SEND_CSD, SD_RCAddr<<16, SDI_FLAG_RSP|SDI_FLAG_RSP_LONG);	
	
	debug ("CMD9 res[0] = 0x%08x", res[0]);
	debug ("CMD9 res[1] = 0x%08x", res[1]);
	debug ("CMD9 res[2] = 0x%08x", res[2]);
	debug ("CMD9 res[3] = 0x%08x", res[3]);
	udelay (5000);
	
	if (res[0] != 0){
		u8 READ_BL_LEN;
		u16 C_SIZE;
		u8 C_SIZE_MULT;
		u32 card_capacity;
		switch ((res[0] & 0xc0000000) >> 30)
		{
			case SD_CSD_REG_V1:
				READ_BL_LEN = (res[1] & 0x000f0000) >> 16;
				C_SIZE = ( ((res[1] & 0x000003ff)<<2) | ((res[2] & 0xc0000000)>>30) );
				C_SIZE_MULT = (res[2] & (0x7 << 16)) >> 16;
				card_capacity = (C_SIZE+1) * (1<<(C_SIZE_MULT+2)) * (1<<READ_BL_LEN) * 1.0;
				printf ("SD Capacity:\t\t%dK\r\n", card_capacity);
				break;
			case SD_CSD_REG_V2:
				C_SIZE = (res[2]&0xffff0000)>>16 | (res[1]&0x3f)<<17;
				debug ("C_SIZE = %08x", C_SIZE);
				//card_capacity = (C_SIZE+1) * 512 / 1024 / 1024;
				card_capacity = (C_SIZE+1) * 512;
				printf ("SD Capacity:\t\t%u K\r\n", card_capacity);
			default:
				break;
		}
		
	}
	
	res = sd_cmd (MMC_CMD_SELECT_CARD, SD_RCAddr<<16, SDI_FLAG_RSP);
	debug ("CMD7 res[0] = 0x%08x", res[0]);
	udelay (5000);
	
	res = sd_cmd (MMC_CMD_APP_CMD, SD_RCAddr<<16, SDI_FLAG_RSP);
	res = sd_cmd (SD_CMD_APP_SET_BUS_WIDTH, 0x02, SDI_FLAG_RSP);
	ret = 1;
	return ret;
}

#define FIFO_WORDS ((SDIFSTA & 0x7f) >> 2)

int8_t sd_block_read (u32 src, u8 *des, u32 len)
{
	u32 SD_Data_Con;
	u32 *des_u32 = (u32 *)des;
	u32 *res;
	//SDIDTimer = 0x7fffff;	
	
	debug ("src = 0x%08x", src);
	debug ("des = 0x%08p", des);
	// Send CMD16
	res = sd_cmd (MMC_CMD_SET_BLOCKLEN, len, SDI_FLAG_RSP);
	debug ("CMD16 res[0] = 0x%08x", res[0]);
	
	switch (SD_Type)
	{
		case SD_TYPE_SD:
			SDIBSize = SD_BLOCK_SIZE;
			break;
		case SD_TYPE_SDHC:
			SDIBSize = SD_BLOCK_SIZE;
			break;
		default:
			printf ("### Unknown SD Card Type, Please reset the board.\r\n");
			for(;;);
	}
	
	SD_Data_Con = SD_DATA_SIZE_WORD | SD_BLOCK_MODE_BLOCK | SD_WIDE_BUS_WIDE | \
	              SD_DATA_MODE_RECEIVE | SD_RACMD | SD_BLOCK_NUMBER(len) | SD_DATA_TRANSFER_START;

	SDIDatCon = SD_Data_Con;
	
	debug ("SDIDatCon: 0x%08x", SD_Data_Con);
	
	// Send CMD17			  
	res = sd_cmd (MMC_CMD_READ_SINGLE_BLOCK, (SD_Type == SD_TYPE_SDHC) ? (src >> 9):src, SDI_FLAG_RSP);
	debug ("CMD17 res[0] = 0x%08x", res[0]);
	
	while (len > 0)
	{
		u32 SDDatSta = SDIDatSta;
		u8 fifo = FIFO_WORDS;
		u32 SDFSta = SDIFSTA;

		//debug ("len :%d", len);
		//debug ("fifo:%d", fifo);
		//debug ("fsta:0x%08x", SDFSta);
		//debug ("dsta:0x%08x", SDDatSta);
		if (SDDatSta & (SD_DATSTA_CRCFAIL |\
						SD_DATSTA_RXCRCFAIL |\
						SD_DATSTA_DATATIMEOUT))
		{
			printf ("### SD Receive Data Fail.\r\n");
			printf ("SDDatSta = 0x%08x\r\n", SDDatSta);
			return -1;	
		}
		
		while (fifo--)
		{
			//debug ("\tfifo:%d", fifo);
			udelay (1000);
			*(des_u32++) = SDIDAT;
			if (len >= 4) {
				len -= 4;
				//debug ("\tlen: %d", len);
			}
			else {
				len = 0;
				//debug ("\tlen: %d", len);
				break;
			}
		}	
	}
	
	while (!(SDIDatSta & (1<<4))){}
	SDIDatCon = 0;
	
	if (!(SDIDatSta & (1<<4)))
		printf ("### SD Transfer not finish.\r\n");
		
	return 0;
}

int8_t sd_read (u32 src, u8 *des, u32 size)
{
	u32 end, part_start, part_end, part_len, aligned_start, aligned_end;
	u32 sd_block_size, sd_block_address;

	if (size == 0)
	{
		return 0;
	}

	if (des == NULL)
	{
		printf ("SD Card read error!\r\n");
		return -1;
	}
	
	sd_block_size = SD_BLOCK_SIZE;
	sd_block_address = ~(sd_block_size -1);

	end = src + size;
	part_start = ~sd_block_address & src;
	part_end = ~sd_block_address & end;
	aligned_start = sd_block_address & src;
	aligned_end = sd_block_address & end;

	if (part_start) {
		part_len = sd_block_size - part_start;
		if (sd_block_read(aligned_start, sd_buf, SD_BLOCK_SIZE) < 0)
			return -1;
		
		memcpy (des, sd_buf+part_start, part_len);
		des += part_len;
		src += part_len;
	}
	for (;src < aligned_end; src += sd_block_size, des += sd_block_size) {
		if ((sd_block_read(src, (u8 *)des, sd_block_size)))
			return -1;
	}

	if (part_end && src < end) {
		if (sd_block_read(aligned_end, sd_buf, sd_block_size)<0)
			return -1;
		memcpy (des, sd_buf, part_end);
	}
	

	return 0;
}