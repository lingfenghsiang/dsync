#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#define FILE_SIZE 10240
int main(void)
{
    u_int16_t randm;
    srand((unsigned)time(NULL));
    FILE *randomFile = fopen("random_data", "w");
    for (int j = 0; j < FILE_SIZE; j++)
    {
        for (int i = 0; i < 1024 * 512; i++)
        {
            randm = rand() & 0xffff;
            fwrite(&randm, 2, 1, randomFile);
        }
    }

    fclose(randomFile);
}