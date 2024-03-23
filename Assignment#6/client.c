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
# include <pthread.h>

# define BUFFERSIZE 65535
# define TEXTSIZE 256
# define THREADNUM 256

int Socket (int family, int type, int protocol, int time) ;
void Connect (int socket, const struct sockaddr *serv_add, socklen_t addlen, int time) ;
ssize_t Write (int socket, const void *buff, size_t byte) ;
ssize_t Read (int socket, void *buff, size_t byte) ;
void Close (int socket) ;
void handler_signal (int sig_num) ;
void handler_timeout (int sig_num) ;
void Fgets (char buffer[TEXTSIZE]) ;
void *ThreadFunction (void *arg) ;

struct sockaddr_in server_add, client_add ;
socklen_t serv_addr_len = sizeof(server_add) ;
socklen_t client_addr_len = sizeof(client_add) ;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER ;
int counter = 0 ;
struct Info {
    int ID ;
    char text[TEXTSIZE] ;
    char character ;
};

int main(int argc , char *argv[]) {

    char buffer[BUFFERSIZE] ;
    struct hostent *host_info ;
	struct in_addr **addr_list ;
    struct servent *serv_info ;
    pthread_t ThreadID[THREADNUM] ;

	//check for input parameter enter
	if (argc < 3) {
		printf("Please enter the IP and port\n") ;
		printf("./client [server] [service]") ;
		exit(1) ;
	}
	if (sizeof(argv[1]) > 20) {
        printf("Error : Number of sarver char more than 20 char\n") ;
        exit(1) ;
	}
	// Get server IP address by name
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
    // Get service information by name
    serv_info = getservbyname(argv[2], "tcp") ;
    if (serv_info == NULL) {
        printf("No port number on this service : %s",argv[2]) ;
        exit(1) ;
    }

    //define the server address
	bzero(&server_add,sizeof(server_add)) ;
	server_add.sin_family = AF_INET ;
	server_add.sin_port = serv_info->s_port ;
	server_add.sin_addr = *addr_list[0] ;

	signal (SIGQUIT , handler_signal) ;

    while (1) {

        char text[TEXTSIZE] ;
        char search[TEXTSIZE] ;
        struct Info info[THREADNUM] ;

        counter = 0 ;
        printf("T1 : Enter an Alpha-Numeric string to look in : ") ;
        Fgets(text) ;
        printf("T1 : Eeter character to be counted (as one string) : ") ;
        Fgets(search) ;

        printf("T1 : Creating %d threads to count for %s\n",strlen(search),search) ;

        for (int i=2 ; i<(strlen(search)+2) ; i++) {
            // Set thread info
            info[i].ID = i ;
            strcpy(info[i].text,text) ;
            info[i].character = search[i-2] ;
            // Create a thread and pass the argument
            if (pthread_create(&ThreadID[i], NULL, ThreadFunction,(void *)&info[i]) != 0) {
                printf("Error in pthread_create") ;
                exit(1) ;
            }
        }

        for (int i=2 ; i<(strlen(search)+2) ; i++) {
            if (pthread_join(ThreadID[i], NULL) != 0) {
                printf("Error in pthread_join") ;
                exit(1) ;
            }
        }

        printf("T1 : # of total occurrences = %d\n",counter) ;
    }

    return 0 ;
}

void *ThreadFunction (void *arg) {

    //Creating Client TCP Socket
    int ClientSocket ;
    ClientSocket = Socket(AF_INET,SOCK_STREAM,0,20) ;

    char buffer[BUFFERSIZE] ;
    struct Info *info = (struct Info *)arg;

    Connect(ClientSocket,(struct sockaddr*) &server_add,serv_addr_len,20) ;

    // Send the text and character as one string
    strcat(info->text, &info->character) ;
    Write(ClientSocket,info->text,sizeof(info->text)) ;

    // Receive number of character in string from server
    Read(ClientSocket,buffer,sizeof(buffer)) ;
    printf("T%d: # of %c's in the string = %s\n",info->ID,info->character,buffer) ;

    // Lock to update counter of  safely
    pthread_mutex_lock(&mtx) ;

    counter += atoi(buffer) ;

    pthread_mutex_unlock(&mtx);

    // Closse socket
    Close(ClientSocket) ;
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
    int time = 10 ;
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
    int time = 10 ;
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
	return ;
}

void handler_signal (int sig_num) {
	printf("Connection terminate from SIGQUIT") ;
	exit(1) ;
}

void handler_timeout (int sig_num) {
	printf("Error timed out\n");
    exit(1);
}

void Fgets (char buffer[TEXTSIZE]) {
    // Read input from the keyboard using fgets
    if (fgets(buffer,TEXTSIZE, stdin) != NULL) {
        // Remove the newline character if present
        size_t length = strlen(buffer);
        if (length > 0 && buffer[length - 1] == '\n') {
            buffer[length - 1] = '\0';
        }
    }
    else {
        // Handle fgets error
        printf("Error in reading from keyboard\n") ;
        exit(1) ;
    }
}

