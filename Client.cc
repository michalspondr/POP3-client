#include "Client.h"

#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>

Client::Client(const std::string& ahostname, unsigned short aport) {

	// get host IP address
	if ((hostinfo = gethostbyname(ahostname.c_str())) == NULL) {
		std::cerr << "Can't get hostname info, error = " << h_errno << std::endl;	// TODO h_errno to be rewritten!
		// TODO exception?
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
	}

	// receive greeting line
	int result = recv(sockfd, buffer, BUFLEN, 0);
	if (result != -1) {
		buffer[result] = '\0';
//		std::cout << buffer;
	}
	else {
		std::cerr << "An error occured during receiving message from POP3 server: " << std::endl;
		// TODO end?
	}
}

Client::~Client() {
	close(sockfd);
}

void Client::login(std::string& user) {
	// get password from commandline
	std::string password;
	std::cout << "Enter password: ";
	std::getline(std::cin, password);	// TODO disable onscreen printing?

	try {
		std::string response = sendReceive("USER " + user + "\n");
//		std::cout << response;
		response = sendReceive("PASS " + password + "\n");
//		std::cout << response;
	}
	catch (const char * e) {
		std::cerr << e << std::endl;
	}
	catch (...) {
		std::cerr << "An error occured during login" << std::endl;
	}
}

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
	// TODO make it usable
	std::string response = (std::string)buffer;
	if (! analyzeMessage(response)) {
		
	}
	
	return response;

}

bool Client::analyzeMessage(std::string& msg) {
	if (msg.find("+OK") != std::string::npos) {
		if (msg.substr(0,3) == "+OK") {	//		we need to be sure +OK is at the beginning
			return true;
		}
		else {
			throw("Invalid position of +OK status\n");
		}
	}
	else if (msg.find("-ERR") != std::string::npos) {
		if (msg.substr(0,4) == "-ERR") {	//	we need to be sure -ERR is at the beginning	
			return false;
		}
		else {
			throw("Invalid position of -ERR status\n");
		}
	}
	else {
		throw("Incorrect response message\n");
	}

	return false;	// just to suppress warning messages
}

void Client::sendMessage(const std::string& message) {
	int result = send(sockfd, message.c_str(), message.length(), 0);
	if (result == -1) {
		std::cerr << "Message can't be sent" << std::endl;
		return;
	}
}

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
	message = tmp;
}

unsigned int Client::listMails() {
	std::string message = "LIST\n";

	sendReceive(message);	// status message
	receiveMessage(message);	// data

	std::cout << message;

	return 0;	// TODO why it returns something?
}

void Client::getMail(unsigned int i) {
	// convert integer value to string
	std::stringstream ss;
	ss << i;
	std::string message = "RETR " + ss.str() + "\n";

	sendReceive(message);	// status message
	receiveMessage(message);	// data

	std::cout << message;
}
