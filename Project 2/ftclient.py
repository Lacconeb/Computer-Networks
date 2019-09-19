#!/usr/bin/env python3

#*****************************************************************
# Brian Laccone
# CS 372 Project 2
# ftclient.py
# 
# Description: This program is designed to be a client that
# connects to a server. Depending on the arguments given, this
# program will either request the server to send a list of it's
# directory or send a specific file that was specified by the 
# user. The program will then wait for the response and info
# from the server and display it for the user to see.
#
# References for both ftserver.c and ftclient.py:
# https://beej.us/guide/bgnet/html/multi/index.html
# http://www.linuxhowtos.org/C_C++/socket.htm
# http://www.gnu.org/savannah-checkouts/gnu/libc/manual/html_node/Simple-Directory-Lister.html
# https://github.com/cirosantilli/cpp-cheat/blob/53196c683990d639fac97fbc416cb86782279739/posix/socket/inet/send_file_client.c
# ftp://gaia.cs.umass.edu/pub/kurose/ftpserver.c
# Kurose and Ross, Computer Networking: A Top-Down Approach, 7th Edition, Pearson
# http://www.bogotobogo.com/cplusplus/sockets_server_client.php
# http://www.bogotobogo.com/python/python_network_programming_server_client_file_transfer.php
# https://docs.python.org/2/library/
# http://stackoverflow.com/questions/17667903/python-socket-receive-large-amount-of-data
#*****************************************************************

import socket
import sys
from os import path
from struct import *
from time import sleep

#***************************************************
# A function sends a message to the server
#***************************************************
def sendMessage(sock, message):
    msg = bytes(message, encoding="UTF-8")
    sock.sendall(msg)


#***************************************************
# A function sends a number to the server
#***************************************************
def sendNumber(sock, message):
    num = pack('i', message)
    sock.send(num)


#***************************************************
# A function to receive a message from the server
#***************************************************
def receiveMessage(sock):
    size = sock.recv(4)
    size = unpack("I", size)
    return receiveHelper(sock, size[0])


#***************************************************
# A function to help receive an entire file 
# transmission
#***************************************************
def receiveHelper(sock, n):
    recv = ""
    while len(recv) < n:
        pack = str(sock.recv(n - len(recv)), encoding="UTF-8")
        if not pack:
            return None
        recv += pack
    return recv


#***************************************************
# A function to receive file from the server
#***************************************************
def receiveFile(conn, filename):
    buffer = receiveMessage(conn)

    with open(filename, 'w') as v:
        v.write(buffer)


#***************************************************
# A function that received and prints the directory
# message that the server sent
#***************************************************
def receiveDirectory(sock):
    size = sock.recv(4)
    size = unpack("I", size)
    recv = str(sock.recv(size[0]), encoding="UTF-8").split("\x00")

    for val in recv:
        print(val)


#***************************************************
# A function that make a request to the server
#***************************************************
def requestServer(conn, cmd, data):
    sendMessage(conn, cmd + "\0")
    sendNumber(conn, data)


#***************************************************
# A function that connects to the server
#***************************************************
def connection(host, port):
    server_conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_conn.connect((host, port))
    return server_conn


#*********************************************************
# A main function to control the inputs of the user.
# Checks if inputs are valid. The function makes contact 
# with the server. It then checks the "command" arg and 
# either sends a request for the server's directory 
# contents or sends a request for a certain file. The 
# program will exit by itself. No need for manual exit.
#*********************************************************
if __name__ == '__main__':

    argLen = len(sys.argv)

    #validate correct input
    if argLen < 5 or argLen > 6:
        print("Format: python3 ftclient.py [hostname] [port number] [command] [filename] [data port]")
        exit(1)

    host = sys.argv[1]
    port = int(sys.argv[2])
    command = sys.argv[3]
    dataPort = 0
    filename = ""

    #if number of args is 5 then set dataPort to the correct arg
    #dataPort is the last argument
    if len(sys.argv) is 5:
        dataPort = int(sys.argv[4])

    #if number of args is 6 then set dataPort and the filename
    #to the correct arg dataPort is the last argument
    elif len(sys.argv) is 6:
        filename = sys.argv[4]
        dataPort = int(sys.argv[5])

    #if the command argument is not -g or -l then print error
    if command not in ["-g", "-l"]:
        raise ValueError("The only commands accepted are -g or -l")

    #if the port or dataPort numbers are out of range, print error
    if port < 1024 or port > 65535 or dataPort < 1024 or dataPort > 65525:
        raise ValueError("Invalid port value, port must be between 1024 and 65535")

    #start server socket and make initial request
    server = connection(host, port)
    requestServer(server, command, dataPort)

    #if the command is -l, wait for the server to transmit the directory contents on the data port.
    if command == "-l":
        sleep(1)
        data = connection(host, dataPort)
        print("Receiving directory structure from {}: {}".format(host, dataPort))
        receiveDirectory(data)
        data.close()

    #if the command is g, request a file and then wait for the server to send the requested file. 
    #the file is then saved to the client's directory.
    if command == "-g":
        sendNumber(server, len(filename))
        sendMessage(server, filename + "\0")

        result = receiveMessage(server)
        if result == "no file":
            print("{}: {} says {}".format(host, port, result))
        elif result == "yes":
            print("Receiving \"{}\" from {}: {}".format(filename, host, dataPort))
            sleep(1)
            data = connection(host, dataPort)
            receiveFile(data, filename)
            print("File transfer completed")
            data.close()

    server.close()
