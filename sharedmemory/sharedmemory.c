#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
//#include  "restart.h"
#define PERM (S_IRUSR | S_IWUSR)

int detachandremove(int shmid, void * shmaddr)
{
    int error = 0;
    if(shmdt(shmaddr) == -1) error = errno;
    if((shmctl(shmid, IPC_RMID, NULL) == -1) && !error)
        error = errno;
    if(!error)
        return 0;
    errno = error;
    return -1;
}

ssize_t r_read(int fd, void *buf, size_t size){
    ssize_t retval;

    while(retval = read(fd, buf, size), retval == -1 && errno == EINTR);
    return retval;
}

ssize_t r_write(int fd, void *buf, size_t size){
    char *bufp;
    size_t bytestowrite;
    ssize_t byteswritten;
    size_t totalbytes;

    for(bufp = buf, bytestowrite = size, totalbytes = 0;
            bytestowrite > 0; 
            bufp +=byteswritten, bytestowrite -= byteswritten){
        byteswritten = write(fd, bufp, bytestowrite);
        if((byteswritten) = -1 && (errno!= EINTR))
            return -1;
        if(byteswritten == -1)
            byteswritten = 0;
        totalbytes += byteswritten;
    }
    return totalbytes;
}

pid_t r_wait(int *stat_loc){
    int retval;

    while(((retval = wait(stat_loc)) == -1) && (errno == EINTR));
    return retval;
}

int readwrite(int fromfd, int tofd){
    char buf[4096];
    int bytesread;
    
    if((bytesread = r_read(fromfd, buf, 4096)) == -1)
        return -1;
    if (bytesread ==0)
        return 0;
    if(r_write(tofd, buf, bytesread)==-1)
        return -1;
    return bytesread;
}

int main(int argc, char *argv[])
{
    int bytesread;
    int childpid;
    int fd,fd1, fd2;
    int id;
    int *sharedtotal;
    int totalbytes = 0;

    if(argc != 3) {
        fprintf(stderr, "Usage : %s file1 file2 \n", argv[0]);
        return 1;
    }
    if (((fd1 = open(argv[1], O_RDONLY)) == -1) ||
        ((fd2 = open(argv[2], O_RDONLY)) == -1)){
        perror("Failed to open file");
        return 1 ;
    }
    if((id = shmget(IPC_PRIVATE, sizeof(int), PERM)) == -1) {
        perror("Failed to create shared memory segment");
        return 1;
    }
    if((sharedtotal = (int*) shmat(id, NULL, 0)) == (void *)-1){
        perror("Failed to attach shared memory segment");
        if(shmctl(id, IPC_RMID, NULL)== -1)
            perror("Failed to remove memory segment");
        return 1;
    }
    if((childpid = fork())==-1) {
        perror("Failed to create child process");
        if(detachandremove(id, sharedtotal)==-1)
            perror("Failed to destroy shared memory segment");
        return 1;
    }
    if(childpid > 0)
        fd = fd1;
    else
        fd = fd2;
    while ((bytesread = readwrite(fd, STDOUT_FILENO))>0)
        totalbytes += bytesread;
    if(childpid ==0) {
        *sharedtotal = totalbytes;
        return 0;
    }
    if(r_wait(NULL)== -1)
        perror("Failed to wait for child");
    else {
        fprintf(stderr, "Bytes copied : %d by parent \n",totalbytes);
        fprintf(stderr, "Bytes copied : %d by child \n",*sharedtotal);
    }
    if(detachandremove(id, sharedtotal ) == -1) {
        perror("Failed to destroy shared memory segment");
        return 1;
    }
    return 0;
}

