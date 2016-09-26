#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdio.h>

#define MSGSZ   128

typedef struct msgbuf {
    long mtype;
    char mtext[MSGSZ];
} message_buf;

int main(){
    int msqid;
    int msqflg = IPC_CREAT | 0666;
    key_t key;
    message_buf sbuf;
    size_t buf_length;
    key = 0x1234;
    msqid = msgget(key, msqflg);
    printf("msgget : msgid = %d \n", msqid);
    sbuf.mtype =1;
    strcpy(sbuf.mtext, "Hello Wordl~");
    buf_length = strlen(sbuf.mtext)+1;
    msgsnd (msqid, &sbuf, buf_length, IPC_NOWAIT);
    printf("Msg : \"%s\" sent \n", sbuf.mtext);
}
