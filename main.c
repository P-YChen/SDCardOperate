#include <stdio.h>
#include "string.h"
#include "serial.h"
#include "i2c.h"
#include "m41t11.h"
#include "timer.h"
#include "sd.h"
#include "generic.h"

#define BUFFER_SIZE (1024*5)
extern int __bss_end;
extern int __bss_start;

struct __Stream_Buf {
    u32 count;
    u8 buf[BUFFER_SIZE][12];
    u8 base;
};
typedef struct __Stream_Buf Stream_Buf;

int main()
{
    char c;
    char str[200];
    int i;
    char tmpbuf[512];
    uart0_init();   // ������115200��8N1(8������λ����У��λ��1��ֹͣλ)
    timer_init();
    
    sd_init ();
    for (i=0; i<512; i++)
        tmpbuf[i] = i;

    sd_block_write (tmpbuf, 0x0, 512);
   
    while (1)
    {   
        u8 buf[BUFFER_SIZE];
        u32 vAddress;
        u32 vSize;
        u8 base = 10;
        int i, ret;
        
        debug ("buf = 0x%08x", buf);
        debug ("__bss_start = 0x%08x", __bss_start);
        debug ("__bss_end = 0x%08x", __bss_end);

        printf("\r\n##### SD Card Read #####\r\n");      
        //printf("Data format: 'year.month.day w hour:min:sec', 'w' is week day\n\r");       
        //printf("eg: 2007.08.30 4 01:16:57\n\r");        
        printf("[R] Read SD card\n\r");       
        printf("[W] Write SD card\n\r");
        printf("Enter your selection: ");

        c = getc();
        printf("%c\n\r", c);
        switch (c)
        {
            case 'r':
            case 'R':
            {
                memset (buf, 0, BUFFER_SIZE);
                printf("Enter Address and Size: ");
                i = 0;
                do
                {
                    c = getc();
                    str[i++] = c;
                    putc(c);
                } while(c != '\n' && c != '\r');
                str[i] = '\0';
                putc ('\n');
                while(--i >= 0)
                {
                    if (str[i] == 'x' || str[i] == 'X') {
                        if ((i-1)>=0 && str[i-1] == '0') {
                            base = 16;
                            str[i-1] = ' ';
                        }
                        else {
                            printf ("Address Error!\r\n");
                            goto LOOP_END;
                        }
                        
                    }
                    if (str[i] < '0' || str[i] > '9')
                        str[i] = ' ';
                }
                switch (base)
                {
                    case 10:
                        sscanf (str, "%u %u", &vAddress, &vSize);
                        break;
                    case 16:
                        sscanf (str, "%x %u", &vAddress, &vSize);
                    default:
                        break;
                }

                if (sd_read (vAddress, buf, vSize) < 0){
                    printf ("## Warning: read card fail\r\n");
                    return -1;    
                }

                dumpHEX (buf, vSize, vAddress);
                break;
            }
            
            case 'w':
            case 'W':
            {
                int tmp;
                u8 *s_str, *e_str;
                Stream_Buf argv_buf;
                argv_buf.count = 0;
                argv_buf.base = 10;

                memset (buf, 0, BUFFER_SIZE);
                printf("Enter Address and Data: ");
                i = 0;
                do
                {
                    c = getc();
                    str[i++] = c;
                    putc(c);
                } while(c != '\n' && c != '\r');
                str[i] = '\0';
                tmp = i;
                putc ('\n');
                while(--i >= 0)
                {
                    if (str[i] == 'x' || str[i] == 'X') {
                        if ((i-1)>=0 && str[i-1] == '0') {
                            argv_buf.base = 16;
                            str[i-1] = ' ';
                        }
                        else {
                            printf ("Address Error!\r\n");
                            goto LOOP_END;
                        }
                        
                    }
                    if (str[i] < '0' || str[i] > '9')
                        str[i] = ' ';
                }

                s_str = str;
                e_str = str;
                while (*e_str != '\0')
                {
                    s_str = e_str;
                    while (*s_str == ' '){
                        s_str++;
                    }
                    for (e_str=s_str; (*e_str!=' ') && (*e_str!='\0'); e_str++){}
                    memset (argv_buf.buf[argv_buf.count], 0, 12);
                    memcpy (argv_buf.buf[argv_buf.count], s_str, e_str-s_str);
                    argv_buf.count++;
                }

                //For Debug 
                for (i=0; i<argv_buf.count; i++)
                {
                    printf ("%d: %s\r\n", i, argv_buf.buf[i]);
                }

                switch (argv_buf.base)
                {
                    case 10:
                        sscanf (argv_buf.buf[0], "%u", &vAddress);
                        break;
                    case 16:
                        sscanf (argv_buf.buf[0], "%x", &vAddress);
                    default:
                        break;
                }

                // For DEBUG_FLAG
                printf ("vAddress = 0x%x\r\n", vAddress);
                printf ("argv_buf.count = %d  base = %d\r\n", argv_buf.count, argv_buf.base);

                vSize = argv_buf.count - 1;
                for (i=1; i<argv_buf.count-1; i++){
                    u8 temp;
                    sscanf (argv_buf.buf[i], "%x", &temp);
                    buf[i-1] = temp;
                    printf ("argv_buf.buf[%d]=%s   buf[%d]=0x%02x   temp=0x%02x\r\n", i, argv_buf.buf[i], i-1, buf[i-1], temp);
                }

                //For Debug
                for (i=0; i<argv_buf.count-1; i++)
                    printf ("0x%02x ", buf[i]);

                printf ("Start write date.\r\n");
                if (sd_write (buf, vAddress, vSize) < 0){
                    printf ("## Warning: write card fail\r\n");
                    return -1;    
                }
                printf ("Data write done.\r\n");
LOOP_END:
                break;
            }
        }
    }
    
    return 0;
}
