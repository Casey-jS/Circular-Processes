#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#define MAX_LEN 10

void endGracefully(int n);
void nodeProcess(int pid);
void getMessage();

struct messageHeader{
    int dest;
    char message[100];
};

int k;
int pipes[MAX_LEN][2];
char messageReceived[100] = "";

int done = false;


int main(void){

    printf("Enter the number of processes: ");
    scanf("%d", &k);

    // create pipes
    for (int i = 0; i < k; i++){
        pipe(pipes[i]);
    }
    
    for (int i = 0; i < k; i++){
        int pid = fork();
        if (pid == 0){ // make child processes enter the nodeProcess function
            nodeProcess(i);
            exit(0);
        }
    }
    getMessage();
    return 0;
}

void getMessage(){
    char message[100];
    int dest = 0;
    struct messageHeader hdr;


    while(1){
        // get message and destination
        printf("Enter a message to send ~ ");
        scanf("%s", message);

        while(1){
            printf("Enter a node to send it to (between 0 and %d) ~ ", k - 1);
            scanf("%d", &dest);

            if (dest < 0 || dest >= k){
                printf("Invalid node. Try again");
            }
            else { break; }
        }
        
        // put data into struct
        hdr.dest = dest;
        strcpy(hdr.message, message);

        // install sig handler
        signal(SIGINT, endGracefully);
 
        write(pipes[0][1], &hdr, sizeof(hdr)); // write to node 0 to start the chain
        if (read(pipes[k - 1][0], &hdr, sizeof(hdr)) > 0){
            printf("\nParent process detects that circle has completed. Attempting to take a new message\n");
        }
    }
}



void nodeProcess(int nodeID){
    int left = (nodeID + k - 1) % k;
    int right = ((nodeID + k) % k); // neat trick courtesy of chatGPT
    int iter = 0;

    while(1){
        iter++;
        struct messageHeader hdr;

        if (read(pipes[left][0], &hdr, sizeof(hdr)) > 0){ // if reading from the pipe results in data
            printf("\nApple is at node %d\n", nodeID);

            // if the message is meant for this node
            if (hdr.dest == nodeID){
                printf("[DEST REACHED] Node %d received a message from Node %d -- %s\n", nodeID, left, hdr.message);
                hdr.dest = -1;
                strcpy(messageReceived, hdr.message);
                strcpy(hdr.message, "Empty");
                sleep(1);
            }

            if (nodeID == 0){
                done = true;
            }
            // if the message is not meant for this node
            else{
                printf("Node %d received the message from node %d, but it wasn't meant for it. Forwarding to node %d\n", nodeID, left, nodeID == k - 1? 0 : right + 1); 
                sleep(1);
            }  
            
            write(pipes[nodeID][1], &hdr, sizeof(hdr));
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