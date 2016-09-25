#include "includes.h"

#define MAXLINE 1024
#define PORT 9316
#define DEBUG 1

// ***************************************************************************
// * Read the command from the socket.
// *  Simply read a line from the socket and return it as a string.
// ***************************************************************************
string readCommand(int sockfd) { 
	int count = 400;
	char* buf = new char[count];
	read(sockfd, buf, count);
	string temp = string(buf);
	string t = temp.substr(0, temp.size()-2);
	//cout << "LOOK AT THIS *** \"" << t << "\"" << endl;
	return t;
}

// ***************************************************************************
// * Parse the command.
// *  Read the string and find the command, returning the number we assoicated
// *  with that command.
// ***************************************************************************
int parseCommand(string commandString) {
	//cout << "In the parseCommand function: \"" << commandString << "\"" << endl;
	//cout << "The math: " << (commandString == string("TEST")) << endl;
	if(commandString == "HELO")
		return 1;
	if(commandString == "MAIL")
		return 2;
	if(commandString == "RCPT")
		return 3;
	if(commandString == "DATA")
		return 4;
	if(commandString == "RSET")
		return 5;
	if(commandString == "NOOP")
		return 6;
	if(commandString == "QUIT")
		return 7;
	
	return -1;
		
}

// ***************************************************************************
// * processConnection()
// *  Master function for processing thread.
// *  !!! NOTE - the IOSTREAM library and the cout varibables may or may
// *      not be thread safe depending on your system.  I use the cout
// *      statments for debugging when I know there will be just one thread
// *      but once you are processing mulClick!tiple rquests it might cause problems.
// ***************************************************************************
void* processConnection(void *arg) {


	// *******************************************************
	// * This is a little bit of a cheat, but if you end up
	// * with a FD of more than 64 bits you are in trouble
	// *******************************************************
	int sockfd = *(int *)arg;
	if (DEBUG)
		cout << "We are in the thread with fd = " << sockfd << endl;

	int connectionActive = 1;
	int seenMAIL = 0;
	int seenRCPT = 0;
	int seenDATA = 0;
	string forwardPath = "";
	string reversePath = "";
	char *messageBuffer = NULL;
	while (connectionActive) {
		
		//cout << sockfd << endl;

		// *******************************************************
		// * Read the command from the socket.
		// *******************************************************
		string cmdString = readCommand(sockfd);
		
		//cout << "cmd string = " << cmdString << endl;

		// *******************************************************
		// * Parse the command.
		// *******************************************************
		int command = parseCommand(cmdString);
		
		//cout << "command = " << command << endl;

		// *******************************************************
		// * Act on each of the commands we need to implement. 
		// *******************************************************
		switch (command) {
		case HELO :
			cout << cmdString << endl;
			break;
		case MAIL :
			cout << cmdString << endl;
			break;
		case RCPT :
			cout << cmdString << endl;
			break;
		case DATA :
			cout << cmdString << endl;
			break;
		case RSET :
			cout << cmdString << endl;
			break;
		case NOOP :
			cout << cmdString << endl;
			break;
		case QUIT :
			cout << cmdString << endl;
			break;
		default :
			cout << "Unknown command (" << cmdString<< ")" << endl;
			break;
		}
	}

	if (DEBUG)
		cout << "Thread terminating" << endl;

}



// ***************************************************************************
// * Main
// ***************************************************************************
int main(int argc, char **argv) {

       if (argc != 1) {
                cout << "useage " << argv[0] << endl;
                exit(-1);
        }

	// *******************************************************************
	// * Creating the inital socket is the same as in a client.
	// ********************************************************************
	int     listenfd = -1;
	if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		cout << "Failed to make socket " << strerror(errno) << endl;
		exit(-1);
	}
	


	// ********************************************************************
	// * The same address structure is used, however we use a wildcard
	// * for the IP address since we don't know who will be connecting.
	// ********************************************************************
	struct sockaddr_in		servaddr;
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = PF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);


    

	// ********************************************************************
	// * Binding configures the socket with the parameters we have
	// * specified in the servaddr structure.  This step is implicit in
	// * the connect() call, but must be explicitly listed for servers.
	// ********************************************************************
	if (DEBUG)
		cout << "Process has bound fd " << listenfd << " to port " << PORT << endl;
		
	if (bind(listenfd, (sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		cout << "bind() failed: " << strerror(errno) <<  endl;
		exit(-1);
	}


	// ********************************************************************
        // * Setting the socket to the listening state is the second step
	// * needed to being accepting connections.  This creates a que for
	// * connections and starts the kernel listening for connections.
        // ********************************************************************
	if (DEBUG)
		cout << "We are now listening for new connections" << endl;
		
	int listenq = 1;
	if (listen(listenfd, listenq) < 0) {
		cout << "listen() failed: " << strerror(errno) <<  endl;
		exit(-1);
	}


	// ********************************************************************
        // * The accept call will sleep, waiting for a connection.  When 
	// * a connection request comes in the accept() call creates a NEW
	// * socket with a new fd that will be used for the communication.
        // ********************************************************************
	set<pthread_t*> threads;
	while (1) {
		if (DEBUG)
			cout << "Calling accept() in master thread." << endl;
			
		int* connfd = new int;
		*connfd = -1;
		
		//int connfd = -1;
			
		//cout << "start" << endl;

		if ((*connfd = accept(listenfd, (sockaddr *) NULL, NULL)) < 0) {
			cout << "accept() failed: " << strerror(errno) <<  endl;
			exit(-1);
		}
		//cout << "connfd = " << *connfd << endl;
		
		
		
		if (DEBUG)
			cout << "Spawing new thread to handled connect on fd=" << connfd << endl;

		pthread_t* threadID = new pthread_t;
		pthread_create(threadID, NULL, processConnection, (void *)connfd);
		threads.insert(threadID);
	}
}
