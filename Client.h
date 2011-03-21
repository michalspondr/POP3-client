#ifndef CLIENT_H
#define CLIENT_H

#include <string>

#define POP3_PORT 110
#define BUFLEN 512

class Client {
	std::string host;				/// host name
	unsigned short port;			/// port number (0-65535)
	int sockfd;						/// socket to connect
	struct hostent *hostinfo;
	char buffer[BUFLEN];				/// buffer for receiving messages, 512 is max length of response message

	std::string username, password;	/// login credentials

	std::string sendReceive(const std::string& message);	/// send a message and receive response message
	void sendMessage(const std::string& message);		/// send message to server
	void receiveMessage(std::string& message);				/// receive message from server

	bool analyzeMessage(std::string& msg);						/// check the response status
	
	public:
	Client(const std::string& ahostname, unsigned short aport=POP3_PORT);
	~Client();

	void login(std::string& user);
	void listMails();
	void getMail(unsigned int i);
	void quit();
};


#endif
