#include "includes.h"

#define MAXLINE 1024
#define PORT 9314
#define DEBUG 1

// ***************************************************************************
// * Read the command from the socket.
// *  Simply read a line from the socket and return it as a string.
// ***************************************************************************

//Reads in a line from a file descripter and cuts off the return characters
string readCommand(int sockfd) { 
	int count = 400;
	char* buf = new char[count];
	bzero(buf, count);
	read(sockfd, buf, count);
	string temp = string(buf);
	string t = temp.substr(0, temp.size()-2);
	//cout << "LOOK AT THIS *** \"" << t << "\"" << endl;
	return t;
}

//Reads in a line from a file descripter
string readLine(int sockfd) { 
	int count = 400;
	char* buf = new char[count];
	bzero(buf, count);
	read(sockfd, buf, count);
	string t = string(buf);
	//string t = temp.substr(0, temp.size()-2);
	//cout << "LOOK AT THIS *** \"" << t << "\"" << endl;
	return t;
}

//Writes a line to a file descripter
void writeCommand(int sockfd, string message) { 
	int count = message.length();
	char* buf = new char[count];
	//buf[2] = 'Y';
	//strcpy(buf, tmp.c_str());
	write(sockfd, message.c_str(), count);
	return;
}

//Reads in data untill a . on it's one line is read
string readData(int sockfd){    
    string line = "";
    string data = "";
    line = readLine(sockfd);
    while(line.substr(0, line.size()-2) != "."){
        data.append(line);
        cout << "Another Line" << endl;
        line = readLine(sockfd);
    }
    return data;
}

//Parses out the username from the adress
string getAdressName(string adress){

	int position = adress.find('@');
	string name = adress.substr(1, position - 1);
	return name;
}

//Parses out the host from the adress
string getAdressHost(string adress){

	int position = adress.find('@');
	string name = adress.substr(position + 1, adress.length() - position - 2);
	return name;
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
	if(commandString == "MAIL FROM")
		return 2;
	if(commandString == "RCPT TO")
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
	time_t currentTime;
	
	//Connected successfully
	writeCommand(sockfd, "220 \n");
	
	while (connectionActive) {
		
		//cout << sockfd << endl;

		// *******************************************************
		// * Read the command from the socket.
		// *******************************************************
		string inString = readCommand(sockfd);
		int pos = inString.find(':');
		
		string cmdString = inString.substr(0, pos);
		string argString = inString.substr(pos+1, inString.length()-pos);
		string eAdress = argString.substr(1, argString.length() - 2);
		string data = "";
		currentTime = time(NULL);
		string date = asctime(localtime(&currentTime));
		cout << date << endl;
		
		
	    ofstream output;
		
		
		//string cmdString = readCommand(sockfd);
		
		
		cout << "cmd string = " << cmdString << endl;
		cout << "arg string = " << argString << endl;

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
			writeCommand(sockfd, "250 Hello! You are connected to an SMTP complient server.\n");
			cout << cmdString << endl;
			break;
		case MAIL :
			seenMAIL = 1;
			reversePath = argString;
			//Reset paths
	        seenRCPT = 0;
	        seenDATA = 0;
			forwardPath = "";
	        data = "";
			cout << cmdString << endl;
	        writeCommand(sockfd, "250 \n");
			break;
		case RCPT :
			seenRCPT = 1;
			forwardPath = argString;  
			cout << cmdString << endl;
	        writeCommand(sockfd, "250 \n");
			break;
		case DATA :
		    if(!(seenMAIL && seenRCPT)){
		        writeCommand(sockfd, "Must first give a MAIL FROM and a RCPT TO.\n");
		        break;
		    }
		    data = readData(sockfd);
			cout << cmdString << endl;
			cout << data << endl;
			
	        cout << "ADRESS HOST: " << getAdressHost(forwardPath) << endl;
	        //If local host, write to file
			if(getAdressHost(forwardPath) == "localhost"){
			    //Write to file
	            output.open(getAdressName(forwardPath).c_str(), std::ios_base::app);
	            output << "From " << reversePath << " " << date;
	            output << data << endl;
	            output.close();
	        }else{
	            //If not local host, pass on to the correct server
	            writeCommand(sockfd, connectToServer(forwardPath, reversePath, data));
	        }
	        
			
			
			break;
		case RSET :
			//Reset paths
		    seenMAIL = 0;
			forwardPath = "";
	        seenRCPT = 0;
	        seenDATA = 0;
			reversePath = "";
	        //*messageBuffer = NULL;
			cout << cmdString << endl;
	        writeCommand(sockfd, "250 \n");
			break;
		case NOOP :
			cout << cmdString << endl;
			break;
		case QUIT :
			writeCommand(sockfd, "OK\n");
			connectionActive = 0;
			cout << cmdString << endl;
			break;
		default :
			writeCommand(sockfd, "Unknown command (" + cmdString + ")\n");
			cout << "Unknown command (" << cmdString << ")" << endl;
			break;
		}
	}
	
	
	writeCommand(sockfd, "221 \n");
	
	close(sockfd);

	if (DEBUG)
		cout << "Thread terminating" << endl;

}


//Thank you to http://www.linuxhowtos.org/C_C++/socket.htm for some of its sample code.
//Connects to another SMPT server and passes on an email.
string connectToServer(string forwardPath, string reversePath, string data){    
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    string code;

    char buffer[256];

    portno = atoi("25");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);    
    server = gethostbyname("exchange.mines.edu");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        cout << "ERROR connecting" << endl;
    code = readCommand(sockfd);
    cout << code << endl;
    if(code.substr(0,3) != "220")
        return "Failure: " + code;
        
    //Hello
    writeCommand(sockfd, "HELO\r\n");
    code = readCommand(sockfd);
    cout << code << endl;
    if(code.substr(0,3) != "250")
        return "Failure: " + code;
    
    //Mail From
    writeCommand(sockfd, ("MAIL FROM:" + reversePath + "\r\n").c_str());
    code = readCommand(sockfd);
    cout << code << endl;
    if(code.substr(0,3) != "250")
        return "Failure: " + code;
    //Mail To
    writeCommand(sockfd, ("RCPT TO:" + forwardPath + "\r\n").c_str());
    code = readCommand(sockfd);
    cout << code << endl;
    if(code.substr(0,3) != "250")
        return "Failure: " + code;
    //Data
    writeCommand(sockfd, "DATA\r\n");
    writeCommand(sockfd, data.c_str());
    writeCommand(sockfd, ".\r\n");
    cout << readCommand(sockfd) << endl;
    writeCommand(sockfd, "QUIT\r\n");
    cout << "SENT THE MAIL" << endl;
    close(sockfd);
    return "250 Success";
	
}  




// ***************************************************************************
// * Main
// ***************************************************************************
int main(int argc, char **argv) {

    //connectToServer("khislop@mines.edu", "khislop@mines.edu", "From: Kel\r\nTo: alsokel\r\nSubject: test3\r\nLine 1\r\nLine 2\r\n");

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
			cout << "Spawing new thread to handled connect on fd=" << *connfd << endl;

		pthread_t* threadID = new pthread_t;
		pthread_create(threadID, NULL, processConnection, (void *)connfd);
		threads.insert(threadID);
	}
}
