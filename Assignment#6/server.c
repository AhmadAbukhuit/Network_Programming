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
# include <dirent.h>
# include <arpa/inet.h>
# include <signal.h>
# include <sys/select.h>
# include <sys/wait.h>
# include <errno.h>
# include <pthread.h>

# define BUFFERSIZE 65535
# define TEXTSIZE 256
# define THREADNUM 256

int Socket (int family, int type, int protocol, int time) ;
void Bind (int socket, struct sockaddr *add, socklen_t addlen) ;
void Listen (int socket, int num) ;
ssize_t Write (int socket, const void *buff, size_t byte) ;
ssize_t Read (int socket, void *buff, size_t byte) ;
void Close (int socket) ;
void signal_handler (int sig_num) ;
void handler_timeout (int sig_num) ;
void *ThreadFunction (void *arg) ;

char ReadText[TEXTSIZE] ;
int ThreadCounter = 1 ;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER ;

int main (int argc, char *argv[]) {

    if (argc != 2) {
	    printf("Please enter the port\n") ;
	    printf("./server [Port]") ;
	    exit(1) ;
    }

    // Get service information by name
    struct servent *serv_info ;
    serv_info = getservbyname(argv[1], "tcp") ;
    if (serv_info == NULL) {
        printf("No port number on this service : %s",argv[1]) ;
        exit(1) ;
    }

    int server_socket = Socket(AF_INET,SOCK_STREAM,0,10) ;

    // Configure server address
    struct sockaddr_in server_address ;
    socklen_t server_addr_len = sizeof(server_address) ;
    bzero(&server_address, sizeof(server_address)) ;
    server_address.sin_family = AF_INET ;
    server_address.sin_port = serv_info->s_port ;
    server_address.sin_addr.s_addr = INADDR_ANY ;

    // Bind the socket
    Bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) ;

    Listen(server_socket,5) ;

	signal (SIGQUIT , signal_handler) ;
    signal (SIGUSR1, signal_handler) ;

    while(1) {

        struct sockaddr_in client_address ;
        socklen_t client_addr_len = sizeof(client_address) ;

        printf("T1 : Waiting for Client Request .....\n") ;

        int client_socket ;
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

        pthread_mutex_lock(&mutex) ;

        pthread_t ThreadID ;

		// Create a thread and pass the argument
        if (pthread_create(&ThreadID, NULL, ThreadFunction,(void *)&client_socket) != 0) {
            printf("Error in pthread_create") ;
            exit(1) ;
        }
        ThreadCounter++ ;
        printf("T1 : Creating thread - %lu\n", pthread_self());
        printf("T1 : Number of created threads so far = %d\n",ThreadCounter) ;

        if (pthread_join(ThreadID, NULL) != 0) {
                printf("Error in pthread_join") ;
                exit(1) ;
        }
    }

    Close(server_socket) ;
    return 0 ;
}

void *ThreadFunction (void *arg) {

    int socket = *((int *)arg) ;
    char buffer[BUFFERSIZE] ;
    int ID = ThreadCounter ;
    int count = 0 ;

    pthread_mutex_unlock(&mutex) ;

    // Receive string from client
    Read(socket,buffer,sizeof(buffer)) ;
    printf("T%d: received string: %s\n",ID,buffer) ;

    // search for character in text
    for (int i=0; i<strlen(buffer)-1; i++) {
        if (buffer[strlen(buffer)-1] == buffer[i]) {
            count++ ;
        }
    }

    printf("T%d: # of %c’s = %d\n",ID,buffer[strlen(buffer)-1],count) ;
    //memset(buffer, 0, sizeof(buffer));
    snprintf(buffer,sizeof(buffer),"%d", count);
    Write(socket,buffer,sizeof(buffer)) ;
    printf("T%d: sent result back, exited.\n",ID) ;
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

void signal_handler (int sig_num) {
    printf("Connection terminate\n") ;
	exit(1) ;
}

void handler_timeout (int sig_num) {
	printf("Error timed out\n");
    exit(1);
}
