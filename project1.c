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

    // create pipes
    for (int i = 0; i < k; i++){
        pipe(pipes[i]);
    }

    // create nodes

    /* int pid = fork();
    if (pid == 0){
        for (int i = 0; i < k; i++){

            printf("Creating new child process [%d]\n", i);
            nodeProcess(i);
            exit(0);
        }
    }
 */

    for (int i = 0; i < k; i++){
        int pid = fork();
        if (pid == 0){ // make child processes enter the nodeProcess function
            printf("Creating new child process [%d]\n", i);
            nodeProcess(i);
            exit(0);
        }
    }
    
    char message[100];
    int dest = 0;
    struct messageHeader hdr;

    while(1){
        // get mesage and destination
        sleep(1);
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

void nodeProcess(int nodeID){
    int left = (nodeID + k - 1) % k;
    int right = (nodeID + k) % k; // neat trick courtesy of chatGPT
    int iter = 0;
    while(1){
        printf("Entered a new iteration, nodeID = %d, iteration = %d\n", nodeID, iter);
        iter++;
        struct messageHeader hdr;

        if (read(pipes[left][0], &hdr, sizeof(hdr)) > 0){ // if reading from the pipe results in data
            printf("\nApple is at node %d\n\n", nodeID);

            // if the message is meant for this node
            if (hdr.dest == nodeID){
                printf("[DEST REACHED] Node %d received a message from Node %d -- %s\n", nodeID, left, hdr.message);
                hdr.dest = -1;
                strcpy(messageReceived, hdr.message);
                strcpy(hdr.message, "Empty");
                sleep(1);
            }
            // if the message is not meant for this node
            else{
                // if the message has reached back to the parent
                if (nodeID == 0){ 
                    printf("Apple has returned to the parent.\n");
                             
                }
                // if a non-parent node receives the message
                else{
                   printf("Node %d received the message from node %d, but it wasn't meant for it. Forwarding to node %d\n", nodeID, left, right + 1); 
                }  
            }
            write(pipes[nodeID][1], &hdr, sizeof(hdr));
            break;
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