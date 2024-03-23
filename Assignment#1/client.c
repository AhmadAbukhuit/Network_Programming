// Ahmad Abukhuit JSUT
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>

// function to print error message for users
void error (const char *msg) {
	perror(msg) ;
	exit(0) ;
}

int main (int argc , char **argv) {
	
	// check the number of argument
	if (argc < 3) {
		fprintf(stderr , "usage %s hostname port\n" , argv[0]) ;
		exit(0) ;
	}

	int sockfd , portno , n ;
	int max_line = 5000 ; 	
	struct sockaddr_in serv_addr ; 
	struct hostent *server ;

	char buffer[max_line] ; 
	
	// set the port number in server
	portno = atoi(argv[2]) ; 
	
	// open socket with server
	sockfd = socket(AF_INET , SOCK_STREAM , 0) ; 
	if (sockfd < 0) {
		error("ERROR opening socket") ; 
	}
	// set the ip address of server
	server = gethostbyname(argv[1]) ; 
	if (server == NULL) {
		fprintf(stderr , "ERROR, no such host\n") ; 
		exit(0) ; 
	}
	
	// set the information of server
	bzero ((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET ; 
	bcopy ((char *) server->h_addr , (char *) &serv_addr.sin_addr.s_addr , server->h_length) ; 
	serv_addr.sin_port = htons(portno) ; 
	
	// start connection with server
	if (connect(sockfd,(struct sockaddr *) &serv_addr , sizeof(serv_addr)) < 0 ) {
		error("ERROR Connecting") ; 
	} 
	
	// implemnt the function of client 
	while(1) {
		bzero(buffer,max_line) ; 
		printf("Client : ") ; 
		fgets(buffer,max_line,stdin) ;  	

		// send data to server
		n = write(sockfd,buffer,strlen(buffer)) ; 
		if (n < 0) {
			error("ERROR writing to socket") ; 
		}

		bzero(buffer,max_line) ; 
		
		// recevie data from server
		n = read(sockfd,buffer,max_line) ;
		if (n < 0) {
			error("ERROR reading from socket") ;
		}

		printf("Server: %s\n",buffer) ;

		int i = strncmp("Bye" , buffer , 3) ; 
		if (i == 0) {
			break ; 
		}
	}
	
	// close socket with server
	close(sockfd) ; 

	return 0 ; 
}
