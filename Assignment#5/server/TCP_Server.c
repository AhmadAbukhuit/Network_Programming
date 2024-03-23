// Group 5
// Ahmad Abukhuit 148643
// Shaimaa Ghbashneh 135342
# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <string.h>
# include <ctype.h>
# include <unistd.h>
# include <netdb.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <dirent.h>
# include <arpa/inet.h>
# include <signal.h>
# include <sys/select.h>
# include <sys/wait.h>
# include <errno.h>

# define BUFFER_SIZE 65535

int Socket (int family, int type, int protocol, int time) ;
void Bind (int socket, struct sockaddr *add, socklen_t addlen) ;
void Listen (int socket, int num) ;
ssize_t Write (int socket, const void *buff, size_t byte) ;
ssize_t Read (int socket, void *buff, size_t byte) ;
void Close (int socket) ;
void signal_child (int sig_num) ;
void signal_handler (int sig_num) ;
void handler_timeout (int sig_num) ;
int counter ;
int server_socket, client_socket ;

int main (int argc, char *argv[]) {

    char buffer[BUFFER_SIZE] ;
    struct sockaddr_in server_address, client_address ;
    socklen_t server_addr_len = sizeof(server_address) ;
    socklen_t client_addr_len = sizeof(client_address) ;

    if (argc != 2) {
	    printf("Please enter the port\n") ;
	    printf("./server [Port]") ;
	    exit(1) ;
    }
    if (atoi(argv[1]) <= 1 && atoi(argv[1]) >= 65535) {
	    printf("Erro : Port number is not vaild\n") ;
	    exit(1) ;
    }

    server_socket = Socket(AF_INET,SOCK_STREAM,0,10) ;

    // Configure server address
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    Bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) ;

    Listen(server_socket,5) ;

    pid_t pid ;

    signal (SIGCHLD , signal_child) ;
	signal (SIGINT , signal_child) ;
	signal (SIGQUIT , signal_handler) ;
    signal (SIGUSR1, signal_handler) ;

    while(1) {

        client_socket = accept(server_socket,(struct sockaddr*) &client_address,&client_addr_len) ;

        if (client_socket < 0) {
			if (errno == EINTR) {
				continue ;
			}
			else {
				printf("Error in client socket") ;
				exit(1) ;
			}
		}

		pid = fork() ;
		counter++ ;

        if (pid == 1) {
			close(client_socket) ;
			continue ;
		}

		if ( pid < 0) {
			printf("Error in fork()\n") ;
			exit(1) ;
		}

		if ( pid == 0 ) {
            int count = counter ;
			close(server_socket) ;

			// print the client details IP address an Port number
			printf("New client connect , client : %d \n" , counter) ;
			// get the client IP address
			printf("Client IP : %s\n" , inet_ntoa(client_address.sin_addr)) ;
			//get the client Port Number
			printf("Client Port : %d\n\n" , ntohs(client_address.sin_port)) ;

            // receive file name from client
            Read(client_socket, buffer, sizeof(buffer));

            if (strcmp(buffer,"END") == 0) {
                printf("Connection terminate\n") ;
                kill(getppid(),SIGUSR1) ;
                Close(client_socket) ;
                exit(1) ;
            }
            if(strcmp(buffer,"EXIT") == 0) {
                printf("Connection terminate\n") ;
                Close(client_socket) ;
                exit(1) ;
            }

            // Open a local file for writing
            FILE *file = fopen(buffer, "wb") ;
            if (file == NULL) {
                printf("Error opening local file for writing\n") ;
                exit(1) ;
            }

            // Receive and write the file content
            size_t total_received = 0 ;
            while (1) {
                bzero(buffer,sizeof(buffer)) ;
                size_t recv_len = Read(client_socket, buffer, sizeof(buffer));
                if (strcmp(buffer,"DONE") == 0) {
                    break ;
                }
                if (recv_len == -1) {
            		perror("Error receiving data");
            		break ;
                }
                if (recv_len == 0) {
            		// End of file
            		break;
                }
                total_received += recv_len;
                // Write received data to the local file
                size_t write_len = fwrite(buffer, 1, recv_len, file);
                if (write_len != recv_len) {
                    printf("Error writing to local file") ;
                    break ;
                }
            }

            printf("File received successfully. Total bytes received: %zu\n", total_received) ;

            // Close the local file
            fclose(file);
            exit(1) ;
        }
    }

    //close the socket
    Close(client_socket) ;

    return 0 ;
}

int Socket (int family, int type, int protocol, int time) {
	int sockfd ;

	// Set up the signal handler for SIGALRM
    signal(SIGALRM, handler_timeout);

	// Set the alarm for the specified timeout duration
    alarm(time);

	if ((sockfd = socket(family,type,protocol)) < 0 ) {
		// Check if the error is due to the alarm signal (timeout)
        if (errno == EINTR) {
            printf("Socket creation attempt timed out\n");
        } else {
            perror("Error in socket creation");
        }

        // Cancel the alarm
        alarm(0);
        exit(1);
	}
	// Socket creation successful, cancel the alarm
    alarm(0);
	return sockfd ;
}

void Bind (int socket, struct sockaddr *add, socklen_t addlen) {
	if (bind(socket,add,addlen) < 0) {
		printf("Error in binding\n") ;
		exit(1) ;
	}
	return ;
}

void Listen (int socket, int num) {
    if (listen(socket,num) < 0) {
        printf("Error in listen function\n") ;
        exit(1) ;
    }
    return ;
}

ssize_t Write (int socket, const void *buff,size_t byte) {
	ssize_t a;

    // Set up the signal handler for SIGALRM
    signal(SIGALRM, handler_timeout);

    // Set the alarm for the specified timeout duration
    int time = 100 ;
    alarm(time);

    // Attempt to write to the socket
    if ((a = write(socket, buff, byte)) < 0) {
        // Check if the error is due to the alarm signal (timeout)
        if (errno == EINTR) {
            printf("Write attempt timed out\n");
        } else {
            perror("Error in writing");
        }

        // Cancel the alarm
        alarm(0);
        exit(1);
    }

    // Write operation successful, cancel the alarm
    alarm(0);
    return a;
}

ssize_t Read (int socket, void *buff, size_t byte) {
	ssize_t a;

    // Set up the signal handler for SIGALRM
    signal(SIGALRM, handler_timeout);

    // Set the alarm for the specified timeout duration
    int time = 100 ;
    alarm(time);

    // Attempt to read from the socket
    if ((a = read(socket, buff, byte)) < 0) {
        // Check if the error is due to the alarm signal (timeout)
        if (errno == EINTR) {
            printf("Read attempt timed out\n");
        } else {
            perror("Error in reading");
        }

        // Cancel the alarm
        alarm(0);
        exit(1);
    }

    // Read operation successful, cancel the alarm
    alarm(0);
    return a;
}

void Close (int socket) {
	if (close(socket) == -1) {
		printf("Error in closing socket\n") ;
		exit(1) ;
	}
}

void signal_child (int sig_num) {
	pid_t spid ;
	int stat ;
	while ( (spid = waitpid(-1 , &stat , WNOHANG) ) > 0 ) {
		printf("Child %d terminate \n" , spid )  ;
	}
	return ;
}

void signal_handler (int sig_num) {
    printf("Connection terminate\n") ;
    Close(client_socket) ;
	exit(1) ;
}

void handler_timeout (int sig_num) {
	printf("Error timed out\n");
    exit(1);
}
