#include "Client.h"
#include <cstdlib>
#include <errno.h>
#include <iostream>
#include <string>
#include <climits>

using namespace std;

/**
 * Prints usage of the program
 */
void usage() {
	cout << endl << "Usage:" << endl <<
	"./pop3client -h hostname [-p port] -u username [id]" << endl << endl <<
	"-h hostname\tHostname or host IP address" << endl <<
	"-p port\t\tTCP port of server (default: 110)" << endl <<
	"-u username\tUsername of mail account" << endl <<
	"id\t\tID of message which will be retrieved. Without this parameter a list of all messages is printed" << endl;

}

int main(int argc, char *argv[]) {
	string hostname = "";
	string username = "";
	unsigned short port = POP3_PORT;	// default
	
	// process input arguments
	char c;
	while ((c = getopt(argc, argv, "h:p:u:")) != -1) {
		switch(c) {
			case 'h' : // hostname
				hostname = (char *)optarg;
				break;
			case 'p' : // port
				port = strtoul(optarg, (char **)NULL, 10);
				if (port == ULONG_MAX && errno == ERANGE) {
					cerr << "Port number is incorrect" << endl;
					return 1;
				}
				break;
			case 'u':
				username = (char *)optarg;
				break;
			default:
				// invalid argument
				usage();
				return 1;
				break;
		}
	}

	// check eventual ID of message
	unsigned int id = 0;	// default value 0 means no ID message
	if (argc > optind) {	// there are some other parameters yet
		// we will process only the first surplus parameter, another parameters will be omitted
		// we suppose the next parameter is the id of the message
		id = (unsigned int)atoi(argv[optind]);
		// according to RFC message ID is counted from 1,
		// in case of incorrect conversion id will be 0, which is invalid value then
		if (id == 0) {
			cerr << "Message ID is in incorrect format" << endl;
			return 1;
		}
	}

	// check of mandatory parameters
	if (hostname == "" || username == "") {
		usage();
		return 1;
	}

	//
	// Main part
	//
	// now we can connect to a POP3 server
	try {
		Client client(hostname, port);
		client.login(username);

		// print message if we have a message ID, otherwise print list of all messages
		if (id != 0)
			client.getMail(id);
		else
			client.listMails();

		// finish session correctly
		client.quit();
	}
	catch (const char *e) {
		cerr << "POP3 client failed: " << e << endl;
		return 1;
	}
	catch (...) {
		cerr << "An unknown error occured, quitting..." << endl;
		return 1;
	}

	return 0;
}
