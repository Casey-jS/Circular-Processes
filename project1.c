#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#define MAX_LEN 10

void endGracefully(int n);
void nodeProcess(int pid);

struct messageHeader{
    int dest;
    char message[100];
};

int k;
int pipes[MAX_LEN][2];


int main(void){
    pid_t nodes[MAX_LEN];

    printf("Enter the number of processes: ");
    scanf("%d", &k);
    printf("Attempting to create %d processes\n", k);

    

    // create pipes
    for (int i = 0; i < k; i++){
        pipe(pipes[i]);
    }

    // create nodes
    for (int i = 0; i < k; i++){
        int pid = fork();
        if (pid == 0){ // make child processes enter the nodeProcess function
            nodeProcess(i);
            exit(0);
        }
        nodes[i] = pid;
    }
    
    char message[100];
    int dest = 0;
    struct messageHeader hdr;

    while(1){
        // get mesage and destination
        printf("Enter a message to send ~ ");
        scanf("%s", message);

        while(1){
            printf("Enter a node to send it to (between 0 and %d) ~ ", k - 1);
            scanf("%d", &dest);

            if (dest < 0 || dest >= k){
                printf("Invalid node. Try again");
            }
            else{
                break;
            }
        }
        
        // put data into struct
        hdr.dest = dest;
        strcpy(hdr.message, message);

        // install sig handler
        signal(SIGINT, endGracefully);
        sleep(1);
        write(pipes[0][1], &hdr, sizeof(hdr));
    }

    return 0;
}

void nodeProcess(int id){

    int left = (id + k - 1) % k;
    int right = (id + k) % k; // neat trick courtesy of chatGPT

    while(1){
        struct messageHeader hdr;
        if (read(pipes[left][0], &hdr, sizeof(hdr)) > 0){
            if (hdr.dest == id){ // if this node is the destination for the message
                printf("Node %d received a message from Node %d -- %s\n", id, left, hdr.message);
                break;
            }
            else{ // if the message is not meant for this node
                printf("Node %d received the message from node %d, but it wasn't meant for it. Forwarding to node %d\n", id, left, right + 1);
                sleep(1);
                write(pipes[id][1], &hdr, sizeof(hdr));
                break;
            }
        }
    }
}

void endGracefully(int n){
    putchar('\n');
    for (int i = 0; i < k; i++){
        close(pipes[i][0]);
        close(pipes[i][1]);
        printf("Pipe %d closed!\n", i);
    }
    printf("Ending the program gracefully\n");
    exit(0);
}