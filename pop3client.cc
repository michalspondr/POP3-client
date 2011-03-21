#include "Client.h"
#include <cstdlib>
#include <errno.h>
#include <iostream>
#include <string>
#include <climits>

using namespace std;

void usage() {
	cout << "./pop3client -h hostname [-p port] -u username [id]" << endl << endl <<
	"-h hostname\tHostname or host IP address" << endl <<
	"-p port\t\tTCP port of server (default: 110)" << endl <<
	"-u username\tUsername of mail account" << endl <<
	"id\t\tID message which will be retrieved. Without this parameter a list of all messages is printed" << endl;

}

int main(int argc, char *argv[]) {
	string hostname = "";
	string username = "";
	unsigned short port = POP3_PORT;	// default
	
	// process input arguments
	char c;
	while ((c = getopt(argc, argv, "h:p:u:i")) != -1) {
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
				break;
		}
	}

	// check of mandatory parameters
	if (hostname == "" || username == "") {
		usage();
		return 1;
	}


	// now we can connect to a POP3 server
	Client client(hostname, port);
	client.login(username);

	client.listMails();
	client.getMail(2);
}
