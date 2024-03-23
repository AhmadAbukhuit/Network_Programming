Goal: investigate TCP/UDP socket programming using I/O multiplexing


Description:
Use socket programming in C to implement a client that sends/receives request using both
UDP and TCP. Similar to assignment # 4, you are requested to build a client that asks for the
server’s directory content, and download one of these files locally using iterative UDP. In addition,
you need to be able to upload a file from the client to the server using concurrent TCP.


Details:
Once you start your client, you should display a menu from which a command is chosen
**********welcome to NES 416 HW #5 *****************
1- Download a file using UDP
2- Upload a file using TCP
3- Quit
***************************************************
If the user selected 1, the same procedure as in assignment 4 is followed. If the user selected
2, he/she is asked to enter a filename that should exist locally to be transferred/uploaded to the
concurrent TCP server using a TCP connection and added to the server at its current working
directory. Once the upload completes, the corresponding child at the server child terminates and the
parent waits for more requests. If the user enters 3, both the client and the servers are terminated.
Note that the menu is kept displayed after each operation until 3 is inserted.
You need to build two servers for testing: one that works with UDP (the same one as
assignment #4) and one with TCP. Both should use the same port for receiving requests. The port
number should be specified at command line when starting the servers
Your client should take hostname and port number as command arguments. i.e. you need to
use server name in your code rather that its IP address. That is, you need to edit the /etc/hosts
file in your machine and add a line that specifies the server name as nes416_server and assign it the
IP address of your machine. In your code you should use the appropriate functions to convert
between hostnames and IP addresses
In order not to over flow the receiver’s buffer with requests, your UDP client can’t call the
server more than onec per five seconds.
Note that your client can accept/receive input from multiple sources, so you need to use I/O
multiplexing for enhanced performance
