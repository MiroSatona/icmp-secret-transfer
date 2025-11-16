/** 
 * @file client.cpp
 * @author Martin Zůbek
 * @date 2025-10-25
 * @brief Implementation file for Client class. There are implemented methods for client-side operations.
 */

#include <iostream>
#include <memory>
#include <cstring>
#include <cstdint>
#include <string>
#include <unistd.h>
#include <random>
#include <poll.h>
#include <chrono>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include "../../inc/program/client.hpp"
#include "../../inc/cipher/encryptor.hpp"
#include "../../inc/cipher/decryptor.hpp"
#include "../../inc/types/struct_cipher.hpp"
#include "../../inc/file_handle/file_reader.hpp"
#include "../../inc/network/client_socket_handler.hpp"
#include "../../inc/protocol_message/message_creator.hpp"
#include "../../inc/protocol_message/message_parser.hpp"

// Constructor of Client class
Client::Client(std::variant<struct sockaddr_in, struct sockaddr_in6> serverAddress, std::string filePath, int type):AbstractProgram(){
    // Initialize encryptor
    this->encryptor = std::make_unique<Encryptor>();
    // Initialize client socket handler
    this->clientSocketHandler = std::make_unique<ClientSocketHandler>(type, serverAddress);
    // Initialize file reader
    try{
        this->fileReader = std::make_unique<FileReader>(filePath);
    }catch(const std::invalid_argument &exception){
        this->clientSocketHandler->closeDescriptors();
        throw exception;
    }
    // Initialize message creator
    this->messageCreator = std::make_unique<MessageCreator>(type, serverAddress, clientSocketHandler->getSourceAddress());
    // Initialize message parser
    this->messageParser = std::make_unique<MessageParser>();
    // Set initial state and other variables
    this->state = CLIENT_STATE::AUTH_STATE;
    this->messageParser->setDestAddress(clientSocketHandler->getSourceAddress());
    this->messageParser->setSrcAddress(serverAddress);
    this->messageParser->setType(type);
    this->serverAddress = serverAddress;
    this->sequenceNumber = 0;
    this->retryAuth = 1;

    /**
     * @note Initialization of the random identifier was inspired by common usage
     *       patterns of the C++11 <random> library.
     *
     *       Reference (conceptual inspiration):
     *       StackOverflow – “Generate random numbers using C++11 <random> library”
     *       https://stackoverflow.com/questions/19665818/generate-random-numbers-using-c11-random-library
     */
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::uniform_int_distribution<uint16_t> distribution(0, 0xFFFF);
    this->identifier = distribution(generator);
    

    
}

void Client::run(){

    try {
        while(this->state != CLIENT_STATE::FIN_STATE) {
            switch(this->state) {
                case CLIENT_STATE::AUTH_STATE:{ // Authentication state
                    std::cerr << "AUTH_STATE" << std::endl;
                    this->handleAuthState();
                    break;
                }
                case CLIENT_STATE::WAIT_STATE:{ // Wait state
                    std::cerr << "WAIT_STATE" << std::endl;
                    this->handleWaitState();
                    break;
                }
                case CLIENT_STATE::SENDING_STATE:{ // Sending state
                    std::cerr << "SENDING_STATE" << std::endl;
                    this->handleSendingState();
                    break;
                }
                case CLIENT_STATE::SERVER_ERROR_STATE:{ // Server error state
                    std::cerr << "SERVER_ERROR_STATE" << std::endl;
                    throw std::logic_error("Error: Server-side error during file transfer!");
                    break;
                }
                
                default:{ // Unknown state
                    std::cerr << "UNKNOWN_STATE" << std::endl;
                    throw std::runtime_error("Error: Internal error in the client logic!");
                }
            }
        }
        
        // Handle finish state
        this->handleFinState();
        this->clientSocketHandler->closeDescriptors();
        this->fileReader->closeStream();
        
    } catch(const std::exception& exception) {
        this->clientSocketHandler->closeDescriptors();
        this->fileReader->closeStream();
        throw;
    }
}

void Client::handleAuthState(){
    
    // Get file name to send
    std::string fileName = this->fileReader->getFileName();
    // Encrypt file name
    size_t fileNameLen = fileName.length();
    unsigned char* fileNameForEncrypt = new unsigned char[fileNameLen];
    memcpy(fileNameForEncrypt, fileName.c_str(), fileNameLen);
    CHIPHER_T encryptData = this->encryptor->encrypt(fileNameForEncrypt, fileNameLen);
    
    // Create HELLO message with encrypted file name
    size_t lengthOfData = 0;
    unsigned char* messageToSend = this->messageCreator->createMessageWithData(encryptData, this->identifier, this->sequenceNumber, MESSAGE_TYPES::HELLO, lengthOfData, ICMP_ECHO, ICMP6_ECHO_REQUEST);
    
    // Prepare for poll with client socket
    struct pollfd fds[1];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = this->clientSocketHandler->getSocketDescriptor();
    fds[0].events = POLLIN;  

    // Send HELLO message to server
    if(sendto(this->clientSocketHandler->getSocketDescriptor(), messageToSend, lengthOfData, 0, (struct sockaddr*)&this->serverAddress, sizeof(this->serverAddress)) == -1){
        delete[] fileNameForEncrypt;
        delete[] messageToSend;
        throw std::runtime_error("Error: Send to failed!");
    }

    // Get time of sending
    std::chrono::steady_clock::time_point sendTime = std::chrono::steady_clock::now();
    // Set timeout for response
    int timeout = RESPONSE_TIMEOUT;

    while(true){
        int pollState = poll(fds, 1, timeout);

        // Poll failed
        if (pollState == -1) {
            delete[] fileNameForEncrypt;
            delete[] messageToSend;
            throw std::runtime_error("Poll failed!");
        }
        // Timeout expired
        else if(pollState == 0){
            break;
        }
        // Event on socket
        else if(fds[0].revents & POLLIN){
            // Recive packet from server
            unsigned char packet[MAX_BUFFER_SIZE];
            size_t packetSize = 0;
            CHIPHER_T receivedData;
            MESSAGE_TYPES receivedMessageType;
            try{
                receivedMessageType = this->receiveMessageFromServer(packet, packetSize, receivedData);
            }catch(const std::runtime_error& reciveException){
                delete[] fileNameForEncrypt;
                delete[] messageToSend;
                throw reciveException;
            }
            // Get time of receive
            std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
            
            // Check if received message is HELLO_ACK, if yes move to SENDING_STATE
            if(receivedMessageType == MESSAGE_TYPES::HELLO_ACK){
                this->state = CLIENT_STATE::SENDING_STATE;
                this->sequenceNumber++;
                break;
            }
            
            // Calculate elapsed time and update timeout if client didnt recive HELLO_ACK message
            int elapsed = (int)(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - sendTime).count());
            timeout = RESPONSE_TIMEOUT - elapsed;
            // Check if the time has not expired
            if(timeout <= 0) break;
                
        }
    }

    // Clean up
    delete[] fileNameForEncrypt;
    delete[] messageToSend;
    
    // If state is not SENDING_STATE, move to WAIT_STATE
    if(this->state != CLIENT_STATE::SENDING_STATE){
        this->state = CLIENT_STATE::WAIT_STATE;
    }
}


void Client::handleWaitState(){
    
    // Check if maximum authentication retries reached
    if(this->retryAuth == MAX_AUTH_RETRIES){
        std::cerr << "Erorr: Client can not be authenticate!" << std::endl;
        throw std::runtime_error("Error: Client can not be authenticate!");
    }

    // Sleep for WAIT_TIMEOUT and retry authentication
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIMEOUT));
    this->retryAuth++;
    this->state = CLIENT_STATE::AUTH_STATE;
}

void Client::handleSendingState(){
    
    // Prepare for poll with client socket
    struct pollfd fileDescriptors[1];
    memset(fileDescriptors, 0, sizeof(fileDescriptors));
    fileDescriptors[0].fd = this->clientSocketHandler->getSocketDescriptor();
    fileDescriptors[0].events = POLLIN;
    
    // Data readed from file
    unsigned char *readedData = nullptr;
    // Length of readed data
    size_t dataLen = 0;  
    // Length of mess to send
    size_t lengthOfMess = 0;
    // Time of sending packet
    std::chrono::steady_clock::time_point sendTime;

    // Read the parts of the file untill eof will be reached
    while((readedData = this->fileReader->read(dataLen)) != nullptr){
        
        // Encrypt the readed data
        CHIPHER_T encryptData = this->encryptor->encrypt(readedData, dataLen);
        // Create DATA message with encrypted data
        unsigned char* messageToSend = this->messageCreator->createMessageWithData(encryptData, this->identifier, this->sequenceNumber, MESSAGE_TYPES::DATA, lengthOfMess, ICMP_ECHO, ICMP6_ECHO_REQUEST);
        // Flag indicating DATA_ACK
        bool getAnswer = false;

        // Sended for MAX_RETREIES
        for(size_t retry = 0; retry < MAX_RETREIES && !getAnswer; retry++){
            
            // Send data to server
            if(sendto(this->clientSocketHandler->getSocketDescriptor(), messageToSend, lengthOfMess, 0, (struct sockaddr*)&this->serverAddress, sizeof(this->serverAddress)) == -1){
                delete[] readedData;
                delete[] messageToSend;
                throw std::runtime_error("Error: Send to failed!");
            }

            // Get time of send
            sendTime = std::chrono::steady_clock::now();
            // Set timeout
            int timeout = RESPONSE_TIMEOUT;
            
            // Wait for response
            while(true){

                int pollState = poll(fileDescriptors, 1, timeout);
                // Poll failed
                if (pollState == -1) {
                    delete[] readedData;
                    delete[] messageToSend;
                    throw std::runtime_error("Poll failed!");
                }
                // Timeout expired
                else if(pollState == 0){
                    break;
                }
                // Recive a message
                else if(fileDescriptors[0].revents & POLLIN){
                    // Recive packet from server
                    unsigned char packet[MAX_BUFFER_SIZE];
                    size_t packetSize = 0;
                    CHIPHER_T receivedData;
                    MESSAGE_TYPES receivedMessageType;
                    try{
                        receivedMessageType = this->receiveMessageFromServer(packet, packetSize, receivedData);
                    }catch(const std::runtime_error& e){
                        delete[] readedData;
                        delete[] messageToSend;
                        throw e;
                    }
                    
                    // Save current time of the recive
                    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
                    
                    // Acknowledgment about server get the data
                    if(receivedMessageType == MESSAGE_TYPES::DATA_ACK){
                        getAnswer = true;
                        this->sequenceNumber++;
                        break;
                    }
                    // Server error received
                    else if(receivedMessageType == MESSAGE_TYPES::FIN_ERR){
                        delete[] readedData;
                        delete[] messageToSend;
                        this->state = CLIENT_STATE::SERVER_ERROR_STATE;
                        return;
                    }
                    
                    // If is not message of the protocol calculate elapsed time and set new timeout
                    int elapsed = (int)(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - sendTime).count());
                    timeout = RESPONSE_TIMEOUT - elapsed;
                    if(timeout <= 0)break;
                }
            }
        }
        
        // If we dont get answer -> than it is error of protocol
        if(!getAnswer) this->state = CLIENT_STATE::SERVER_ERROR_STATE;
        
        // Clean up
        delete[] readedData;
        delete[] messageToSend;
    }

    // File transfer was successful, move to FIN_STATE
    this->state = CLIENT_STATE::FIN_STATE;
}


void Client::handleFinState(){
    // Prepare FIN message
    size_t packetSize = 0;
    unsigned char* finPacket = this->messageCreator->createMessageWithNoData(
        this->identifier, 
        this->sequenceNumber, 
        MESSAGE_TYPES::FIN, 
        packetSize,
        ICMP_ECHO,
        ICMP6_ECHO_REQUEST
    );
    // Prepare poll with client socket
    struct pollfd fileDescriptors[1];
    memset(fileDescriptors, 0, sizeof(fileDescriptors));
    fileDescriptors[0].fd = this->clientSocketHandler->getSocketDescriptor();
    fileDescriptors[0].events = POLLIN;
    
    // Flag indicating FIN_ACK received
    bool finAckReceived = false;
    // Time of sending packet
    std::chrono::steady_clock::time_point sendTime;
    
    // Send FIN message and wait for FIN_ACK for MAX_RETREIES
    for(size_t retry = 0; retry < MAX_RETREIES && !finAckReceived; retry++){
        // Send FIN message to server
        if(sendto(this->clientSocketHandler->getSocketDescriptor(), finPacket, packetSize, 0, (struct sockaddr*)&this->serverAddress, sizeof(this->serverAddress)) == -1){
            delete[] finPacket;
            throw std::runtime_error("Error: Sendto failed!");
        }
        // Get time of sending
        sendTime = std::chrono::steady_clock::now();
        // Set timeout
        int timeout = RESPONSE_TIMEOUT;
        
        // Wait for FIN_ACK
        while(true){
            int pollState = poll(fileDescriptors, 1, timeout);

            // Poll failed
            if(pollState == -1){
                delete[] finPacket;
                throw std::runtime_error("Error: Poll failed!");
            }
            // Timeout expired
            else if(pollState == 0){
                break;
            }
            // Message received on socket
            else if(fileDescriptors[0].revents & POLLIN){
                // Receive packet from server and parse him
                unsigned char packet[MAX_BUFFER_SIZE];
                size_t packetSize = 0;
                CHIPHER_T receivedData;
                MESSAGE_TYPES receivedMessageType;
                try{
                    receivedMessageType = this->receiveMessageFromServer(packet, packetSize, receivedData);
                }catch(const std::runtime_error& finException){
                    delete[] finPacket;
                    throw finException;
                }
                // Get current time of receiving
                std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
                
                // Check if received message is FIN_ACK
                if(receivedMessageType == MESSAGE_TYPES::FIN_ACK){
                    finAckReceived = true;
                    break;
                }
                
                // Calculate elapsed time and update timeout if FIN_ACK not received
                int elapsed = (int)(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - sendTime).count());
                timeout = RESPONSE_TIMEOUT - elapsed;
                if(timeout <= 0)break;
            }
        }
    }
    
    // Clean up FIN packet
    delete[] finPacket;
    
    // If FIN_ACK not received -> protocol error
    if(!finAckReceived){
        throw std::logic_error("Error: Cannot finish connection with server!");
    }
}

MESSAGE_TYPES Client::receiveMessageFromServer(unsigned char* packet, size_t& packetSize, CHIPHER_T& receivedData){
    // Prepare buffer for receiving
    unsigned char receiveBuffer[MAX_BUFFER_SIZE];
    memset(receiveBuffer, 0, MAX_BUFFER_SIZE);
    
    // Source address storage
    struct sockaddr_storage sourceAddress;
    socklen_t addrLen = sizeof(sourceAddress);
    
    // Receive message from server
    ssize_t recivedByte = recvfrom(this->clientSocketHandler->getSocketDescriptor(), receiveBuffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&sourceAddress, &addrLen);
    
    // Check for recvfrom error
    if(recivedByte == -1){
        throw std::runtime_error("Error: Recvfrom failed!");
    }
    
    // Copy received data to packet with right size
    packetSize = recivedByte;
    memcpy(packet, receiveBuffer, recivedByte);
    
    // Parse the received message and return type of the message
    return this->messageParser->parseMessage(packet, recivedByte, this->identifier, this->sequenceNumber, receivedData);
}

