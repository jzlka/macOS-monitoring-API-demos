/**
 *  @file       test.c
 *  @brief      Brief description
 *  @author     Jozef Zuzelka <jozef.zuzelka@gmail.com>
 *  @date
 *   - Created: 01.06.2020 01:45
 *   - Edited:  04.06.2020 15:50
 *  @version    1.0.0
 *  @par        gcc: Apple clang version 11.0.3 (clang-1103.0.32.62)
 *  @bug
 *  @todo
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>   // fopen(3)
#include <unistd.h>     // close(2)
#include <fcntl.h>      // open(2)

int main()
{
#if 0
    FILE* f1 = fopen("/dev/disk2", "r");
    if (f1 == NULL)
        perror("disk2");
    int res1 = flock(fileno(f1), LOCK_EX | LOCK_NB);
    if (res1)
        perror("disk2 flock");

    FILE* f2 = fopen("/dev/disk2s1", "r");
    if (f2 == NULL)
        perror("disk2s1");
    int res2 = flock(fileno(f2), LOCK_EX | LOCK_NB);
    if (res2)
        perror("disk2s1 flock");

    while (1)
        ;

    fclose(f1);
    fclose(f2);
#else

    int f1 = open("/dev/disk2", O_RDWR | O_EXLOCK);
    if (f1 == -1)
        perror("disk2");

    int f2 = open("/dev/disk2s1", O_RDWR | O_EXLOCK);
    if (f2 == -1)
        perror("disk2s1");

    while (1)
        ;

    close(f1);
    close(f2);
#endif
    return 0;
}


/* vim: set ts=4 sw=4 tw=0 et :*/
