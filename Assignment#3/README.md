Goal: Understand how to build a concurrent server and deal with signals

Description:
You need to re-implement assignment 2 such that the server serves clients concurrently. That is,
the server (parent) creates a child for each request of the client (you need to print the port and IP
address of the client). Then it waits for the child to finish processing client’s request and prints the
process ID of the terminated child.
The connection between your client and server should stay open, so that the client can send
another request. However, instead of the using the string “done”, the exit will happen when the user
sends the client program the SIGQUIT signal. The handler will print a notifying message (for example,
“Good bye”) before the client exits. Note that in all cases, the parent server is kept running.
In addition to the previous requirements for the password, the server need to consider this new
requirement about the special characters
 Supports the following Special Character(s) (@#$^&*+=) . Note that the % symbol is
not allowed!
