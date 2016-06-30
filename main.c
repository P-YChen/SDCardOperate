#include <stdio.h>
#include "string.h"
#include "serial.h"
#include "i2c.h"
#include "m41t11.h"
#include "timer.h"
#include "sd.h"
#include "generic.h"

extern int __bss_end;
extern int __bss_start;

int main()
{
    char c;
    char str[200];
    int i;

    uart0_init();   // ������115200��8N1(8������λ����У��λ��1��ֹͣλ)
    timer_init();
    
    sd_init ();
   
    
    while (1)
    {   
        u8 buf[1024*10];
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
                            continue;
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
                break;
            }
        }
        
    }
    
    return 0;
}
