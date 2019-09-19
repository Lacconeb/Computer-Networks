#*****************************************************************
# Brian Laccone
# CS 372 Project 1
# chatserve.py
# 
# References:
# 1. http://www.linuxhowtos.org/C_C++/socket.htm (heavily used)
# 2. https://beej.us/guide/bgnet/html/multi/index.html
# 3. https://docs.python.org/release/2.6.5/library/internet.html
#
# Description: This program is designed to be a server that
# clients can connect to. The connection is made through API
# sockets through a TCP connection. The client connects to 
# the server and sends a message. Both the server and client
# will alternate between sending messages until either the
# server or client send "\quit". This program will not end after 
# the connectin is lost. It will wait until another client 
# connections. This program must be exited by pressing Ctrl + c
#*****************************************************************

import socket
import os
import sys

#***********************************
#A function to start up the server
#***********************************
def StartUp():

    #set the portnumber to the first argument
    portno = int(sys.argv[1])

    #creates a socket
    sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    #bind to the portnumber
    sockfd.bind(('',portno))

    #listen for clients that want to connect
    sockfd.listen(1)

    return sockfd 

#***************************************************
#A function that receives a message from the client
#***************************************************
def ReceiveMessage(connection):

    return connection.recv(500)

#**********************************************
#A function that sends a message to the client
#**********************************************
def SendMessage(connection):
	#prompts user to input a message
    userInput = raw_input("server: ")

    #send message to the client
    connection.send("server: " + userInput)

    return userInput


#check if the user input the correct number of arguments
if len(sys.argv) != 2:
    print "Invalid number of arguments"
    sys.exit()

#start up the sever
server = StartUp()

#loops forever until manual exit with Ctrl + c
while 1:

    #print a message to let the user know the server
    #is waiting for a connection from a client
    print "Waiting for client"

    #boolean to control if it's the first message or not
    #to display the chat header
    header = True

    #waits to accept incoming client connection
    connection, addr = server.accept()

    #loops until either server or client inputs "\quit"
    while 1:
        #receive message from the client
        receiveMSG = ReceiveMessage(connection)

        #checks if \quit was in message
        if "\quit" in receiveMSG: break

        #display header then set to false
        if header:
            print "Chat"
            #header no longer needed
            header = False

        #displys the recieved message
        print receiveMSG

        #gets the server's message
        sendMSG = SendMessage(connection)

        #checks if \quit was in message
        if "\quit" in sendMSG: break

    connection.close()
