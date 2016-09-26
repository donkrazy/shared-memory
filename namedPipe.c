#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int state;
    if (argc !=2 )
    {
        fprintf(stderr,"Usage : %s [filename]\n", argv[0]);
        return 1;
    }
    state = mkfifo("/tmp/myfifo", S_IRUSR|S_IWUSR);
    if (state < 0)
    {
        perror("mkfifo error \n");
        return 1;
    }
    return 0;
}