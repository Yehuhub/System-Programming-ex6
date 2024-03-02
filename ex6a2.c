#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>        // for read/write/close
#include <sys/types.h>     /* standard system types */
#include <netinet/in.h>    /* Internet address structures */
#include <sys/socket.h>    /* socket interface functions */
#include <netdb.h>         /* host to IP resolution */

const int BUFLEN = 10;    /* maximum response size */
const int ARR_SIZE = 100;

int main(int argc, char *argv[])
{
    int rc; /* system calls return value storage */
    int my_socket;
    char rbuf[BUFLEN];
    char wbuf[BUFLEN];
    int random_number, random_index, answer, count_added = 0, count_sent = 0;
    struct addrinfo con_kind,
        *addr_info_res;
        
    if (argc != 4)
    {
        fprintf(stderr, "Wrong arguments \n");
        exit(EXIT_FAILURE);
    }
    


    srand(atoi(argv[1]));
    memset(&con_kind, 0, sizeof(con_kind));
    con_kind.ai_family = AF_UNSPEC; // AF_INET, AF_INET6
    con_kind.ai_socktype = SOCK_STREAM;

    if ((rc = getaddrinfo(argv[2], argv[3],
                          &con_kind,
                          &addr_info_res) != 0))
    {
        fprintf(stderr, "(getaddrinfo() failed %s\n", gai_strerror(rc));
        exit(EXIT_FAILURE);
    }

    my_socket = socket(addr_info_res->ai_family,   // PF_INET
                       addr_info_res->ai_socktype, // SOCK_STREAM
                       addr_info_res->ai_protocol);
    // 0 = u decide which protocal to use
    if (my_socket < 0)
    {
        perror("socket: allocation failed");
        exit(EXIT_FAILURE);
    }


    sleep(rand() % 10);

    // connect to server
    rc = connect(my_socket,
                 addr_info_res->ai_addr, // addr of server: IP+PORT
                 addr_info_res->ai_addrlen);
    if (rc){
        perror("connect failed:");
        exit(EXIT_FAILURE);
    }
    else{
        printf("connection success\n"); // need to remove
    }

    random_number = rand() % 200;
    printf("%d\n", random_number);
    sprintf(wbuf, "%d", random_number);
    //initial read before entering the loop
    rc = write(my_socket, wbuf, sizeof(wbuf));
    rc = read(my_socket, rbuf, sizeof(rbuf));
    sscanf(rbuf, "%d", &answer);
    printf("got %d from server\n", answer);


    while(1){

        //randomize the number and cell
        random_number = rand() % 200;
        random_index = rand() % 100;
        snprintf(wbuf, sizeof(wbuf),"%d %d", random_number, random_index);
        rc = write(my_socket, wbuf, sizeof(wbuf));
        count_sent++;

        //read the result from server
        rc = read(my_socket, rbuf, sizeof(rbuf));
        sscanf(rbuf, "%d", &answer);

        if(answer == 1){
            count_added++;
        }
        else if(answer == -1){
            printf("%d sent: %d, added: %d\n", atoi(argv[1]), count_sent, count_added);
            close(my_socket);
            freeaddrinfo(addr_info_res);
            exit(EXIT_SUCCESS);
        }
    }

    return EXIT_SUCCESS;
}