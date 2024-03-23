// Ahmad Abukhuit JUST
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
# include <arpa/inet.h> 

# define MAX_LINE 512 
# define ServerPort 2222

void check_password (const char *password , int client_socket) ; 

int main() {

	char buffer[MAX_LINE] ;
	int n ; 
	
	//creating the server socket 
	int server_socket = socket(AF_INET,SOCK_STREAM,0) ;
	if (server_socket == -1) {
		printf("Error in open Socket") ; 
		exit(1) ; 
	}

	//define the server address
	struct sockaddr_in server_add ; 
	bzero(&server_add,sizeof(server_add)) ; 
	server_add.sin_family = AF_INET ; 
	server_add.sin_port = htons(ServerPort) ; 
	server_add.sin_addr.s_addr = htonl(INADDR_ANY) ; 

	//bind the IP and Port to the server and check for error
	if (bind(server_socket , (struct sockaddr *) &server_add , sizeof(server_add)) == -1) {
		printf("Error in Bind function") ;
		exit(1) ; 
	}	
	//listen at the port and check for error
	if (listen(server_socket,5) == -1) {
		printf("Error in listen function") ;
		exit(1) ; 
	}		

	//accept the incoming connection from the client and check for error
	struct sockaddr_in client_add ;
        socklen_t client_add_len = sizeof(client_add) ; 	
	bzero(&client_add,sizeof(client_add)) ; 
	int client_socket = accept(server_socket,(struct sockaddr *) &client_add , &client_add_len) ;
        if (client_socket == -1) {
		printf("Error in client socket") ; 
		exit(1) ; 
	}	

	// print the client details IP address an Port number
	printf("New client connect\n") ;
	// get the client IP address
	printf("Client IP : %s\n" , inet_ntoa(client_add.sin_addr)) ; 
	//get the client Port Number
	printf("Client Port : %d \n" , ntohs(client_add.sin_port)) ;

	//start chating with client
	while(1) {

		bzero(buffer,MAX_LINE) ;
		// receive data from client and check error
		n = read(client_socket,buffer,MAX_LINE) ;
		if (n == -1) {
			printf("Error in data receiveing\n") ;
			exit(1) ;
		}

		// end connecction if receive done from client
		if (strcmp(buffer,"done\n") == 0) {
			printf("Connection closed") ;
		        break ; 	
		}
		 
		//print password received from client
		printf("Password : %s\n" , buffer) ; 
		
		// call check_passowrd function to validate password received
		check_password(buffer,client_socket) ; 

	}

	close(server_socket) ; 

	return 0 ; 
}

void check_password (const char *password , int client_socket) {

	char buff[MAX_LINE] ; 
	
	// Initialize flags to count categories
	bool upper=0 , lower=0 , digit=0 , special=0 , length=0 ; 

	// check the password length
	if (strlen(password) > 8 && strlen(password) < 16) 
		length = 1 ; 

	//loop through each character in password
	for (int i=0 ; i<strlen(password) ; i++) {
		char ch = password[i] ; 

		if (isupper(ch)) 
			upper = 1 ; 
		else if (islower(ch))
			lower = 1 ; 
		else if (isdigit(ch)) 
			digit = 1 ; 
		else if (ch == '!' || ch == '@' || ch == '#') 
			special = 1 ; 
	}

	//check if at least three categories are satisfied 
	int counter = length + upper + lower + digit + special ; 
	int n ; 

	if (counter < 3) {
		bzero(buff,MAX_LINE) ; 
		strcpy(buff, "Password not valid") ; 
		n = write(client_socket,buff,MAX_LINE) ; 
	}
	else {
		bzero(buff,MAX_LINE) ; 
		strcpy(buff, "Password is valid") ; 
		n = write(client_socket,buff,MAX_LINE) ;
	}

	if (!length) {
		bzero(buff,MAX_LINE) ; 
		strcpy(buff, "Error : passowrd length") ; 
		n = write(client_socket,buff,MAX_LINE) ; 
	}
	else {
		bzero(buff,MAX_LINE) ; 
		strcpy(buff, "Done : password length") ; 
		n = write(client_socket,buff,MAX_LINE) ; 
	}

	if (!upper) {
		bzero(buff,MAX_LINE) ;
		strcpy(buff, "Error : uppercase alphabet (A-Z)") ; 
		n = write(client_socket,buff,MAX_LINE) ; 
	}
	else {
		bzero(buff,MAX_LINE) ;
		strcpy(buff, "Done : uppercase alphabet (A-Z)") ; 
		n = write(client_socket,buff,MAX_LINE) ; 
	}

	if (!lower) {
		bzero(buff,MAX_LINE) ; 
		strcpy(buff, "Error : lowercase alphabet (a-z)") ; 
		n = write(client_socket,buff,MAX_LINE) ; 
	}
	else {
		bzero(buff,MAX_LINE) ; 
		strcpy(buff, "Done : lowercase alphabet (a-z)") ; 
		n = write(client_socket,buff,MAX_LINE) ; 
	}
	
	if (!digit) {
		bzero(buff,MAX_LINE) ; 
		strcpy(buff, "Error : numeric characters (0-9)") ; 
		n = write(client_socket,buff,MAX_LINE) ; 
	}
	else {
		bzero(buff,MAX_LINE) ; 
		strcpy(buff, "Done : numeric characters (0-9)") ; 
		n = write(client_socket,buff,MAX_LINE) ; 
	}

	if (!special) {
		bzero(buff,MAX_LINE) ; 
		strcpy(buff, "Error : special characters (!,@,#)") ;
		n = write(client_socket,buff,MAX_LINE) ; 
	}
	else {
		bzero(buff,MAX_LINE) ; 
		strcpy(buff, "Done : special characters (!,@,#)") ; 
		n = write(client_socket,buff,MAX_LINE) ; 
	}
}
