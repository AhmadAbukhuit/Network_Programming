// Ahmad Abukhuit JUST
# include <stdio.h>
# include <stdlib.h>
# include <string.h> 
# include <unistd.h>
# include <netdb.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <arpa/inet.h> 
# include <errno.h> 

# define BUFFER_SIZE 999999
# define FILE_SIZE 256

void download_file(int client_socket, const char* filename) ;
int Socket (int family, int type, int protocol) ;
void Connect (int socket, const struct sockaddr *serv_add, socklen_t addlen) ; 
int Write (int socket, const void *buff, size_t byte) ; 
int Read (int socket, void *buff, size_t byte) ; 
void Close (int socket) ; 

int main(int argc , char *argv[]) {

	int client_socket ; 
	struct sockaddr_in server_add ; 
	char buffer[BUFFER_SIZE] ;
	char filename[FILE_SIZE];

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
	if (inet_pton(AF_INET,argv[1],&(server_add.sin_addr)) == 0) {
		printf("Error : IP address is not vaild\n") ; 
		exit(1) ; 
	}

	//creating Client UDP Socket  
	client_socket = Socket(AF_INET,SOCK_DGRAM,0) ;

	// Get and report the sending buffer size
    	int optval;
    	socklen_t optlen = sizeof(optval);
    	if (getsockopt(client_socket, SOL_SOCKET, SO_SNDBUF, &optval, &optlen) == -1) {
		printf("Error in get Send Buffer Size\n") ; 
		exit(1) ; 
	}
    	printf("Initial Sending Buffer Size: %d\n", optval);
	fflush(stdout) ; 

    	// Get and report the receiving buffer size
    	getsockopt(client_socket, SOL_SOCKET, SO_RCVBUF, &optval, &optlen);
    	printf("Initial Receiving Buffer Size: %d\n", optval);
	fflush(stdout) ; 

    	// Ask the user to enter new buffer sizes
    	printf("Enter new sending buffer size: ");
	fflush(stdout) ; 
    	scanf("%d", &optval);
    	setsockopt(client_socket, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
    	printf("New Sending Buffer Size: %d\n", optval);
	fflush(stdout) ; 

    	printf("Enter new receiving buffer size: ");
	fflush(stdout) ; 
    	scanf("%d", &optval);
    	setsockopt(client_socket, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
    	printf("New Receiving Buffer Size: %d\n", optval);
	fflush(stdout) ; 

	//define the server address
	bzero(&server_add,sizeof(server_add)) ; 
	server_add.sin_family = AF_INET ; 
	server_add.sin_port = htons(atoi(argv[2])) ; 
	server_add.sin_addr.s_addr = inet_addr(argv[1]) ;
	
	Connect(client_socket, (struct sockaddr*) &server_add,sizeof(server_add)) ; 

	// perform initial authentication with the server
	printf("#Start Authentication With Server\n") ; 
	char hello_msg = 0x55 ;
        char ack_msg = 0xAA ; 	
	Write(client_socket, &hello_msg,sizeof(hello_msg)) ; 
	printf("@Send hello msg\n") ; 
	fflush(stdout) ; 

	Read(client_socket,buffer,sizeof(buffer)) ; 
	if (buffer[0] == hello_msg) { 
		printf("@Send ack msg\n") ; 
		fflush(stdout) ; 
		Write(client_socket,&ack_msg,sizeof(ack_msg)) ;
	}
	else {
		printf("Server authentication failed\n") ; 
		close(client_socket) ;
		exit(1) ; 
	}
	
	// Receive the list of files from the server
	printf("#List of Files in Server :\n") ; 
	fflush(stdout) ; 
	while (1) {
		Read(client_socket, buffer, sizeof(buffer)) ;
		if (strcmp(buffer,"END") == 0) {
			break ;
		}
		printf("%s\n", buffer);
		bzero(buffer,sizeof(buffer)) ;
	}

    	// Select file or write done to close connection  
    	while (1) {
        	printf("\n#Enter file name for download (or 'done' to exit): ");
		fflush(stdout) ; 
		bzero(filename,sizeof(filename)) ; 
        	scanf("%s" , filename) ;  
	 
		// Remove newline character from filename
		size_t len = strlen(filename);
		if (len > 0 && filename[len - 1] == '\n') {
			filename[len - 1] = '\0' ;
		} 

        	if (strcmp(filename, "done") == 0) {
			write(client_socket, filename,strlen(filename)) ; 
			printf("System end , Good bay\n") ;
		        fflush(stdout) ; 	
            		exit(1) ;
        	}

        	download_file(client_socket, filename) ;
    	}

    	// Close the socket
    	Close(client_socket);

	return 0 ; 
}

void download_file(int client_socket, const char* filename) {
	char buffer[BUFFER_SIZE];
	
    	// Send the filename to the server
    	Write(client_socket, filename, strlen(filename));

    	// Receive the response from the server
    	Read(client_socket, buffer, sizeof(buffer));

    	if (buffer[0] == 0) {
        	printf("@File does not exist on the server.\n") ;
		fflush(stdout) ; 
        	return ; 
    	}
	else {
		printf("@File exsist on the server.\n") ; 
		fflush(stdout) ; 
	}

    	// Open a local file for writing
    	FILE *file = fopen(filename, "w") ;
    	if (file == NULL) {
        	printf("Error opening local file for writing\n") ;
		fflush(stdout) ; 
        	return ;
    	}

    	// Receive and write the file content
    	size_t total_received = 0;
    	while (1) {
		bzero(buffer,sizeof(buffer)) ; 
        	size_t recv_len = read(client_socket, buffer, BUFFER_SIZE);
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
}

int Socket (int family, int type, int protocol) {
	int sockfd ; 
	if ((sockfd = socket(family,type,protocol)) < 0 ) {
		printf("Error in socket creation\n") ; 
		exit(1) ; 
	}
	return sockfd ; 
}

void Connect (int socket, const struct sockaddr *serv_add,socklen_t addlen) {
	if (connect(socket,serv_add,addlen) == -1) {
			printf("Error in connection\n") ; 
			exit(1) ; 
	}
} 

int Write (int socket, const void *buff,size_t byte) {
	int a ; 
	if ((a = write(socket,buff,byte)) < 0) {
		printf("Error in writing\n") ; 
		exit(1) ; 
	}
}

int Read (int socket, void *buff, size_t byte) {
	int a ; 
	if ((a = read(socket,buff,byte)) < 0) {
		printf("Error in reading\n") ; 
		exit(1) ; 
	}
}

void Close (int socket) {
	if (close(socket) == -1) {
		printf("Error in closing socket\n") ; 
		exit(1) ; 
	}
}

