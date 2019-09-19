*****************************
Instructions
*****************************
1. Compile ftserver.c
2. Run ftserver.c on flip1
3. Run ftclient.py on flip 2

How to compile the server:
	gcc ftserver.c -o ftserver

How to run the server:
	./ftserver <port number>

How to run the client:
	List the server's directory:
		python3 ftclient.py flip1 <port number> -l <dataport number>

	Get a file:
		python3 ftclient.py flip1 <port number> -g <filename> <dataport number>


Note: 
1. The server and client should be in different directories.
2. ftserver.c must be exited with CTRL + c
3. ftclient.py will be terminated on its own


References for both ftserver.c and ftclient.py:
	https://beej.us/guide/bgnet/html/multi/index.html
	http://www.linuxhowtos.org/C_C++/socket.htm
	http://www.gnu.org/savannah-checkouts/gnu/libc/manual/html_node/Simple-Directory-Lister.html
	https://github.com/cirosantilli/cpp-cheat/blob/53196c683990d639fac97fbc416cb86782279739/posix/socket/inet/send_file_client.c
	ftp://gaia.cs.umass.edu/pub/kurose/ftpserver.c
	Kurose and Ross, Computer Networking: A Top-Down Approach, 7th Edition, Pearson
	http://www.bogotobogo.com/cplusplus/sockets_server_client.php
	http://www.bogotobogo.com/python/python_network_programming_server_client_file_transfer.php
	https://docs.python.org/2/library/
	http://stackoverflow.com/questions/17667903/python-socket-receive-large-amount-of-data