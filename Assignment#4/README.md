Description:
Use UDP socket programming in C to implement a pair of iterative client and server
programs to build a file downloading service. An initial authentication is performed at the beginning
(see below), after which the client asks for the filenames available in the server’s working directory
to choose one for downloading and saving locally.

Details:
Once the client starts, it sends a “hello” message to the server which is running on port
22334. As a result, the server sends out a one byte value equal to 01010101 (0x55) to the client and
waits for an acknowledgement from the client, which should be a one byte value equal to 10101010
(0xAA). The server, after that, prepares a list of files in its current working directory and sends that
list to the client, and waits for a request from the client containing the filename (null-terminated) to
download. The client, on the other hand, displays that list to the user and asks him/her to enter a
filename to download and sends that request to the server. if the file exists on the server, the server
will send a one byte value equal to 1 to the client; else, the server will send a one byte value equal to
0 and wait for other future connections; if file exists (after sending the (char) 1), the server opens the
local file and sends it to the client in chunks of up to1024 bytes at a time. Once finished, it waits for
another request. The client saves the file locally [make sure the client and server run at different
directories if they are on the same machine so that you can use the same filename] or you can ask the
user to enter a different filename for saving. After that, the client asks the user to enter another
filename to download, or enters the string “done” to exits. If the server receives another filename,
the above steps are repeated ( if the file exists on the server …..etc). If the received string is “done” ,
the server also exits.
In order to make sure that you don’t face a buffer limitation problem, your client code should
get and report the sending and receiving buffer sizes for the UDP sockets (SO_SNDBUF and
SO_RCVBUF). After printing the results on the screen, the program should change the send and
receive buffer sizes to some value entered by the user and report the new values again into the
screen. This step needs to be done at the beginning before start sending requests to the server
