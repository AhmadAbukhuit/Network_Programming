Goal: investigate multithreading programming using TCP sockets

Description:
You are required to implement a multi-threaded client/server application that counts number of
specific characters given by a user using TCP sockets. The server will accept TCP requests on a port
which is passed as an argument in the command line when starting the server. To pass the port, you
need to use service name rather than port number (see below).

Client-Side:
The client should be multi-threaded. The main thread asks the user to enter two strings: the
second one contains the characters that need to be searched for in the first one. In addition, it creates
some other threads (depending on the size of the second string) which are responsible to send TCP
requests to the server. Each thread takes only one of the unique characters from the second string,
append it to the end of the first string, and ask the corresponding thread of the server to count the
occurrence of that specific character in the given string. In other words, counting operation is supposed
to be performed by a number of threads equal the number of unique characters in the second string.
Every output operation on the standard output should be associated with the thread-ID doing that. In
addition to printing the count of each specific character, the main thread will output also the total
number of counted characters found in the first string as the sum of all returned values of the search
threads. Then it exits.
To run the client, you need to specify the server name and the service name in your code rather that
IP address and port number. To do so, you need to edit the /etc/hosts file in your client machine
and add a line that specifies the server name as nes416_count_server and assign it the IP address of
your server machine. Also, you need to edit the /etc/services in your client machine and add a
line that specifies the port name as count_port and assign it some value (same port used when running
server, see below). Your code should use the appropriate functions to convert between hostnames/IP
addresses and service/port. See the man page of the files for more information

Server-Side:
The server application is required to be multi-threaded. The server waits for requests from the
client. For each request, a thread is created to server that request. In addition, the server main thread is
required to output the number of created threads so far.
A server thread searches the received string and return the count of the last character. It sends the
result back to the corresponding client thread and exits. Also, each thread report its finding on the
standard output together with its thread-ID. The main thread stays open for more connections.
  To run the server, you need to specify the service name (rather than the port number) on the
command line. To so, you need to edit the /etc/services in your server machine and add a line
that specifies the port name as count_port and assign any value greater than 1024 if you are working
locally. To avoid conflict if working on the course server, each student is required to use a port number
for the server using the following format: 55abc for its connection, where abc is the least significant 3
digits of your students ID. For example, a student whose ID is 12345 will have the server listening port = 55345

Example: (where Ti is the thread number)

client$

T1:Enter an Alpha-Numeric string to look in: hfgrtAf1fjhdADBFADRE123

T1:Enter characters to be counted (as one string): rA1

T1: Creating 3 threads to count for rA1

T2: # of r’s in the first string=1

T3: # of A’s in the first string=3

T4: # of 1’s in the first string=2

T1: # of total occurrences = 6

T1: exiting…

client$

*note: the output order and thread numbers might differ on your machine for client and server

Server$

T8: waiting for client request

T8: creating thread (report the thread ID)

T8: number of created threads so far = 2 changes with every creation

T9: received string: hfgrtAf1fjhdADBFADRE123r

T9: # of r’s =1

T9: sent result back, exited.

T8: waiting for client request

T8: creating thread (report the thread ID)

T8: number of created threads so far = 3 changes with every creation

T10: received string: hfgrtAf1fjhdADBFADRE1231

T10: # of 1’s =2

T10: sent result back, exited.
and so on for all reaming threads
