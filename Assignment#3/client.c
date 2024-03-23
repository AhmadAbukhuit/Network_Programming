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
# include <signal.h> 
# include <errno.h>

# define MAX_LINE 512

int client_socket ; 

void signal_handler (int sig_num) ; 

int main(int argc , char *argv[]) {

	signal (SIGALRM , signal_handler) ; 

	char buffer[MAX_LINE] ;
	int n ;
        struct sockaddr_in server_add ; 

	//check for input parameter enter
	if (argc < 3) {
		printf("Please enter the IP and Port\n") ;
	        printf("./client [IP] [Port]") ; 	
		exit(0) ; 
	} 
	if (atoi(argv[2]) <= 1 && atoi(argv[2]) >= 65535) {
		printf("Error : Port number is not valid\n") ; 
	       	exit(0) ; 
	}
	if (inet_pton(AF_INET,argv[1],&(server_add.sin_addr)) == 0) {
		printf("Error : IP address is not valid\n") ; 
		exit(0) ; 
	}
	
	//creating the client socket
	client_socket = socket(AF_INET,SOCK_STREAM,0) ;
        if ( client_socket == -1 ) { 
		printf("Error in open Socket\n") ; 
		exit(0) ; 
	}

	//define the client address
	bzero(&server_add,sizeof(server_add)) ; 
	server_add.sin_family = AF_INET ; 	
	server_add.sin_port = htons(atoi(argv[2])) ;
	server_add.sin_addr.s_addr = inet_addr(argv[1]) ;  	

	//connection request
	if (connect(client_socket , (struct sockaddr*) &server_add , sizeof(server_add)) == -1 ) {
		printf("Error in Connection\n") ; 
		exit(0) ; 
	}
	
	//start chating with server
	while(1) {
		bzero(buffer,MAX_LINE) ; 
		printf("\nEnter the Password to verify it or enter done to close connection\n") ; 
		fgets(buffer,MAX_LINE,stdin) ; 

		n = write(cli{ent_socket,buffer,strlen(buffer)) ;
		if (n <= 0) {
			if (errno == EAGAIN) {
				printf("Server didn't respond\n") ;
				exit(1) ; 
			}
			else {
				printf("Error in data sending\n") ; 
				exit(1) ; 
			}
		}

		if (strcmp(buffer,"done\n") == 0) {
			printf("System end , Good bay\n") ; 
			break ; 
		}
		
		printf("Server : ") ;

		for (int i=0 ; i < 8 ; i++) {
			bzero(buffer,MAX_LINE) ; 
			n = read(client_socket,buffer,MAX_LINE) ;
			if (n <= 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					printf("Server didn't respond \n") ;
					exit(1) ; 
				}
				else {
					printf("Error in data reseiving\n") ; 
					exit(1) ;
				}	
			}
			printf("%s\n" , buffer) ; 
		}
	}
	
	close(client_socket) ;  

	return 0;

}

void signal_handler (int sig_num) { 
	printf("Error in server , Conection closed\n") ; 
	close(client_socket) ; 
	exit(1) ;
}
