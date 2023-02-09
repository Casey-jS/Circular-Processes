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
char messageReceived[100] = "";


int main(void){

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
        printf("Creating new child process [%d]\n", i);
        if (pid == 0){ // make child processes enter the nodeProcess function
            nodeProcess(i);
            exit(0);
        }
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
            else { break; }
        }
        
        // put data into struct
        hdr.dest = dest;
        strcpy(hdr.message, message);

        // install sig handler
        signal(SIGINT, endGracefully);
        sleep(1);
        write(pipes[0][1], &hdr, sizeof(hdr)); // write to node 0 to start the chain
        printf("Message has made it back to the parent -- %s\n", messageReceived);
    }

    return 0;
}

void nodeProcess(int id){

    int left = (id + k - 1) % k;
    int right = (id + k) % k; // neat trick courtesy of chatGPT

    while(1){
        struct messageHeader hdr;
        if (read(pipes[left][0], &hdr, sizeof(hdr)) > 0){ // if reading from the pipe results in data
            printf("\nApple is at node %d\n\n", id);
            if (hdr.dest == id){ // if this node is the destination for the message
                printf("[DEST REACHED] Node %d received a message from Node %d -- %s\n", id, left, hdr.message);
                hdr.dest = -1;
                strcpy(messageReceived, hdr.message);
                strcpy(hdr.message, "Empty");
                sleep(1);
                write(pipes[id][1], &hdr, sizeof(hdr));
                break;
            }
            else{ // if the message is not meant for this node
                if (strcmp(hdr.message, "Empty") == 0){
                    if (id == 0){
                        printf("Apple has returned to the parent.\n");
                        printf("Enter a new message to send:");
                        char message[100];
                        scanf("%s", message);
                        struct messageHeader newHdr;
                        strcpy(newHdr.message, message);
                        newHdr.dest = 5;
                        write(pipes[id][1], &newHdr, sizeof(newHdr));
                    }
                    printf("Node %d received the empty message (it has already been delivered)\n", id);                    
                }
                else{
                    printf("Node %d received the message from node %d, but it wasn't meant for it. Forwarding to node %d\n", id, left, right + 1);
                }
                sleep(1); // sleep to avoid timing issues
                write(pipes[id][1], &hdr, sizeof(hdr)); // write to the pipe so the next node can read
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