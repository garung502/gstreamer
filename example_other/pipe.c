/*
// C program to implement pipe in Linux 
#include <errno.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/wait.h> 
#include <unistd.h> 

int main() 
{ 
    // array of 2 size a[0] is for reading 
    // and a[1] is for writing over a pipe 
    int a[2]; 

    // opening of pipe using pipe(a) 
    char buff[10]; 
    if (pipe(a) == -1) 
    { 
        perror("pipe"); // error in pipe 
        exit(1); // exit from the program 
    } 

    // writing a string "code" in pipe 
    write(a[1], "code", 5); 
    printf("\n"); 

    // reading pipe now buff is equal to "code" 
    read(a[0], buff, 5); 

    // it will print "code" 
    printf("%s", buff); 
} 
*/

/*
// C program to illustrate 
// pipe system call in C 
#include <stdio.h> 
#include <unistd.h> 
#define MSGSIZE 16 
char* msg1 = "hello, world #1"; 
char* msg2 = "hello, world #2"; 
char* msg3 = "hello, world #3"; 

int main() 
{ 
    char inbuf[MSGSIZE]; 
    int p[2], i; 

    if (pipe(p) < 0) 
        exit(1); 

    //continued
    // write pipe

    write(p[1], msg1, MSGSIZE); 
    write(p[1], msg2, MSGSIZE); 
    write(p[1], msg3, MSGSIZE); 

    for (i = 0; i < 3; i++) { 
        //read pipe
        read(p[0], inbuf, MSGSIZE); 
        printf("% s\n", inbuf); 
    } 
    return 0; 
} 
*/

/*
// C program to illustrate 
// pipe system call in C 
// shared by Parent and Child 
#include <stdio.h> 
#include <unistd.h> 
#define MSGSIZE 16 
char* msg1 = "hello, world #1"; 
char* msg2 = "hello, world #2"; 
char* msg3 = "hello, world #3"; 

int main() 
{ 
    char inbuf[MSGSIZE]; 
    int p[2], pid, nbytes; 

    if (pipe(p) < 0) 
        exit(1); 

    // continued 
    if ((pid = fork()) > 0) { 
        write(p[1], msg1, MSGSIZE); 
        write(p[1], msg2, MSGSIZE); 
        write(p[1], msg3, MSGSIZE); 

        // Adding this line will 
        // not hang the program 
        // close(p[1]); 
        wait(NULL); 
    } 

    else { 
        // Adding this line will 
        // not hang the program 
        // close(p[1]); 
        while ((nbytes = read(p[0], inbuf, MSGSIZE)) > 0) 
            printf("% s\n", inbuf); 
        if (nbytes != 0)
             printf("Finished reading\n"); 
            exit(2); 
        //printf("Finished reading\n"); 
    } 
    return 0; 
} 
*/

// C program to illustrate 
// I/O system Calls 
#include<stdio.h> 
#include<string.h> 
#include<unistd.h> 
#include<fcntl.h> 

int main (void) 
{ 
    int fd[2]; 
    char buf1[12] = "hello world"; 
    char buf2[12]; 

    // assume foobar.txt is already created 
    fd[0] = open("foobar.txt", O_RDWR);      
    fd[1] = open("foobar.txt", O_RDWR); 
    
    write(fd[0], buf1, strlen(buf1));        
    write(1, buf2, read(fd[1], buf2, 12)); 

    close(fd[0]); 
    close(fd[1]); 

    return 0; 
} 



