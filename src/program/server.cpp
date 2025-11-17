/** 
 * @file server.cpp
 * @author Martin Zůbek
 * @date 2025-10-25
 * @brief Implementation file for Server class. There are implemented methods for server-side operations.
 */

#include <iostream>
#include <string>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <variant>
#include <cstdint>
#include <chrono>
#include "../../inc/program/server.hpp"
#include "../../inc/cipher/decryptor.hpp"
#include "../../inc/file_handle/file_writer.hpp"
#include "../../inc/program/abstract_program.hpp"
#include "../../inc/types/protocol_definitions.hpp"
#include "../../inc/network/server_socket_handler.hpp"
#include "../../inc/protocol_message/message_parser.hpp"
#include "../../inc/protocol_message/message_creator.hpp"

// Constructor of the Server class
Server::Server(){
    this->state = SERVER_STATE::LISTENING_STATE;
    this->decryptor = std::make_unique<Decryptor>();
    this->fileWriter = std::make_unique<FileWriter>();
    this->messageCreator = std::make_unique<MessageCreator>(0, std::variant<struct sockaddr_in, struct sockaddr_in6>{}, std::variant<struct sockaddr_in, struct sockaddr_in6>{});
    this->messageParser = std::make_unique<MessageParser>();
    this->serverSocketHandler = std::make_unique<ServerSocketHandler>();
    this->actualSequenceNumber = 0;
    this->actualIdentifier = 0;
}

void Server::run(){

    // Main loop for handling server states
    while (this->state != SERVER_STATE::ENDING_STATE){
        switch (this->state){

            case SERVER_STATE::LISTENING_STATE:{ // Waiting for new client connections 
                this->handleListeningState();
                break;
            }
            case SERVER_STATE::INITIALIZING_STATE:{ // Initializing the informations about the client and preparing to receive data 
                this->handleInitializingState();
                break;
            }
            case SERVER_STATE::RECEIVING_STATE:{ // Receiving data from the client, decrypting it and writing to file
                this->handleReceivingState();
                break;
            }
            case SERVER_STATE::TERMINATE_SESSION_STATE:{ // Terminating the session with the non-active client
                this->handleTerminateSession();
                break;
            }
            default:{ // Internal error in the server logic
                throw std::runtime_error("Error: Internal error in the server logic!");

            }
        }
    }

    // Handle ending state and cleanup
    this->handleEndingState();
    this->serverSocketHandler->closeDescriptors();
    this->fileWriter->closeStream();
}


ssize_t Server::sendMessageToClient(MESSAGE_TYPES msgType){
    // Verify if message type
    if(msgType != MESSAGE_TYPES::HELLO_ACK &&
       msgType != MESSAGE_TYPES::DATA_ACK &&
       msgType != MESSAGE_TYPES::FIN_ACK &&
       msgType != MESSAGE_TYPES::FIN_ERR &&
       msgType != MESSAGE_TYPES::FIN){
        throw std::runtime_error("Error: Invalid message type for sending to client!");
    }

    // Size of the created message
    size_t messageSize = 0;
    // Create message to the client without data
    unsigned char *messageToClient = nullptr;
    if(msgType == MESSAGE_TYPES::FIN || msgType == MESSAGE_TYPES::FIN_ERR){
        messageToClient = this->messageCreator->createMessageWithNoData(this->actualIdentifier, this->actualSequenceNumber, msgType, messageSize, ICMP_ECHO, ICMP6_ECHO_REQUEST);
    } else {
        messageToClient = this->messageCreator->createMessageWithNoData(this->actualIdentifier, this->actualSequenceNumber, msgType, messageSize, ICMP_ECHOREPLY, ICMP6_ECHO_REPLY);
    }
    

    // Send the message to the client
    ssize_t sendedBytes = -1;
    if(this->clientIpvType == AF_INET){
        struct sockaddr_in* addr = &std::get<struct sockaddr_in>(this->clientAddress);
        sendedBytes = sendto(this->clientSocketDescriptor, messageToClient, messageSize, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
    } else {
        struct sockaddr_in6* addr = &std::get<struct sockaddr_in6>(this->clientAddress);
        sendedBytes = sendto(this->clientSocketDescriptor, messageToClient, messageSize, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in6));
    }
    
    // Free allocated memory
    delete[] messageToClient;
    // Return the number of sent bytes
    return sendedBytes;
}


void Server::handleInitializingState(){

    MESSAGE_TYPES recivedMessageType = MESSAGE_TYPES::BAD_TYPE;
    // Prepare buffer for receiving data
    unsigned char reciveBuffer[MAX_BUFFER_SIZE];

    // Receive message from the client
    struct sockaddr_storage clientAddress;
    socklen_t addrLen = sizeof(clientAddress);
    ssize_t recivedBytes = recvfrom(this->clientSocketDescriptor, reciveBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&clientAddress, &addrLen);

    // Verify if recvfrom was not failed
    if(recivedBytes == -1){
        this->serverSocketHandler->closeDescriptors();
        this->fileWriter->closeStream();
        throw std::runtime_error("Error: Recvfrom failed!");
    }
    
    // Copy received data to packet with exact size
    unsigned char packet[MAX_BUFFER_SIZE];
    memcpy(packet, reciveBuffer, recivedBytes);

    // Variable for store server address
    std::variant<struct sockaddr_in, struct sockaddr_in6> serverAddress;

    // Store Ipv4 or IPv6 addresses and identifier from the received message
    if(this->clientIpvType == AF_INET && recivedBytes >= ICMP_HEADER_SIZE + IPV4_HEADER_SIZE){
        struct iphdr *ipHeader = (struct iphdr *)packet;
        
        struct sockaddr_in clientAddressIpv4;
        memset(&clientAddressIpv4, 0, sizeof(clientAddressIpv4));
        clientAddressIpv4.sin_family = AF_INET;
        clientAddressIpv4.sin_addr.s_addr = ipHeader->saddr;
        this->clientAddress = clientAddressIpv4;
        
        struct sockaddr_in serverAddressIpv4;
        memset(&serverAddressIpv4, 0, sizeof(serverAddressIpv4));
        serverAddressIpv4.sin_family = AF_INET;
        serverAddressIpv4.sin_addr.s_addr = ipHeader->daddr;
        serverAddress = serverAddressIpv4;
        
        struct icmphdr *icmpHeader = (struct icmphdr *)(packet + IPV4_HEADER_SIZE);
        this->actualIdentifier = ntohs(icmpHeader->un.echo.id);
    }
    else if(this->clientIpvType == AF_INET6 && recivedBytes >= ICMP_HEADER_SIZE){
        struct sockaddr_in6 clientAddressIpv6;
        memcpy(&clientAddressIpv6, &clientAddress, sizeof(struct sockaddr_in6));
        this->clientAddress = clientAddressIpv6;
        
        struct sockaddr_in6 serverAddressIpv6;
        memset(&serverAddressIpv6, 0, sizeof(serverAddressIpv6));
        serverAddressIpv6.sin6_family = AF_INET6;
        socklen_t lenOfAddress = sizeof(serverAddressIpv6);
        if(getsockname(this->clientSocketDescriptor, (struct sockaddr*)&serverAddressIpv6, &lenOfAddress) == 0){
            serverAddress = serverAddressIpv6;
        } else {
            this->serverSocketHandler->closeDescriptors();
            this->fileWriter->closeStream();
            throw std::runtime_error("Error: Server cannot get own address!");
        }
        
        struct icmp6_hdr *icmp6Header = (struct icmp6_hdr *)(packet);
        this->actualIdentifier = ntohs(icmp6Header->icmp6_id);
    }
    // Not message of the protocol -> reset state to listening state
    else{
        this->actualSequenceNumber = 0;
        this->actualIdentifier = 0;
        this->state = SERVER_STATE::LISTENING_STATE;
        return;
    }

    // Set addresses and type in message creator
    this->messageCreator->setDestAddress(this->clientAddress);
    this->messageCreator->setSrcAddress(serverAddress);
    this->messageCreator->setType(this->clientIpvType);
    // Set addresses and type in message parser
    this->messageParser->setDestAddress(serverAddress);
    this->messageParser->setSrcAddress(this->clientAddress);
    this->messageParser->setType(this->clientIpvType);

    // Parse the received message and file name from HELLO message
    CHIPHER_T recivedFileName;
    recivedMessageType = this->messageParser->parseMessage(packet, recivedBytes, this->actualIdentifier, this->actualSequenceNumber, recivedFileName);
    // Not HELLO message -> reset state to listening state
    if (recivedMessageType != MESSAGE_TYPES::HELLO){
        this->actualSequenceNumber = 0;
        this->actualIdentifier = 0;
        this->state = SERVER_STATE::LISTENING_STATE;
        return;
    }

    // Decrypt the received file name
    std::string fileName = this->decryptor->decrypt(recivedFileName);

    // Open the file for writing
    this->fileWriter->setFileName(fileName);
    this->fileWriter->openStream();

    // Send HELLO_ACK message to the client
    ssize_t sendedBytes = this->sendMessageToClient(MESSAGE_TYPES::HELLO_ACK);
    // Increment actual sequence number
    this->actualSequenceNumber++;

    // Verify if sending was not failed
    if(sendedBytes == -1){
        this->serverSocketHandler->closeDescriptors();
        this->fileWriter->closeStream();
        throw std::runtime_error("Error: Sendto failed!");
    }
    
    // Move to receiving state with the new client
    this->state = SERVER_STATE::RECEIVING_STATE;
}


void Server::handleListeningState(){

    // Get all poll file descriptors from server socket handler
    struct pollfd *fdEvents = this->serverSocketHandler->getAllPollFileDescriptors();
    // Get count of poll file descriptors
    int countOfDescriptors = this->serverSocketHandler->getTotalPollFileDescriptors();
    
    while(true){ 
        // Poll for events on the file descriptors
        int pollState = poll(fdEvents,countOfDescriptors,-1);
        // Poll failed
        if (pollState == -1) {
            this->serverSocketHandler->closeDescriptors();
            this->fileWriter->closeStream();
            throw std::runtime_error("Error: Poll failed!");
        }
        // Event on IPv4 socket descriptor
        if(fdEvents[IPV4_DESCRIPTOR].revents & POLLIN){
            this->clientIpvType = AF_INET;
            this->clientSocketDescriptor = fdEvents[IPV4_DESCRIPTOR].fd;
            this->state = SERVER_STATE::INITIALIZING_STATE;
            return;
        }
        // Event on IPv6 socket descriptors
        for(int descriptorIndex = DEFAULT_SIZE_OF_POLL_FDS; descriptorIndex < countOfDescriptors; descriptorIndex++){
            if(fdEvents[descriptorIndex].revents & POLLIN){
                this->clientIpvType = AF_INET6;
                this->clientSocketDescriptor = fdEvents[descriptorIndex].fd;
                this->state = SERVER_STATE::INITIALIZING_STATE;
                return;
            }
        }
        // Event on SIGINT signal descriptor -> terminate the server
        if(fdEvents[SIGINT_DESCRIPTOR].revents & POLLIN){
            this->state = SERVER_STATE::ENDING_STATE;
            return;
        }
    }
}


void Server::handleReceivingState(){

    // Setup poll with server socket descriptor and SIGINT descriptor
    struct pollfd descriptorsEvent[DEFAULT_SIZE_OF_POLL_FDS];
    memset(descriptorsEvent, 0, sizeof(descriptorsEvent));
    descriptorsEvent[0].fd = this->clientSocketDescriptor;
    descriptorsEvent[0].events = POLLIN;

    descriptorsEvent[1].fd = this->serverSocketHandler->getAllPollFileDescriptors()[SIGINT_DESCRIPTOR].fd;
    descriptorsEvent[1].events = POLLIN;
    descriptorsEvent[1].revents = 0;

    // Get start time for timeout calculation
    std::chrono::steady_clock::time_point startWait = std::chrono::steady_clock::now();
    // Set timeout for maximum not received time
    int timeout = MAX_NOT_ACTIVE_TIME;

    // Wait for events on the descriptors
    while(true){
       int pollState = poll(descriptorsEvent, DEFAULT_SIZE_OF_POLL_FDS, timeout);
        // Poll failed
        if (pollState == -1) {
            this->serverSocketHandler->closeDescriptors();
            this->fileWriter->closeStream();
            throw std::runtime_error("Error: Poll failed!");
        }
        // Poll timeout -> terminate the session with the client
        if(pollState == 0){
            this->state = SERVER_STATE::TERMINATE_SESSION_STATE;
            return;
        }
        // Event on client socket descriptor
        if(descriptorsEvent[0].revents & POLLIN){
            // Prepare buffer for receiving data
            unsigned char reciveBuffer[MAX_BUFFER_SIZE];
            // Receive data from the client
            struct sockaddr_storage clientAddress;
            socklen_t lenOfAddress = sizeof(clientAddress);
            ssize_t recivedBytes = recvfrom(this->clientSocketDescriptor, reciveBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&clientAddress, &lenOfAddress);
            // Get current time for timeout calculation if this message is not message of the protocol
            std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

            // Verify if recvfrom was not failed
            if(recivedBytes == -1){
                this->serverSocketHandler->closeDescriptors();
                this->fileWriter->closeStream();
                return;
            }
            
            // Copy received data to packet with exact size
            unsigned char packet[MAX_BUFFER_SIZE];
            memcpy(packet, reciveBuffer, recivedBytes);

            // Parse the received message
            MESSAGE_TYPES recivedMessageType;
            CHIPHER_T recivedData;
            recivedMessageType = this->messageParser->parseMessage(packet, recivedBytes, this->actualIdentifier, this->actualSequenceNumber, recivedData);
          
            if(recivedMessageType == MESSAGE_TYPES::DATA){
                // Decrypt the received data
                std::string decryptedData = this->decryptor->decrypt(recivedData);
                // Write the decrypted data to the file
                try{
                    this->fileWriter->write(decryptedData);
                }catch(const std::runtime_error& writterException){
                    this->sendMessageToClient(MESSAGE_TYPES::FIN_ERR);
                    this->serverSocketHandler->closeDescriptors();
                    this->fileWriter->closeStream();
                    throw std::runtime_error(writterException.what());
                }
                // Send acknowledgment for received data
                ssize_t sendedBytes = this->sendMessageToClient(MESSAGE_TYPES::DATA_ACK);
                
                // Verify if sending was not failed
                if(sendedBytes == -1){
                    this->serverSocketHandler->closeDescriptors();
                    this->fileWriter->closeStream();
                    throw std::runtime_error("Error: Sendto failed!");
                }

                // Update timeout and sequence number for next messages
                startWait = std::chrono::steady_clock::now();
                timeout = MAX_NOT_ACTIVE_TIME;
                this->actualSequenceNumber++;
            }
            // Received FIN message -> close the file and go to listening state
            else if(recivedMessageType == MESSAGE_TYPES::FIN){
                this->fileWriter->closeStream();
                this->sendMessageToClient(MESSAGE_TYPES::FIN_ACK);
                this->state = SERVER_STATE::LISTENING_STATE;
                return;
            // Received other message -> ignore it and reset timout for rest of waiting time
            }else{
                int elapsed = (int)(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startWait).count());
                timeout = MAX_NOT_ACTIVE_TIME - elapsed;
            }
        }

        // Event on SIGINT signal descriptor -> terminate the server
        if(descriptorsEvent[SIGINT_DESCRIPTOR].revents & POLLIN){
            this->state = SERVER_STATE::ENDING_STATE;
            return;
        }
    }
    
}

void Server::handleTerminateSession(){
    // Setup poll with server socket descriptor and SIGINT descriptor
    struct pollfd fdEvent[DEFAULT_SIZE_OF_POLL_FDS];
    memset(fdEvent, 0, sizeof(fdEvent));
    fdEvent[0].fd = this->clientSocketDescriptor;
    fdEvent[0].events = POLLIN;
    
    fdEvent[1].fd = this->serverSocketHandler->getAllPollFileDescriptors()[SIGINT_DESCRIPTOR].fd;
    fdEvent[1].events = POLLIN;
    fdEvent[1].revents = 0;
    // Flag for FIN_ACK received
    bool finAckReceived = false;
    
    // Try to send FIN message with retries
    for(size_t retry = 0; retry < MAX_RETREIES && !finAckReceived; retry++){
        // Send FIN message to the client
        ssize_t sendedBytes = this->sendMessageToClient(MESSAGE_TYPES::FIN);
        // Verify if sending was not failed
        if(sendedBytes == -1){
            this->serverSocketHandler->closeDescriptors();
            this->fileWriter->closeStream();
            throw std::runtime_error("Error: Sendto failed!");
        }
        // Set timeout for response
        int timeout = RESPONSE_TIMEOUT;
        
        // Wait for FIN_ACK
        while(true){
            int pollState = poll(fdEvent, DEFAULT_SIZE_OF_POLL_FDS, timeout);
            // Poll failed
            if(pollState == -1){
                this->sendMessageToClient(MESSAGE_TYPES::FIN_ERR);
                this->serverSocketHandler->closeDescriptors();
                this->fileWriter->closeStream();
                throw std::runtime_error("Error: Sendto failed!");
            }
            // Poll timeout -> break to resend FIN message
            else if(pollState == 0){
                break;
            }
            // Event on client socket descriptor
            else if(fdEvent[0].revents & POLLIN){
                // Prepare buffer for receiving data
                unsigned char reciveBuffer[MAX_BUFFER_SIZE];
                struct sockaddr_storage srcAddr;
                socklen_t addrLen = sizeof(srcAddr);
                ssize_t recivedBytes = recvfrom(this->clientSocketDescriptor, reciveBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&srcAddr, &addrLen);

                // Verify if recvfrom was not failed
                if(recivedBytes == -1){
                    this->serverSocketHandler->closeDescriptors();
                    this->fileWriter->closeStream();
                    throw std::runtime_error("Error: Recvfrom failed!");
                }
                
                // Copy received data to packet with exact size
                unsigned char packet[MAX_BUFFER_SIZE];
                memcpy(packet, reciveBuffer, recivedBytes);
                
                // Parse the received message
                MESSAGE_TYPES receivedMessageType;
                CHIPHER_T receivedData;
                receivedMessageType = this->messageParser->parseMessage(packet, recivedBytes, this->actualIdentifier, this->actualSequenceNumber, receivedData);
               
                // FIN_ACK received -> set flag and break
                if(receivedMessageType == MESSAGE_TYPES::FIN_ACK){
                    finAckReceived = true;
                    break;
                }
            }
            // Event on SIGINT signal descriptor -> terminate the server
            else if(fdEvent[SIGINT_DESCRIPTOR].revents & POLLIN){
                this->state = SERVER_STATE::ENDING_STATE;
                return;
            }
        }
    }
    
    this->actualSequenceNumber = 0;
    this->actualIdentifier = 0;
    this->fileWriter->closeStream();
    this->state = SERVER_STATE::LISTENING_STATE;
}

void Server::handleEndingState(){
    // Send FIN_ERR to the client if connected
    if(this->state != SERVER_STATE::LISTENING_STATE){
        this->sendMessageToClient(MESSAGE_TYPES::FIN_ERR);
    }
    this->actualSequenceNumber = 0;
    this->actualIdentifier = 0;
}

