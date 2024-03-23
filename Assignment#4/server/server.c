// Ahmad Abukhuit JUST
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <dirent.h> 
# include <sys/socket.h>
# include <sys/types.h> 
# include <arpa/inet.h>

# define BUFFER_SIZE 65535

void handle_client(int client_socket) ;
int Socket (int family, int type, int protocol) ;  
void Bind(int socket, struct sockaddr *add,socklen_t addlen) ; 
ssize_t Sendto (int socket, const void *buff, size_t byte, int flag, const struct sockaddr *to, socklen_t addlen) ; 
ssize_t Recvfrom (int socket, void *buff, size_t byte, int flag, struct sockaddr *from, socklen_t *addlen) ; 
void Close (int socket) ; 

int main(int argc , char *argv[]) {
    
    int server_socket;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_address, client_address;
    socklen_t addr_len = sizeof(client_address);

    if (argc != 2) {
	    printf("Please enter the port\n") ; 
	    printf("./server [Port]") ;
	    exit(1) ; 
    }
    if (atoi(argv[1]) <= 1 && atoi(argv[1]) >= 65535) {
	    printf("Erro : Port number is not vaild\n") ; 
	    exit(1) ; 
    }

    server_socket = Socket(AF_INET,SOCK_DGRAM,0) ; 

    // Configure server address
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    Bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) ; 

    printf("\n#Server listening on port %d...\n", atoi(argv[1]));

    char hello_msg = 0x55;
    char ack_msg = 0xAA;
    // Perform initial authentication
    printf("#Initial Authentication\n") ; 
    Recvfrom(server_socket,buffer,BUFFER_SIZE,0,(struct sockaddr*) &client_address,&addr_len) ;
    if (buffer[0] == hello_msg) {
	    printf("\tHello msg received\n") ;
	    Sendto(server_socket, &hello_msg, sizeof(hello_msg),0,(struct sockaddr*) &client_address,sizeof(client_address));
    }
    else {
	    printf("\tAuthentication failed. Exiting.\n") ;
	    Close(server_socket) ;
	    exit(1) ;
    }

    // Receive acknowledgment from the client
    Recvfrom(server_socket, buffer,BUFFER_SIZE,0,(struct sockaddr*) &client_address,&addr_len);
    printf("\tack msg received\n") ;
    if (buffer[0] != ack_msg) {
	    printf("\tAuthentication failed. Exiting.\n");
	    Close(server_socket);
	    exit(1) ;
    }

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
	    Sendto(server_socket,"END",sizeof("END"),0,(struct sockaddr*) &client_address,sizeof(client_address)) ;
    	    closedir(directory) ;
    }
    else {
	    printf("Unable to open directory") ;
	    exit(1) ;
    }

    while (1) { 

    	// Wait for the client to request a file
    	printf("\n#Waiting for the client to request a file\n") ;
	bzero(buffer,BUFFER_SIZE) ; 
    	Recvfrom(server_socket,&buffer, sizeof(buffer), 0,(struct sockaddr*) &client_address,&addr_len);

	if (strcmp(buffer,"done") == 0 ) {
		printf("Connection close from client") ; 
		exit(1) ; 
	} 
	
    	printf("#Client send request file : %s \n",buffer) ; 
	 
    	// Open the file to be sent
    	FILE *req_file = fopen(buffer, "r") ;

    	// Check if file exists
    	if (req_file == NULL) {
		// Send a response indicating the file does not exist
		printf("\n#File does not exists .....\n") ; 
		char response = 0 ;
		Sendto(server_socket, &response, sizeof(response), 0,(struct sockaddr*) &client_address,sizeof(client_address)) ;
       		continue ;
    	} 
	else  {
        	// Send a response indicating the file exists
		printf("\n#File exists .....\n") ; 
	        char response = 1;
        	Sendto(server_socket, &response, sizeof(response), 0, (struct sockaddr*) &client_address,sizeof(client_address));
    	}	

    	// Send the file to the client in chunks
    	size_t bytes_read;
    	printf("@Start sending file ....\n") ; 
    	while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, req_file)) > 0) {
        	Sendto(server_socket, buffer, bytes_read, 0, (struct sockaddr*) &client_address,sizeof(client_address));
    	}
    	printf("@End sending file\n") ; 
    
    	// Close the file
    	fclose(req_file) ;
    }

    // Close the socket
    Close(server_socket);

    return 0;
}

int Socket (int family, int type, int protocol) {
	int sockfd ; 
	if ((sockfd = socket(family,type,protocol)) < 0) {
		printf("Error in socket creation\n") ; 
		exit(1) ; 
	}
}

void Bind (int socket, struct sockaddr *add, socklen_t addlen) {
	if (bind(socket,add,addlen) < 0) {
		printf("Error in binding\n") ; 
		exit(1) ; 
	}
}

ssize_t Sendto (int socket, const void *buff, size_t byte, int flag, const struct sockaddr *to, socklen_t addlen) {
	int a ; 
	if ((a = sendto(socket,buff,byte,flag,to,addlen)) < 0 ) {
		printf("Error in sending\n") ; 
		exit(1) ; 
	}
	return a ; 
}

ssize_t Recvfrom (int socket, void *buff, size_t byte, int flag, struct sockaddr *from, socklen_t *addlen) {
	int a ; 
	if ((a = recvfrom(socket,buff,byte,flag,from,addlen)) < 0) {
		printf("Error in receiving\n") ; 
		exit(1) ; 
	}
}

void Close (int socket) {
	if (close(socket) == -1) {
		printf("Error in closing socket\n") ; 
		exit(1) ; 
	}
}
