*****************************
Instructions
*****************************
1. Run the server
2. Compile the client
3. Run the client

How to run the server:
	python chatserve.py <port#>


How to run the client:

	First compile chatclient.c:
		gcc chatclient.c -o chatclient

	Then run the compiled program:
		./chatclient <hostname> <port#>


The client must enter a username and send the first message. Both the client and server will 
alternate between sending message until either the server or client send "\quit". The client 
program will exit after that but the server must be exited by pressing Ctrl + c.


This project was successfully tested on flip1.engr.oregonstate.edu and flip3.engr.oregonstate.edu

References:
1. http://www.linuxhowtos.org/C_C++/socket.htm (heavily used)
2. https://beej.us/guide/bgnet/html/multi/index.html
3. https://docs.python.org/release/2.6.5/library/internet.html