#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <WS2tcpip.h>
/*
* ������������ ��'���� � ��������� Winsock
* (Linking with the Winsock library)
*/
#pragma comment(lib, "ws2_32.lib")

const int defaultPort = 27015; // ���� ��� ��'���� �� ������������� (Default port for communication)
const int defaultBufferLength = 512; // ����� ������ �� ������������� ��� ����������/��������� ����� (Default buffer size for sending/receiving data)

int main(int argc, char* argv[]) {
    /*
    * ����������, �� ������� ��������� ������� ��������� ���������� �����
    * (Check if correct number of command line arguments are provided)
    */
    if (argc != 3) 
    { 
        /*
        * �������� ���������� ��� �������� ��������� ��� ����
        * (Print instructions on the required parameters to be entered)
        */
        std::cerr << "Usage: " << argv[0] << " <server_ip> <file_to_send>" << std::endl; 
        return 1;
    }
    /*
    * ��������� ��� ��������� ���������� ��� ��������� Windows Sockets
    * (Structure to hold information about the Windows Sockets implementation)
    */
    WSADATA wsaData; 
    SOCKET ConnectSocket = INVALID_SOCKET; //����� ��� ���������� �� ������� (Socket for connecting to server)
    struct sockaddr_in server; // ��������� ��� ��������� ������ ������� (Structure to hold server address)
    int result; // ����� ��� ��������� ���������� ������� ������� (Variable to hold function call results)

    // ���������� WinSock (Initialize Winsock)
    result = WSAStartup(MAKEWORD(2, 2), &wsaData); // ������ ��������� ������ Windows (Start up the Windows Sockets implementation)
    if (result != 0) 
    {
        std::cerr << "WSAStartup failed with error: " << result << std::endl; //�������� �������, ���� ������ ���������� (Print error message if startup fails)
        return 1; // ��������� ������� (Return error)
    }

    /*
    * ��������� SOCKET ��� ���������� �� �������
    * (Creating a SOCKET for connecting to server)
    */
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // ��������� TCP ����� (Create a TCP socket)
    if (ConnectSocket == INVALID_SOCKET) 
    {
        /*
        * �������� �������, ���� ��������� �������� �����
        * (Print error message if socket creation fails)
        */
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl; 
        WSACleanup(); // ������� Winsock ������� (Cleaning up Winsock resources)
        return 1; // ��������� ������� (Return error)
    }

    /*
    * ����������� ��������� ������ �������
    * (Setup the server address structure)
    */ 
    server.sin_family = AF_INET; // ��������� �������� ������ �� IPv4 (Set address family to IPv4)
    // ��������� IP ������ ������� (Set the server IP address)
    if (inet_pton(AF_INET, argv[1], &server.sin_addr) != 1) 
    {
	    std::cerr << "Invalid address supplied" << std::endl;
	    closesocket(ConnectSocket);
	    WSACleanup();
	    return 1;
	}

    server.sin_port = htons(defaultPort); // ��������� ����� �����(Set the port number)

    result = connect(ConnectSocket, (struct sockaddr*)&server, sizeof(server)); // ϳ�'�������� �� ������� (Connecting to the server)
    if (result == SOCKET_ERROR) 
    {
        std::cerr << "connect failed with error: " << WSAGetLastError() << std::endl; // �������� �������, ���� ��������� ���������� �'������� (Print error message if connection fails)
        closesocket(ConnectSocket); // ��������� �볺������� ����� (Close the client socket)
        WSACleanup(); // ������� Winsock ������� (Cleaning up Winsock resources)
        return 1; // ��������� ������� (Return error)
    }

    /**
     * ³���������� ��'� ����� �� ������
     * (Sending file name to server)
     */
    result = send(ConnectSocket, argv[2], strlen(argv[2]), 0); // ³���������� ��'� ����� �� ������ (Sending file name to server)
    if (result == SOCKET_ERROR) 
    {
        std::cerr << "send failed with error: " << WSAGetLastError() << std::endl; // �������� �������, ���� ����������� ���� �������� (Print error message if sending fails)
        closesocket(ConnectSocket); // ��������� �볺������� ����� (Close the client socket)
        WSACleanup(); // ������� Winsock ������� (Cleaning up Winsock resources)
        return 1; // ��������� ������� (Return error)
    }

    /**
     * ³�������� ����, ���� ���� ����������
     * (Opening the file to send)
     */
    std::ifstream file(argv[2], std::ios::binary); // ³�������� ����� � �������� ������ ��� ���������� (Opening a file in binary mode for reading)
    if (!file.is_open()) 
    {
        std::cerr << "Unable to open file for reading." << std::endl; // �������� �������, ���� �������� ����� ���� �������� (Print error message if file opening fails)
        closesocket(ConnectSocket); // ��������� �볺������� ����� (Close the client socket)
        WSACleanup(); // ������� Winsock ������� (Cleaning up Winsock resources)
        return 1; // ��������� ������� (Return error)
    }

    char dataToSendBuffer[defaultBufferLength]; // ����� ��� ��������� ����� ��� �������� (Buffer for storing data to send)
    int bytesRead; // ����� ��� ��������� ������� ����, ���������� � ����� (Variable to hold number of bytes read from file)

    /**
     * ³���������� ��� ����� �� ������
     * (Sending file data to server)
     */
    while (!file.eof()) 
    {
        file.read(dataToSendBuffer, defaultBufferLength); // ������� ��� � ����� � ����� (Read data from the file into the buffer)
        bytesRead = file.gcount(); // �������� ������� ���������� ���� (Get the number of bytes read)
        result = send(ConnectSocket, dataToSendBuffer, bytesRead, 0); // �������� ��� �� ������ (Send the data to the server)
        if (result == SOCKET_ERROR) 
        {
            std::cerr << "send failed with error: " << WSAGetLastError() << std::endl; //�������� �������, ���� ����������� ���� ������� (Print error message if sending fails)
            file.close(); // ��������� ���� (Closing the file)
            closesocket(ConnectSocket); // ��������� �볺������� ����� (Closing the client socket)
            WSACleanup(); // ������� Winsock ������� (Cleaning up Winsock resources)
            return 1; // ��������� ������� (Return error)
        }
    }

    std::cout << "File sent successfully: " << argv[2] << std::endl; // ������� ����������� ��� ���� (Print success message

    /**
     * ��������� �������
     * (Cleaning up)
     */
    file.close(); // ��������� �볺������� ����� (Close the client socket)
    closesocket(ConnectSocket); // ��������� �볺������� ����� (Closing the client socket)
    WSACleanup(); // ������� Winsock ������� (Cleaning up Winsock resources)

    return 0; //�������� ���� ��������� ������ (The program was completed successfully)
}
