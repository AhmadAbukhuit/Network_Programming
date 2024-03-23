// Ahmad Abukhuit JUST
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <netdb.h>
# include <dirent.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <signal.h>
# include <errno.h>

# define BUFFER_SIZE 999999
# define READ_BUFFER 1024
# define FILE_SIZE 256

void download_file (int client_socket) ;
void upload_file (int client_socket) ;
int Socket (int family, int type, int protocol, int time) ;
void Connect (int socket, const struct sockaddr *serv_add, socklen_t addlen, int time) ;
ssize_t Write (int socket, const void *buff, size_t byte) ;
ssize_t Read (int socket, void *buff, size_t byte) ;
ssize_t Sendto (int socket, const void *buff, size_t byte, int flag, const struct sockaddr *to, socklen_t addlen) ;
ssize_t Recvfrom (int socket, void *buff, size_t byte, int flag, struct sockaddr *from, socklen_t *addlen) ;
void Close (int socket) ;
int Select (int maxfdp , fd_set *readset, fd_set *writeset, fd_set * exceptset, const struct timeval *timeout) ;
void handler_signal (int sig_num) ;
void handler_timeout (int sig_num) ;

struct sockaddr_in server_add, client_add ;
socklen_t serv_addr_len = sizeof(server_add) ;
socklen_t client_addr_len = sizeof(client_add) ;

//setup for select
fd_set readset ;
struct timeval timeout ;

int main(int argc , char *argv[]) {

    int client_socket_UDP , client_socket_TCP ;
    char buffer[BUFFER_SIZE] ;
    struct hostent *host_info ;
	struct in_addr **addr_list ;

	//check for input parameter enter
	if (argc < 3) {
		printf("Please enter the IP and port\n") ;
		printf("./client [IP] [Port}") ;
		exit(1) ;
	}
	if (atoi(argv[2]) <= 1 && atoi(argv[2]) >= 65535) {
		printf("Error : Port number is not vaild\n") ;
		exit(1) ;
	}
	if (sizeof(argv[1]) > 16) {
        printf("Error : Number of sarver char more than 16 char\n") ;
        exit(1) ;
	}
	host_info = gethostbyname(argv[1]) ;
	if ( host_info == NULL) {
		printf("Error in get IP from host name: %s\n",argv[1]) ;
		exit(1) ;
	}
	addr_list = (struct in_addr **)host_info->h_addr_list;
	if (addr_list[0] == NULL) {
        printf("No address associated with the given hostname\n") ;
        exit(1) ;
    }

    //define the server address
	bzero(&server_add,sizeof(server_add)) ;
	server_add.sin_family = AF_INET ;
	server_add.sin_port = htons(atoi(argv[2])) ;
	server_add.sin_addr = *addr_list[0] ;

	signal (SIGQUIT , handler_signal) ;

    while(1) {

		//Set timout
		timeout.tv_sec = 5 ;
		timeout.tv_usec = 0 ;

        printf("\n********** Welcome to NES416 HW#5 **********\n") ;
        printf("   Enter the number of what you want:\n") ;
        printf("    1. Download a file using UDP\n") ;
        printf("    2. Upload a file using TCP\n") ;
        printf("    3. End Connection\n") ;
        printf("********************************************\n") ;

        int num = 0;
		printf("\tEnter : ") ;
        scanf("%d", &num) ;

        if (num == 1) {

			//creating Client UDP Socket
			client_socket_UDP = Socket(AF_INET,SOCK_DGRAM,0,10) ;

			//send connection with UDP server
			Sendto(client_socket_UDP,"HELLO",sizeof("HELLO"),0,(struct sockaddr*) &server_add,sizeof(server_add)) ;

            // Receive the list of files from the server
            printf("\n#List of Files in Server :\n") ;
            while (1) {
				Recvfrom(client_socket_UDP,buffer,BUFFER_SIZE,0,(struct sockaddr*) &server_add,&serv_addr_len) ;
                if (strcmp(buffer,"END") == 0) {
                    break ;
                }
                printf("\t%s\t", buffer);
                bzero(buffer,sizeof(buffer)) ;
            }

            download_file(client_socket_UDP) ;

		}
        else if (num == 2) {

			//Creating Client TCP Socket
			client_socket_TCP = Socket(AF_INET,SOCK_STREAM,0,10) ;

			Connect(client_socket_TCP, (struct sockaddr*) &server_add,serv_addr_len,10) ;

			printf("List of files you can sent : \n") ;

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
					printf(" \t%s\t" , entry->d_name) ;
				}
				closedir(directory) ;
			}
			else {
				printf("Unable to open directory") ;
				exit(1) ;
			}

			upload_file(client_socket_TCP) ;
			Close(client_socket_TCP);

        }
        else if (num == 3) {

			// Send the end message to UDP server
			Sendto(client_socket_UDP,"END",strlen("END"),0,(struct sockaddr*) &server_add,serv_addr_len) ;

			client_socket_TCP = Socket(AF_INET,SOCK_STREAM,0,10) ;
			// Send the end message to TCP server
			Connect(client_socket_TCP, (struct sockaddr*) &server_add,serv_addr_len,10) ;
			Write(client_socket_TCP,"END",sizeof("END")) ;

			printf("Connection terminate\n") ;
			exit(1) ;
        }
        else {
            printf("Please enter number from above\n") ;
            exit(1) ;
        }
    }
    // Close the socket
    Close(client_socket_TCP);
	Close(client_socket_UDP) ;

    return 0;
}

void upload_file (int client_socket) {

	char buffer[BUFFER_SIZE] ;
	char filename[FILE_SIZE] ;

	while(1) {

		printf("\n#Enter file name to send : ") ;

		FD_ZERO(&readset) ;
		FD_SET(client_socket, &readset) ;
		FD_SET(fileno(stdin), &readset) ;

		int maxfd = (client_socket > fileno(stdin)) ? client_socket : fileno(stdin) ;

		int mode = 0 ;
		while(mode == 0) {
			Select (maxfd+1 ,&readset, NULL, NULL, &timeout) ;

			if (FD_ISSET(client_socket,&readset)) {
				char tmp[BUFFER_SIZE] ;
				bzero(tmp,BUFFER_SIZE) ;
				Recvfrom(client_socket,tmp,BUFFER_SIZE,0,(struct sockaddr*) &server_add,&serv_addr_len) ;
				printf("Received Data from Server : %s \n" , tmp) ;
			}
			if (FD_ISSET(fileno(stdin),&readset)) {
				bzero(filename,FILE_SIZE) ;
				scanf("%s",filename) ;
				mode = 1 ;
			}
		}

		// Open the file to be sent
		FILE *req_file ;
		req_file = fopen(filename, "rb") ;

		// Check if file exists
		if (req_file == NULL) {
			// print a response indicating the file does not exist
			printf("\n#File does not exists .....\n") ;
			printf("You want to reupload file Y/N : ") ;
			char enter[10] ;
			scanf("%s",enter) ;

			if (strcmp(enter,"Y") == 0 | strcmp(enter,"y") == 0) {
				continue ;
			}
			else {
				Write(client_socket,"EXIT",sizeof("EXIT")) ;
				break ;
			}
		}
		else  {
			// print a response indicating the file exists
			printf("\n#File exists .....\n") ;
			Write(client_socket,filename,sizeof(filename)) ;
			printf("Upload file : %s\n",filename) ;
		}

		// Send the file to the server in chunks
		size_t bytes_read;
		char readbuffer[READ_BUFFER] ;
		printf("@Start sending file ....\n") ;
		while ((bytes_read = fread(readbuffer, 1, sizeof(readbuffer), req_file)) > 0) {
			Write(client_socket,readbuffer,bytes_read) ;
		}
		Write(client_socket,"DONE",sizeof("DONE")) ;
		printf("@End sending file\n") ;

		// Close the file
		fclose(req_file) ;
		break ;
	}
}

void download_file(int client_socket) {

	char buffer[BUFFER_SIZE];
	char filename[FILE_SIZE] ;

	while (1) {

		// Select file
		printf("\n#Enter file name for download : ");

		FD_ZERO(&readset) ;
		FD_SET(client_socket, &readset) ;
		FD_SET(fileno(stdin), &readset) ;

		int maxfd = (client_socket > fileno(stdin)) ? client_socket : fileno(stdin) ;

		int mode = 0 ;
		while(mode == 0) {
			Select (maxfd+1 ,&readset, NULL, NULL, &timeout) ;

			if (FD_ISSET(client_socket,&readset)) {
				char tmp[BUFFER_SIZE] ;
				bzero(tmp,BUFFER_SIZE) ;
				Recvfrom(client_socket,tmp,BUFFER_SIZE,0,(struct sockaddr*) &server_add,&serv_addr_len) ;
				printf("Received Data from Server : %s \n" , tmp) ;
			}
			if (FD_ISSET(fileno(stdin),&readset)) {
				bzero(filename,sizeof(filename)) ;
				scanf("%s" , filename) ;
				mode = 1 ;
			}
		}

		// Remove newline character from filename
		size_t len = strlen(filename);
		if (len > 0 && filename[len - 1] == '\n') {
			filename[len - 1] = '\0' ;
		}

		// Send the filename to the server
		Sendto(client_socket,filename,strlen(filename),0,(struct sockaddr*) &server_add,serv_addr_len) ;

		// Receive the response from the server
		Recvfrom(client_socket,buffer,BUFFER_SIZE,0,(struct sockaddr*) &server_add,&serv_addr_len) ;

		if (buffer[0] == 0) {
			printf("@File does not exist on the server.\n") ;

			printf("##you want to continue Y/N : ") ;
			char enter[1] ;
			scanf("%s" , enter) ;
			if (strcmp(enter,"Y") == 0 | strcmp(enter,"y") == 0) {
				Sendto(client_socket,"Y",strlen("Y"),0,(struct sockaddr*) &server_add,serv_addr_len) ;
				continue ;
			}
			else if (strcmp(enter,"N") == 0 | strcmp(enter,"n") == 0) {
				Sendto(client_socket,"N",strlen("N"),0,(struct sockaddr*) &server_add,serv_addr_len) ;
				break ;
			}
			else {
				printf("Error in input\n") ;
				break ;
			}
		}
		else {
			printf("@File exsist on the server\n") ;
		}

		// Open a local file for writing
		FILE *file = fopen(filename, "wb") ;
		if (file == NULL) {
			printf("Error opening local file for writing\n") ;
			break ;
		}

		// Receive and write the file content
		size_t total_received = 0 ;
		while (1) {
			bzero(buffer,sizeof(buffer)) ;
			size_t recv_len = Recvfrom(client_socket,buffer,sizeof(buffer),0,(struct sockaddr*) &server_add,&serv_addr_len) ;
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
		break ;
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

void Connect (int socket, const struct sockaddr *serv_add,socklen_t addlen, int time) {
	// Set up the signal handler for SIGALRM
    signal(SIGALRM, handler_timeout);

	// Set the alarm for the specified timeout duration
    alarm(time);

	if (connect(socket,serv_add,addlen) < 0) {
		// Check if the error is due to the alarm signal (timeout)
        if (errno == EINTR) {
            printf("Connection attempt timed out\n");
        } else {
            perror("Error in connection\n");
        }

        // Cancel the alarm
        alarm(0);
        exit(1);
	}
	// Connection successful, cancel the alarm
    alarm(0);
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

ssize_t Sendto (int socket, const void *buff, size_t byte, int flag, const struct sockaddr *to, socklen_t addlen) {
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

int Select (int maxfdp , fd_set *readset, fd_set *writeset, fd_set * exceptset, const struct timeval *timeout) {
	int sel ;

	sel = select(maxfdp + 1, readset, NULL, NULL,NULL);

	if ((sel < 0) & (errno != EINTR)) {
		printf("Error in selcet\n") ;
		exit(1) ;
	}
	return sel ;
}

void handler_signal (int sig_num) {
	printf("Connection terminate from SIGQUIT") ;
	exit(1) ;
}

void handler_timeout (int sig_num) {
	printf("Error timed out\n");
    exit(1);
}

