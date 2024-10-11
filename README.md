# System Programming - ex6
## An exercise done to learn the use of the socket ipc tool


## File : ex6a1.c 
    server program, through socket receives random numbers and indices, 
    will try to insert them into an integer array.
    if it was inserted it will return 1 to the sender, if not will return
    0 to the sender.
    if array is full or 100 failed numbers received it will send -1 to the clients
    to finish.

    it will connect to local host using a port received through argv  
    server needs 3 clients.

 ## File : ex6a2.c 
    client program, connects to the socket and sends a random number, and a
    random index. if the number was inserted into the array it will read 1
    from the socket, if it is not inserted it will read 0  from the socket.
    if the array is full or 100 numbers failed to insert it will read -1 from
    the socket and exit.

    it will receive ip and port through arguments vector.
----------------------------------------------------------------
  
  ### Compile: 
  gcc ex6a1.c -Wall -o ex6a1  
  gcc ex6a2.c -Wall -o ex6a2
  ### Run: 
  ./ex6a1 <DESIRED_PORT>
  ./ex6a2 <ID> localhost <SERVER_PORT>
  ./ex6a2 <ID> localhost <SERVER_PORT>
  ./ex6a2 <ID> localhost <SERVER_PORT>
  ### input: 
  ex6a1 - SERVER_PORT
  ex6a2 - integer as id in argv, localhost, and server port
  ### output: 
  server - biggest and smallest values in the array
  client - how many random numbers sent, and how many inserted to the array
