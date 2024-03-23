Description:
Use TCP socket programming in C to implement a pair of iterative client and server programs.
The server waits for client’s request which is a string representing user’s password that needs to
verified against some requirements ( see below). The server performs the verification and sends
the result ( indicating whether the password passes the verification or not) back to the client
which is reported to the user at the client’s terminal

Client-side:
The client asks the user to input a string representing his/her password. The password is sent to
the server for verification and wait for a reply.
Once the reply from the server arrives, the client displays the result to the user (with
appropriate formatting) and asks the user for another round of input. Note that the connection
between the client and server should stay open, so that the client can repeat the operation again
until the user ask for termination by entering the word “done”. At this point, also the server
terminates.
The client should use command line arguments to read the IP and port number of the server,
and leave it for the kernel to assign its local socket address.

Server_side:
The server will wait for clients’ requests in the specified format as explained above,
perform the verification, sends the result back to the client, then waits for next request until it
terminates when the client enters “done”. The server verifies the password against the following
requirements:
1. Be 8-16 characters in length
2. Include characters from at least three of the following four categories:
a. Uppercase alphabet characters (A-Z)
b. Lowercase alphabet characters (a-z)
c. Numeric characters (0-9)
d. Any of the following special characters (!@#)
If the server receives an incorrect password, it should report back the password that
caused the error to the client with the error message indicating which requirement(s) failed. The
client need to display that for the user.
At the server side, the server is required to display the IP and the port number of the
client together with the password itself. Note that the server should use command line arguments to
indicate the port it will listen to. To avoid conflict when working on the course server, each
student is required to use a port number for the server using the following format: 44abc for its
connection, where abc is the least significant 3 digits of your students ID. For example, a student
whose ID is 12345 will have the server listening port = 44345
