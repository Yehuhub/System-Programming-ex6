/* File : ex6a1.c 
==============================================
    Name: Yehu Raccah , ID: 315346726 , login: yehura

    server program, through socket receives random numbers and indices, 
    will try to insert them into an integer array.
    if it was inserted it will return 1 to the sender, if not will return
    0 to the sender.
    if array is full or 100 failed numbers received it will send -1 to the clients
    to finish.

    it will connect to local host using a port received through argv

*/

//----------------------include------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>

//----------------------const------------------------
const int BUFLEN = 10;
const int ARR_SIZE = 100;

//----------------------prototypes------------------------
bool is_inserted(int *arr, int num, int index);
bool is_existing(int *arr, int num);
void print_min_max(int * arr);
void main_loop(int main_socket, fd_set *rfd, int * arr_to_fill, 
            int* amnt_added, int *amnt_failed);
void initial_write(int main_socket, fd_set * rfd);
void accept_clients(int main_socket, fd_set * rfd, 
            struct sockaddr_storage * her_addr, socklen_t * her_addr_size);

//----------------------main------------------------
int main(int argc, char**argv){
    int rc; // return code
    int main_socket;

    int amnt_added = 0, amnt_failed = 0;
    int i;
    fd_set rfd;
    int arr_to_fill[ARR_SIZE];

    struct sockaddr_storage her_addr;
    socklen_t her_addr_size;
    struct addrinfo con_kind,
        *addr_info_res;


    if(argc != 2){
        perror("no port provided");
        exit(EXIT_FAILURE);
    }

    memset(&con_kind, 0, sizeof con_kind);
    con_kind.ai_family = AF_UNSPEC;
    con_kind.ai_socktype = SOCK_STREAM;
    con_kind.ai_flags = AI_PASSIVE;     

    if ((rc = getaddrinfo("localhost", 
                          argv[1],
                          &con_kind,
                          &addr_info_res)) != 0){
        fprintf(stderr, "(getaddrinfo() failed %s\n", gai_strerror(rc));
        exit(EXIT_FAILURE);
    }

    main_socket = socket(addr_info_res->ai_family,
                             addr_info_res->ai_socktype,
                             addr_info_res->ai_protocol);

    if (main_socket < 0)
    {
        perror("socket: allocation failed");
        exit(EXIT_FAILURE);
    }

    rc = bind(main_socket, addr_info_res->ai_addr,
              addr_info_res->ai_addrlen);
    if (rc)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    rc = listen(main_socket, 3);
    if (rc)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }


    her_addr_size = sizeof(her_addr);

    FD_ZERO(&rfd);
    FD_SET(main_socket, &rfd);

    //initialize array to illegal values
    for(i = 0; i < ARR_SIZE; i++){
        arr_to_fill[i] = -1;
    }

    
    accept_clients(main_socket, &rfd, &her_addr, &her_addr_size);

    //send initial messages before starting
    initial_write(main_socket, &rfd);

    main_loop(main_socket, &rfd, arr_to_fill, &amnt_added, &amnt_failed);
    
    print_min_max(arr_to_fill);
    return (EXIT_SUCCESS);
}

//function to insert a number if possible
bool is_inserted(int *arr, int num, int index){
    
    if(arr[index] == -1){
        if(!is_existing(arr, num)){
            arr[index] = num;
            return true;
        }
        return false;
    }
    return false;


}
//function to check if integer exists in array
bool is_existing(int *arr, int num){
    for(int i = 0; i < ARR_SIZE; i++){
        if(arr[i] == num){
            return true;
        }
    }
    return false;
}
//prints the max and min number in an array
void print_min_max(int * arr){
    int min = 199, max = 0;
    int i;

    for(i = 0 ; i < ARR_SIZE; i++){
        if(arr[i] != -1){
            if(arr[i] > max){
                max = arr[i];
            }
            else if(arr[i] < min){
                min = arr[i];
            }
            
        }
    }
    printf("min value is: %d, max value is: %d\n", min, max);
}

//main running loop
void main_loop(int main_socket, fd_set *rfd, int * arr_to_fill, 
            int* amnt_added, int *amnt_failed){

    bool run = true;
    int fd, rc;
    int current_number, current_index;

    char wbuf[BUFLEN];
    char rbuf[BUFLEN];

    while (run)
    {

        fd_set c_rfd = *rfd;

        //if needs to finish send -1 to every fd
        if(*amnt_added >= 100 || *amnt_failed >= 100){
            for (fd = main_socket + 1 ; fd < FD_SETSIZE ; fd++){
                run = false;
                if (FD_ISSET(fd, &c_rfd)){
                rc = read(fd, rbuf, sizeof(rbuf));
                    
                    if (rc == 0){
                        close(fd);
                        FD_CLR(fd, rfd);

                    }
                    else if(rc > 0){
                        snprintf(wbuf, sizeof(wbuf), "%d", -1);
                        write(fd, wbuf, sizeof(wbuf));
                    }
                    else{
                        perror("read() failed");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
        //read and write to each connected client
        else{
            for (fd = main_socket + 1 ; fd < FD_SETSIZE ; fd++){
                if (FD_ISSET(fd, &c_rfd)){
                    rc = read(fd, rbuf, sizeof(rbuf));
                    sscanf(rbuf,"%d %d", &current_number, &current_index);
                    if (rc == 0){
                        close(fd);
                        FD_CLR(fd, rfd);

                    }
                    else if(rc > 0){
                        if(is_inserted(arr_to_fill, current_number, current_index)){
                            printf("num: %d, index: %d\n", current_number, current_index);
                            (*amnt_added)++;
                            snprintf(wbuf, sizeof(wbuf), "%d", 1);
                            write(fd, wbuf, sizeof(wbuf));
                        }
                        else{
                            printf("num: %d, index: %d\n", current_number, current_index);
                            (*amnt_failed)++;
                            snprintf(wbuf, sizeof(wbuf), "%d", 0);
                            write(fd, wbuf, sizeof(wbuf));
                        }
                    }
                    else{
                        perror("read() failed");
                        exit(EXIT_FAILURE);
                    }
                }


        }
        }
    }

}

//first write to signal clients to start sending random numbers
void initial_write(int main_socket, fd_set * rfd){
    int connected_clients = 0;
    int fd;
    char wbuf[BUFLEN];

    while(connected_clients < 3){
        fd_set c_rfd = *rfd;

        for (fd = main_socket + 1 ; fd < FD_SETSIZE ; fd++)
            if (FD_ISSET(fd, &c_rfd)){
                snprintf(wbuf, sizeof(wbuf),"%d", 5);
                write(fd, wbuf, sizeof(wbuf));
                connected_clients++;
            }
    }
}

//function to accept clients
void accept_clients(int main_socket, fd_set * rfd, 
            struct sockaddr_storage * her_addr, socklen_t * her_addr_size){
    int connected_clients = 0;
    int rc;
    int serving_socket;
    while(connected_clients < 3){

        fd_set c_rfd = *rfd;

        rc = select(getdtablesize(),&c_rfd, NULL, NULL, (struct timeval *)NULL);
        if(rc < 0){
            perror("select() failed\n");
            exit(EXIT_FAILURE);
        }


        if (FD_ISSET(main_socket, &c_rfd)){
            serving_socket = accept(main_socket,
                                    (struct sockaddr *) her_addr,
                                    her_addr_size);

            if (serving_socket >= 0){
                FD_SET(serving_socket, rfd);
                connected_clients++;

            }

        }
    }
}

