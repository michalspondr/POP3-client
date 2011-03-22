CC=g++
CFLAGS=-Wall -O2

pop3client: Client.o pop3client.cc
	$(CC) $(CFLAGS) -o pop3client pop3client.cc Client.o

Client.o: Client.cc Client.h
	$(CC) $(CFLAGS) -c Client.cc

clean:
	        rm -f *.o pop3client

