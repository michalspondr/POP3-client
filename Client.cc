#include "Client.h"

#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <unistd.h>

/**
 * Constructor class connect to a socket and receives a greeting line from POP3 server
 */
Client::Client(const std::string& ahostname, unsigned short aport) {

	// get host IP address
	if ((hostinfo = gethostbyname(ahostname.c_str())) == NULL) {
		std::cerr << "Can't get hostname" << std::endl;
		throw "Can't get host name";
	}

	// create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		std::cerr << "Can't open socket, error = " << strerror(errno) << std::endl;
	}
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;
	address.sin_port = htons(aport);
	int len = sizeof(address);

	// connect
	if ((connect(sockfd, (struct sockaddr *)&address, len)) != 0) {
		std::cerr << "Can't connect to server, error = " << strerror(errno) << std::endl;
		throw "Can't connect to server";
	}

	// receive greeting line
	// we just receive it, there is no need to print it
	// one recv() is enough because according to RFC message can't be longer than BUFLEN (=512) octets
	int result = recv(sockfd, buffer, BUFLEN, 0);
	if (result != -1) {
		buffer[result] = '\0';
	}
	else {
		std::cerr << "An error occured during receiving greeting message from POP3 server" << std::endl;
		throw "Error during receiving greeting line";
	}
}

Client::~Client() {
}

/**
 * Login using a given username
 * Password is read from command line and it is not printed while typing
 */
void Client::login(std::string& user) {
	// get password from commandline
	char *pass = getpass("Enter password (won't be printed):");

	std::string response;
	// USER
	try {
		response = sendReceive("USER " + user + "\n");
	}
	catch (const char * e) {
		std::string tmp(e);
		if (tmp == "Error response") {
			// USER command can receive an -ERR message even if the user exists! (security reasons).
			// If we receive -ERR for USER command, we ignore it
		}
		else throw e;
	}
	catch (...) {
		std::cerr << "An error occured during login" << std::endl;
		throw "Login error";
	}

	// PASS
	try {
		response = sendReceive("PASS " + (std::string)pass + "\n");
	}
	catch (const char * e) {
		std::string tmp(e);
		if (tmp == "Error response") {
			std::cerr << "Login is unsuccessful" << std::endl;
			throw e;
		}
	}
	catch (...) {
		std::cerr << "An error occured during login" << std::endl;
		throw "Login error";
	}
}

/**
 * This method sends a message and receive the corresponding response message (not a data part!)
 * It analyzes whether the message is +OK or -ERR and throws exception in case of error
 */
std::string Client::sendReceive(const std::string& message) {
	// send
	int result = send(sockfd, message.c_str(), message.length(), 0);
	if (result == -1) {
		std::cerr << "Message can't be sent" << std::endl;
	}
	
	// receive
	result = recv(sockfd, buffer, BUFLEN, 0);
	buffer[result] = '\0';
	if (result == -1) {
		std::cerr << "Message can't be received" << std::endl;
		close(sockfd);
	}

	// we've got the message, now check it
	std::string response = (std::string)buffer;
	if (! analyzeMessage(response)) {
		throw "Error response";
	}
	
	return response;
}

/**
 * Analyze a message whether it's +OK or -ERR
 * Returns true in case of +OK, otherwise false
 */
bool Client::analyzeMessage(std::string& msg) {
	if (msg.find("+OK") != std::string::npos) {
		if (msg.substr(0,3) == "+OK") {	//		we need to be sure +OK is at the beginning
			return true;
		}
		else {
			throw("Invalid position of +OK status");
		}
	}
	else if (msg.find("-ERR") != std::string::npos) {
		if (msg.substr(0,4) == "-ERR") {	//	we need to be sure -ERR is at the beginning	
			return false;
		}
		else {
			throw("Invalid position of -ERR status");
		}
	} else {
		throw("Incorrect response message");
	}

	return false;	// just to suppress warning messages
}

/**
 * Simply send a message to socket
 */
void Client::sendMessage(const std::string& message) {
	int result = send(sockfd, message.c_str(), message.length(), 0);
	if (result == -1) {
		std::cerr << "Message can't be sent" << std::endl;
		throw "Message can't be sent";
	}
}

/**
 * Simple receive a message
 * It can read multi-line messages, too
 *
 * This method doesn't check response status (+OK/-ERR) messages! It is supposed to read data part of message only!
 */
void Client::receiveMessage(std::string& message) {
	int result=0;
	std::string tmp;
	while ((result = recv(sockfd, buffer, BUFLEN, 0)) > 0) {
		buffer[result] = '\0';
		tmp += buffer;
		// if the message ends with CRLF, we don't continue on reading from socket
		if (tmp.length() >= 2) { 
			if (tmp.substr( tmp.length()-2, 2) == "\r\n") {
				break;
			}
		}
		// else {
		//  message doesn't end with CRLF -> it's not the end of message, we need to read further
		//}
	}
	message = tmp;	// final response
}

/**
 * List all messages at POP3 server using the LIST command
 */
void Client::listMails() {
	std::string message = "LIST\n";

	sendReceive(message);	// status message
	receiveMessage(message);	// data
	// TODO remove last dot + CRLF

	std::cout << message;
}

/**
 * Retrieve a given email using its ID number
 */
void Client::getMail(unsigned int i) {
	// convert integer value to string
	std::stringstream ss;
	ss << i;
	std::string message = "RETR " + ss.str() + "\n";

	try {
		sendReceive(message);	// status message
		receiveMessage(message);	// data
	}
	catch (const char * e) {
		std::string tmp(e);
		if (tmp == "Error response") {
			std::cerr << "Can't get message " << i << std::endl;
			throw e;
		}
	}
	catch (...) {
		std::cerr << "An error occured during receiving message " << i << std::endl;
	}

	std::cout << message;
}

/**
 * Quit the POP3 session
 */
void Client::quit() {
	try {
		sendReceive("QUIT\n");
//		close(sockfd);
	}
	catch (...) {
		// an error occured but we don't care, we are quitting anyway
	}
}
