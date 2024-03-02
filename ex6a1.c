#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>

const int BUFLEN = 10;
const int ARR_SIZE = 100;

bool is_inserted(int *arr, int num, int index);
bool is_existing(int *arr, int num);
void print_min_max(int * arr);

int main(int argc, char**argv){
    int rc; // return code
    int main_socket;
    int serving_socket;
    int fd;
    int connected_clients = 0;
    int amnt_added = 0, amnt_failed = 0;
    int current_number, current_index;
    int i;
    bool run = true;

    int arr_to_fill[ARR_SIZE];
    char rbuf[BUFLEN];
    char wbuf[BUFLEN];

    fd_set rfd;
    fd_set c_rfd;
    // fd_set wfd;
    // fd_set c_wfd;

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
    con_kind.ai_flags = AI_PASSIVE;     // system will fill my IP

    if ((rc = getaddrinfo("localhost", // NULL = you set IP address
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

    rc = listen(main_socket, 5);
    if (rc)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // size_cliet_address = sizeof(cliet_address);
    her_addr_size = sizeof(her_addr);

    FD_ZERO(&rfd);
    FD_SET(main_socket, &rfd);

    //initialize array to illegal values
    for(i = 0; i < ARR_SIZE; i++){
        arr_to_fill[i] = -1;
    }

    
    while(connected_clients < 3){

        c_rfd = rfd;

        rc = select(getdtablesize(),&c_rfd, NULL, NULL, (struct timeval *)NULL);
        if(rc < 0){
            perror("select() failed\n");
            exit(EXIT_FAILURE);
        }


        if (FD_ISSET(main_socket, &c_rfd)){
            serving_socket = accept(main_socket,
                                    (struct sockaddr *)&her_addr,
                                    &her_addr_size);

            if (serving_socket >= 0){
                FD_SET(serving_socket, &rfd);
                connected_clients++;

            }

        }
    }

    connected_clients = 0;
    //read initial messages before starting
    while(connected_clients < 3){
        c_rfd = rfd;

        for (fd = main_socket + 1 ; fd < FD_SETSIZE ; fd++)
            if (FD_ISSET(fd, &c_rfd)){
                rc = read(fd, rbuf, sizeof(rbuf));
                connected_clients++;
                if (rc == 0){
                    close(fd);
                    FD_CLR(fd, &rfd);

                }
                else if(rc < 0){
                    perror("read() failed");
                    exit(EXIT_FAILURE);
                }
            }
    }
    connected_clients = 0;
    //send initial messages before starting
    while(connected_clients < 3){
        c_rfd = rfd;

        for (fd = main_socket + 1 ; fd < FD_SETSIZE ; fd++)
            if (FD_ISSET(fd, &c_rfd)){
                snprintf(wbuf, sizeof(wbuf),"%d", 5);
                write(fd, wbuf, sizeof(wbuf));
                connected_clients++;
            }
    }

    printf("main loop starting\n");
    while (run)
    {


        c_rfd = rfd;
        
        if(amnt_added >= 100 || amnt_failed >= 100){
            for (fd = main_socket + 1 ; fd < FD_SETSIZE ; fd++){
                run = false;
                if (FD_ISSET(fd, &c_rfd)){
                rc = read(fd, rbuf, sizeof(rbuf));
                    
                    if (rc == 0){
                        close(fd);
                        FD_CLR(fd, &rfd);

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
        else{
            for (fd = main_socket + 1 ; fd < FD_SETSIZE ; fd++){
                if (FD_ISSET(fd, &c_rfd)){
                    rc = read(fd, rbuf, sizeof(rbuf));
                    sscanf(rbuf,"%d %d", &current_number, &current_index);
                    if (rc == 0){
                        close(fd);
                        FD_CLR(fd, &rfd);

                    }
                    else if(rc > 0){
                        if(is_inserted(arr_to_fill, current_number, current_index)){
                            printf("%d\n", current_number);
                            amnt_added++;
                            snprintf(wbuf, sizeof(wbuf), "%d", 1);
                            write(fd, wbuf, sizeof(wbuf));
                        }
                        else{
                            amnt_failed++;
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
    print_min_max(arr_to_fill);
    return (EXIT_SUCCESS);
}

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

bool is_existing(int *arr, int num){
    for(int i = 0; i < ARR_SIZE; i++){
        if(arr[i] == num){
            return true;
        }
    }
    return false;
}

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