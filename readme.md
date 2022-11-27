## Compiling
To compile the client and server run

```$ make```

To make only the server

```$ make server```

To make only the client

```$ make client```


## Running the server/client
To run the server use,

```$ ./server ```

To run the client use,

```$ ./client <host> ```

where host is the hostname of the machine running the server

Example: ```$ ./client localhost ```


## To do list:

- create a applicaltion layer protocol over TCP that the client/server will use to communicate. Can just use HTTP if you would like. 

    we will use whatever protcol is chosen/designed to make requests from the client such as

    - send message
    - register
    
- once above is completed, implement handlers for each request type.

    if we want to store users we would need to have a database of some kind.

- securing the application

    I was thinking that we do this last. We should get the entire application working first (unsecured) then once thats done we can work on securing it.

    We can secure communications by implementing TLS.
    Read this for an example on how to use the OpenSSL TLS library in C

    https://fm4dd.com/openssl/sslconnect.shtm

    It just builds on the existing unix socket library


    This would be on a lower level than our application layer protocol. So it would be completely agnostic to what protocol we choose. 
    So if we choose HTTP then once secured we would have HTTP+TLS. If we decide to make our own application layer protocol then it would be that protocol + TLS.