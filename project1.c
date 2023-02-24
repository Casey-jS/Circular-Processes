#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#define MAX_LEN 10

void endGracefully(int n);
void nodeProcess(int nodeID);
void endProcess(int n);
void getMessage();

struct messageHeader{
    int dest;
    char message[100];
};

int k;
int pipes[MAX_LEN][2];

int main(void){

    while(1){
        char input[2];
        printf("Enter the number of processes: (between 1 and 9): ");
        scanf("%s", input);
        
        k = atoi(input);
        if (k <= 0){
            printf("Input must be a number greater than 0\n");
        }
        else if (k > 10){
            printf("Too many processes\n");
        }
        else{
            k++;
            break;
        }
    }
    
    // create pipes
    for (int i = 0; i < k; i++){
        pipe(pipes[i]);
    }
    
    for (int i = 1; i < k; i++){
        int pid = fork();
        if (pid == 0){ // make child processes enter the nodeProcess function
            printf("Node (process) %d created\n", i);
            nodeProcess(i);
        }
    }
    getMessage();
    return 0;
}

void getMessage(){
    char message[100];
    struct messageHeader hdr;
    int destInt;

    // install sig handler
    signal(SIGINT, endGracefully);


    while(1){
        // get message and destination
        sleep(1);
        printf("Enter a message to send ~ ");
        scanf("%s", message);
        char* dest;


        while(1){
            printf("Enter a node to send it to (between 1 and %d) ~ ", k - 1);
            scanf("%s", dest);
            destInt = atoi(dest);

            // if a string is entered
            if (destInt == 0){
                printf("Input must be a number between 1 and %d\n", k-1);
            }

            else if (destInt < 0 || destInt >= k){
                printf("Invalid node.\n");
                continue;
            }

            else if (dest == 0){
                printf("Node 0 is the parent node.\n");
            }
            else { break; }
        }
        
        // put data into struct
        hdr.dest = destInt;
        strcpy(hdr.message, message);

        printf("\nParent process handing the apple off to Node 1\n");
 
        write(pipes[0][1], &hdr, sizeof(hdr)); // write to first child to start the chain
        
        if (read(pipes[k - 1][0], &hdr, sizeof(hdr)) > 0){
            printf("\nParent process received the apple.\n\n");
        }
    }
}

void nodeProcess(int nodeID){
    int left = (nodeID + k - 1) % k;
    int right = ((nodeID + k) % k); // neat trick courtesy of chatGPT
    signal(SIGINT, endProcess);
    while(1){
        struct messageHeader hdr;

        if (read(pipes[left][0], &hdr, sizeof(hdr)) > 0){ // if reading from the pipe results in data
            printf("\nApple is at node %d\n", nodeID);

            // if the message is meant for this node
            if (hdr.dest == nodeID){
                printf("[DEST REACHED] Node %d received a message from Node %d -- %s\n", nodeID, left, hdr.message);
                char receivedMessage[100];
                strcpy(receivedMessage, hdr.message);
                printf("Node %d copied the message \"%s\" into memory\n", nodeID, receivedMessage);
                hdr.dest = -1;
                strcpy(hdr.message, "Empty");
                sleep(1);
            }
            // if the message is not meant for this node
            else{
                printf("Node %d received the message from node %d, but it wasn't meant for it. Forwarding to %s", nodeID, left,
                nodeID == k - 1 ? "parent process\n" : "next node\n"); 
                sleep(1);
            }  
            write(pipes[nodeID][1], &hdr, sizeof(hdr));
        }
    }
}
void endProcess(int n){
    printf("\nEnding node process w/ pid %d\n", getpid());
    exit(0);
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