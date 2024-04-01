#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>

/*
* ������������ ��'���� � ��������� Winsock
* (Linking with the Winsock library)
*/
#pragma comment(lib, "ws2_32.lib")

const int defaultPort = 27015; // ���� ��� ��'���� �� ������������� (Default port for communication)
const int defaultBufferLength = 512; // ����� ������ �� ������������� ��� ����������/��������� ����� (Default buffer size for sending/receiving data)

int main()
{
    WSADATA wsaData; //  ��������� ��� ��������� ���������� ��� ��������� Windows Sockets (Structure to hold information about the Windows Sockets implementation)
    SOCKET listenSocket = INVALID_SOCKET; // ����� ��� ��������������� ������� �'������ (Socket for listening for incoming connections)
    SOCKET clientSocket = INVALID_SOCKET; // ����� ��� ��'���� � �볺���� (Socket for communicating with the client)
    struct sockaddr_in server, client; // ��������� ��� ��������� ����� ������� �� �볺��� (Structures to hold server and client addresses)
    int addressLength = sizeof(struct sockaddr_in); //  ������� ������� ��������� (Length of the address structure)
    int result; // ����� ��� ��������� ���������� ������� ������� (Variable to hold function call results)
    char receivingBuffer[defaultBufferLength]; // ����� ��� ��������� ����� �� �볺��� (Buffer for receiving data from the client)
    int receivingBufferLength = defaultBufferLength; // ������� ������ ��������� ����� (Length of the received data buffer)

    // ���������� WinSock (Initializing Winsock)
    result = WSAStartup(MAKEWORD(2, 2), &wsaData); // ��������� ��������� Windows Sockets (Starting up the Windows Sockets implementation)
    if (result != 0) 
    {
        std::cerr << "WSAStartup failed with error: " << result << std::endl; // �������� �������, � ��� ��������� ������� (Print error message if startup fails)
        return 1;
    }

    // ��������� SOCKET ��� ��������������� ������� �'������ (Creating a SOCKET for listening for incoming connections)
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // ��������� TCP ����� (Create a TCP socket)
    if (listenSocket == INVALID_SOCKET) 
    {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl; // �������� �������, ���� �� ������� �������� ����� (Print error message if socket creation fails)
        WSACleanup(); // �������� ������� Winsock (Clean up Winsock resources)
        return 1;
    }

    // ����������� TCP-����� ��� ��������������� (Setting up the TCP listening socket)
    server.sin_family = AF_INET; // ��������� �������� ������ �� IPv4 (Setting address family to IPv4)
    server.sin_addr.s_addr = INADDR_ANY; // �������� �'������� � ����-��� IP-������ (Accepting connections from any IP address)
    server.sin_port = htons(defaultPort); // ����������� ����� ����� (Setting the port number)

    result = bind(listenSocket, (struct sockaddr*)&server, sizeof(server)); // ����'����� ����� �� ������ ������� (Bind the socket to the server address)
    if (result == SOCKET_ERROR) 
    {
        std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl; // �������� �������, ���� ��'�������� ���� �������� (Print error message if binding fails)
        closesocket(listenSocket); // ��������� ����� ��� ��������������� (Close the listening socket)
        WSACleanup(); // ������� Winsock ������� (Clean up Winsock resources)
        return 1;
    }

    result = listen(listenSocket, SOMAXCONN); // ������������ ����� �'������� (Listening for incoming connections)
    if (result == SOCKET_ERROR) 
    {
        std::cerr << "listen failed with error: " << WSAGetLastError() << std::endl; // �������� �������, � ��� ��������� ���������������(Printing the error message if listening fails)
        closesocket(listenSocket); // ��������� ����� ��� ��������������� (Close the listening socket)
        WSACleanup(); // ������� Winsock ������� (Clean up Winsock resources)
        return 1;
    }

    std::cout << "Server is waiting for incoming connections..." << std::endl;

    /**
     * �������� �볺������� �����
     * (Accepting a client socket)
     */
    clientSocket = accept(listenSocket, (struct sockaddr*)&client, &addressLength); // �������� �'������� �� �볺��� (Accept a connection from a client)
    if (clientSocket == INVALID_SOCKET) 
    {
        std::cerr << "accept failed with error: " << WSAGetLastError() << std::endl; // �������� �������, ���� ��������� �������� �'������� (Print error message if accepting fails)
        closesocket(listenSocket); // ��������� ����� ��� ��������������� (Close the listening socket)
        WSACleanup(); // ������� Winsock ������� (Clean up Winsock resources)
        return 1;
    }

    // �������� ��'� ����� �� �볺��� (Receiving file name from client)
    result = recv(clientSocket, receivingBuffer, receivingBufferLength, 0); // �������� ��� �� �볺��� (Receive data from the client)
    if (result > 0) 
    {
        std::string filename(receivingBuffer, result); // �������� ��'� ����� � ��������� ����� (Extract the filename from the received data)

        /**
         * ³�������� ���� ��� ��������� �����
         * Opening the file to save received data
         */
        std::ofstream file(filename, std::ios::binary); // ³�������� ���� � �������� ������ ��� ������ (Open file in binary mode for writing)
        if (!file.is_open()) 
        {
            std::cerr << "Unable to open file for writing." << std::endl; // �������� �������, � ��� ��������� �������� (Print error message if file opening fails)
            closesocket(clientSocket); // ��������� �볺������� ����� (Close the client socket)
            closesocket(listenSocket); // ��������� �������������� ����� (Close the listening socket)
            WSACleanup(); // ������� Winsock ������� (Clean up Winsock resources)
            return 1;
        }

        /**
         * �������� ��� � ����� �볺��� �� �������� �� � ����
         * Receiving file data from client and save it to the file
         */
        while ((result = recv(clientSocket, receivingBuffer, receivingBufferLength, 0)) > 0) 
        {
            file.write(receivingBuffer, result); // �������� ������� ��� � ���� (Write received data to the file)
        }

        std::cout << "File received successfully: " << filename << std::endl; // �������� ����������� ��� ���� (Print success message)
        file.close(); // ��������� ���� (Close the file)
    }
	else if (result == 0) 
    {
        std::cout << "Connection closing..." << std::endl; // �������� �����������, ���� ��'���� ��� ���������� (Print message if connection is closing)
    }
	else 
    {
        std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl; // �������� �������, � ��� ��������� ��������� ����� (Print error message if receiving data fails)
        closesocket(clientSocket); // ��������� �볺������� ����� (Close the client socket)
        closesocket(listenSocket); // ��������� ������������� ����� (Close the listening socket)
        WSACleanup(); // ������� Winsock ������� (Clean up Winsock resources)
        return 1;
    }

    /**
     * ��������� �'�������, ���� ����������
     * (Shutting down the connection since we're done)
     */
    result = shutdown(clientSocket, SD_SEND); // ³�'������ �볺������ ������� �� ������� (Shutdown the sending side of the connection)
    if (result == SOCKET_ERROR) 
    {
        std::cerr << "shutdown failed with error: " << WSAGetLastError() << std::endl; // �������� �������, � ��� ��������� ���������� (Print error message if shutdown fails)
        closesocket(clientSocket); // ��������� �볺������� ����� (Close the client socket)
        closesocket(listenSocket); // ��������� ������������� ����� (Close the listening socket)
        WSACleanup(); // ������� Winsock ������� (Clean up Winsock resources)
        return 1;
    }

    /**
     * ��������� �������
     * (Cleaning up)
     */
    closesocket(clientSocket); // ��������� �볺������� ����� (Close the client socket)
	closesocket(listenSocket); // ��������� ������������� ����� (Close the listening socket)
	WSACleanup(); // ������� Winsock ������� (Clean up Winsock resources)

    return 0; //�������� ���� ��������� ������ (The program was completed successfully)
}
