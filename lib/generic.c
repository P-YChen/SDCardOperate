#include "generic.h"
#include "string.h"
#include <stdio.h>
#include "timer.h"

void displayProgress(int progress){  
        int k = 0;  
        for (k=0; k<106; k++)  
                putchar ('\b');//将当前行全部清空，用以显示最新的进度条状态  
        int j = 0;
		putchar ('[');
        for (j=0; j<progress; j++)  
                putchar ('+');//打印进度条上已经完成的部分，用‘+’表示  
        for (j=1; j<=100-progress; j++)  
                putchar (' ');//打印进度条上还有多少没有完成的  
        putchar (']');
		printf ("%-3d%", progress);
        
        udelay (1000000);
}


u32
dumpHEX (u8 *buffer, u32 size, u32 start)
{
    u32 i = 0;

    if (buffer == NULL)
    {
        printf ("[dumpHEX]: buffer is NULL!\r\n");
        return -1;
    }

    while (i < size)
    {
        if ((size-i) >= 16)
        {
            u32 tmp = i + 16;
            u32 j = i;
            printf ("0x%08x: ", start);
            start += 16;
            for (; i<tmp; i++)
            {
                printf ("%02x", buffer[i]);
                if (i%2 == 1)
                    printf (" ");
            }

            printf ("\t");
            for (;j<tmp; j++)
            {
                if (buffer[j]>=32 && buffer[j]<=126)
                    printf ("%c", buffer[j]);
                else
                    printf (".");
            }
            printf ("\r\n");
        }
        else
        {
            u32 tmp = i+16;
            u32 j = i;
            u32 t;
            printf ("0x%08x: ", start);
            start += 16;
            for (; i<size; i++)
            {
                printf ("%02x", buffer[i]);
                if (i%2 == 1)
                    printf (" ");
            }
            for (t=0; t<tmp-size; t++)
            {
                printf ("  ");
                if (i%2 == 1)
                    printf (" ");
            }
            printf ("\t");
            for (;j<size; j++)
            {
                if (buffer[j]>='A' && buffer[j]<='Z')
                    printf ("%c", buffer[j]);
                else if(buffer[j]>='a' && buffer[j]<='z')
                    printf ("%c", buffer[j]);
                else if (buffer[j]>='0' && buffer[j]<='9')
                    printf ("%d", buffer[j]);
                else
                    printf (".");
            }
            printf ("\r\n");
        }
    }
    return i;
}