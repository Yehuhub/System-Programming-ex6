/* File : ex6a2.c 
==============================================
    Name: Yehu Raccah , ID: 315346726 , login: yehura

    client program, connects to the socket and sends a random number, and a
    random index. if the number was inserted into the array it will read 1
    from the socket, if it is not inserted it will read 0  from the socket.
    if the array is full or 100 numbers failed to insert it will read -1 from
    the socket and exit.

    it will receive ip and port through arguments vector.

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

//----------------------const------------------------
const int BUFLEN = 10;    
const int ARR_SIZE = 100;

//----------------------main------------------------
int main(int argc, char *argv[])
{
    int rc; 
    int my_socket;
    char rbuf[BUFLEN];
    char wbuf[BUFLEN];
    int random_number, random_index;
    int answer; // answer from the server
    int count_added = 0, count_sent = 0;
    struct addrinfo con_kind,
        *addr_info_res;
        
    if (argc != 4)
    {
        fprintf(stderr, "Wrong arguments \n");
        exit(EXIT_FAILURE);
    }
    
    srand(atoi(argv[1])); //seed using id

    memset(&con_kind, 0, sizeof(con_kind));
    con_kind.ai_family = AF_UNSPEC; 
    con_kind.ai_socktype = SOCK_STREAM;

    if ((rc = getaddrinfo(argv[2], argv[3],
                          &con_kind,
                          &addr_info_res) != 0))
    {
        fprintf(stderr, "(getaddrinfo() failed %s\n", gai_strerror(rc));
        exit(EXIT_FAILURE);
    }

    my_socket = socket(addr_info_res->ai_family,   
                       addr_info_res->ai_socktype, 
                       addr_info_res->ai_protocol);

    if (my_socket < 0)
    {
        perror("socket: allocation failed");
        exit(EXIT_FAILURE);
    }

    // connect to server
    rc = connect(my_socket,
                 addr_info_res->ai_addr, // addr of server: IP+PORT
                 addr_info_res->ai_addrlen);
    if (rc){
        perror("connect failed:");
        exit(EXIT_FAILURE);
    }
    

    //initial read before entering the loop
    rc = read(my_socket, rbuf, sizeof(rbuf));
    sscanf(rbuf, "%d", &answer);


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