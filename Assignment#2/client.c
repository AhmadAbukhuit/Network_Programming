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

# define MAX_LINE 512

int main(int argc , char *argv[]) {

	char buffer[MAX_LINE] ;
	int n ; 

	//check for input parameter enter
	if (argc < 3) {
		printf("Please enter the IP and Port\n") ;
	        printf("./client [IP] [Port]") ; 	
		exit(1) ; 
	}
	
	//creating the client socket
	int client_socket = socket(AF_INET,SOCK_STREAM,0) ;
        if ( client_socket == -1 ) { 
		printf("Error in open Socket\n") ; 
		exit(1) ; 
	}

	//define the client address
	struct sockaddr_in server_add ;
	bzero(&server_add,sizeof(server_add)) ; 
	server_add.sin_family = AF_INET ; 	
	server_add.sin_port = htons(atoi(argv[2])) ;
	server_add.sin_addr.s_addr = inet_addr(argv[1]) ;  	

	//connection request
	if (connect(client_socket , (struct sockaddr*) &server_add , sizeof(server_add)) == -1 ) {
		printf("Error in Connection\n") ; 
		exit(1) ; 
	}
	
	//start chating with server
	while(1) {
		bzero(buffer,MAX_LINE) ; 
		printf("\nEnter the Password to verify it or enter done to close connection\n") ; 
		fgets(buffer,MAX_LINE,stdin) ; 

		n = write(client_socket,buffer,strlen(buffer)) ;
		if (n == -1) {
			printf("Error in data sending") ;
		}

		if (strcmp(buffer,"done\n") == 0) {
			printf("System end , Good bay\n") ; 
			break ; 
		}
		
		printf("Server : ") ;

		for (int i=0 ; i < 6 ; i++) {
			bzero(buffer,MAX_LINE) ; 
			n = read(client_socket,buffer,MAX_LINE) ;
			if (n == -1) {
				printf("Error in data receiveing\n") ;
			}
			printf("%s\n" , buffer) ; 
		}
	}
	
	close(client_socket) ;  

	return 0;
}
