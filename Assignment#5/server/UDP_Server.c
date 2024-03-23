// Group 5
// Ahmad Abukhuit 148643
// Shaimaa Ghbashneh 135342
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <dirent.h>
# include <signal.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <arpa/inet.h>
# include <errno.h>

# define BUFFER_SIZE 65535
# define READ_BUFFER 1024

int Socket (int family, int type, int protocol, int time) ;
void Bind (int socket, struct sockaddr *add, socklen_t addlen) ;
ssize_t Sendto (int socket, const void *buff, size_t byte, int flag, const struct sockaddr *to, socklen_t addlen) ;
ssize_t Recvfrom (int socket, void *buff, size_t byte, int flag, struct sockaddr *from, socklen_t *addlen) ;
void Close (int socket) ;
void signal_handler (int sig_num) ;
void handler_timeout (int sig_num) ;
void handle_download (int server_socket) ;

struct sockaddr_in server_address, client_address ;
socklen_t serv_add_len = sizeof(server_address) ;
socklen_t client_addr_len = sizeof(client_address) ;

int main (int argc, char *argv[]) {

    int server_socket ;
    char buffer[BUFFER_SIZE] ;
    struct sockaddr_in server_address, client_address ;
    socklen_t serv_add_len = sizeof(server_address) ;
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

    server_socket = Socket(AF_INET,SOCK_DGRAM,0,10) ;

    // Configure server address
    bzero(&server_address, serv_add_len);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    Bind(server_socket, (struct sockaddr*) &server_address, serv_add_len) ;

    printf("\n#Server listening on port %d...\n", atoi(argv[1]));

    signal (SIGINT , signal_handler) ;
	signal (SIGQUIT , signal_handler) ;

    while (1) {

        printf("\n#Waiting Client to send request to connect\n") ;

        // waiting client to send request
        bzero(buffer,sizeof(buffer)) ;
        Recvfrom(server_socket,buffer,sizeof(buffer),0,(struct sockaddr*) &client_address,&client_addr_len) ;

        if (strcmp(buffer,"END") == 0) {
            printf("connection closed\n") ;
            exit(1) ;
        }
        else if (strcmp(buffer,"HELLO") == 0) {

            // Send the list of files to the client
            DIR *directory ;
            struct dirent *entry ;

            // Open the current directory
            directory = opendir(".") ;

            // check if the directory opened successfully
            if (directory != NULL) {
                //read each entry in the directory
                while ((entry = readdir(directory)) != NULL) {
                // skip "." and ".."
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                        continue;
                    }
                    Sendto(server_socket,&entry->d_name,sizeof(entry->d_name),0,(struct sockaddr*) &client_address,sizeof(client_address)) ;
                }
                bzero(buffer,sizeof(buffer)) ;
                Sendto(server_socket,"END",sizeof("END"),0,(struct sockaddr*) &client_address,client_addr_len) ;
                closedir(directory) ;
            }
            else {
                printf("Unable to open directory") ;
                exit(1) ;
            }

            handle_download(server_socket) ;
        }
        else {
            printf("error in connection\n") ;
            exit(1) ;
        }
    }

    //close the socket
    Close(server_socket) ;

    return 0 ;
}

void handle_download (int server_socket) {

    char buffer[BUFFER_SIZE] ;

    while(1) {

        // Wait for the client to request a file
        printf("\n#New Client connect\n") ;
        printf("#Waiting to request a file\n") ;
        bzero(buffer,BUFFER_SIZE) ;
        Recvfrom(server_socket,&buffer, sizeof(buffer), 0,(struct sockaddr*) &client_address,&client_addr_len);

        printf("#Client send request file : %s \n",buffer) ;

        // Open the file to be sent
        FILE *req_file = fopen(buffer, "rb") ;

        // Check if file exists
        if (req_file == NULL) {
            // Send a response indicating the file does not exist
            printf("\n#File does not exists .....\n") ;
            char response = 0 ;
            Sendto(server_socket, &response, sizeof(response), 0,(struct sockaddr*) &client_address,sizeof(client_address)) ;
            bzero(buffer,BUFFER_SIZE) ;
            Recvfrom(server_socket,&buffer, sizeof(buffer), 0,(struct sockaddr*) &client_address,&client_addr_len);
            if (strcmp(buffer,"Y") == 0) {
                continue ;
            }
            else {
                return ;
            }
        }
        else  {
            // Send a response indicating the file exists
            printf("\n#File exists .....\n") ;
            char response = 1;
            Sendto(server_socket, &response, sizeof(response), 0, (struct sockaddr*) &client_address,sizeof(client_address));
        }

        // Send the file to the client in chunks
        size_t bytes_read;
        char readbuffer[READ_BUFFER] ;
        printf("@Start sending file ....\n") ;
        while ((bytes_read = fread(readbuffer, 1,sizeof(readbuffer),req_file)) > 0) {
            Sendto(server_socket, readbuffer, bytes_read, 0, (struct sockaddr*) &client_address,sizeof(client_address));
        }
        Sendto(server_socket, "DONE", sizeof("DONE"), 0, (struct sockaddr*) &client_address,sizeof(client_address));
        printf("@End sending file\n") ;

        // Close the file
        fclose(req_file) ;
        return ;
    }
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

ssize_t Sendto(int socket, const void *buff, size_t byte, int flag, const struct sockaddr *to, socklen_t addlen) {
    ssize_t a ;

    // Set up the signal handler for SIGALRM
    signal(SIGALRM, handler_timeout) ;

    // Set the alarm for the specified timeout duration
	int time = 100 ;
    alarm(time) ;

    // Attempt to send data using sendto
    if ((a = sendto(socket, buff, byte, flag, to, addlen)) < 0) {
        // Check if the error is due to the alarm signal (timeout)
        if (errno == EINTR) {
            printf("Sendto attempt timed out\n");
        } else {
            perror("Error in sending");
        }

        // Cancel the alarm
        alarm(0) ;
        exit(1) ;
    }

    // Sendto operation successful, cancel the alarm
    alarm(0) ;
    return a ;
}

ssize_t Recvfrom (int socket, void *buff, size_t byte, int flag, struct sockaddr *from, socklen_t *addlen) {
	ssize_t a;

    // Set up the signal handler for SIGALRM
    signal(SIGALRM, handler_timeout) ;

    // Set the alarm for the specified timeout duration
	int time = 100 ;
    alarm(time) ;

    // Attempt to receive data using recvfrom
    if ((a = recvfrom(socket, buff, byte, flag, from, addlen)) < 0) {
        // Check if the error is due to the alarm signal (timeout)
        if (errno == EINTR) {
            printf("Recvfrom attempt timed out\n");
        } else {
            perror("Error in receiving");
        }

        // Cancel the alarm
        alarm(0);
        exit(1);
    }

    // Recvfrom operation successful, cancel the alarm
    alarm(0);
    return a;
}

void Close (int socket) {
	if (close(socket) == -1) {
		printf("Error in closing socket\n") ;
		exit(1) ;
	}
	return ;
}

void signal_handler (int sig_num) {
    printf("Connection terminate from SIGQUIT") ;
	exit(1) ;
}

void handler_timeout (int sig_num) {
	printf("Error timed out\n");
    exit(1);
}
