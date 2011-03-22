#ifndef CLIENT_H
#define CLIENT_H

#include <string>

#define POP3_PORT 110
// Size of 512 bytes is determined by RFC 1939 - "Responses may be up to 512 characters long, including the terminating CRLF"
#define BUFLEN 512

/**
 * Simple POP3 client class
 *
 * It enables connecting to a POP3 server, login, receive a list of all messages and print message with a given ID number
 * Messages are raw and unformatted
 */
class Client {
	std::string host;				/// host name
	unsigned short port;			/// port number (0-65535)
	int sockfd;						/// socket to connect
	struct hostent *hostinfo;		/// IP address + port
	char buffer[BUFLEN+1];			/// buffer for receiving messages, 512 is max length of response message, +1 is for terminating '\0' byte

	std::string username;	/// login credentials

	std::string sendReceive(const std::string& message);	/// send a message and receive response message
	void sendMessage(const std::string& message);			/// send message to server
	void receiveMessage(std::string& message);				/// receive message from server

	bool analyzeMessage(std::string& msg);					/// check the response status
	
	public:
	Client(const std::string& ahostname, unsigned short aport=POP3_PORT);
	~Client();

	void login(std::string& user);	// login with a given username
	void listMails();				// list all emails
	void getMail(unsigned int i);	// retrieve a given email
	void quit();					// quit the POP3 server
};


#endif
