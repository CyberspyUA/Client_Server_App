#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>

/*
* Встановлення зв'язку з бібліотекою Winsock
* (Linking with the Winsock library)
*/
#pragma comment(lib, "ws2_32.lib")

const int defaultPort = 27015; // Порт для зв'язку за замовчуванням (Default port for communication)
const int defaultBufferLength = 512; // Розмір буфера за замовчуванням для надсилання/приймання даних (Default buffer size for sending/receiving data)

int main()
{
    WSADATA wsaData; //  Структура для зберігання інформації про реалізацію Windows Sockets (Structure to hold information about the Windows Sockets implementation)
    SOCKET listenSocket = INVALID_SOCKET; // Сокет для прослуховування вхідних з'єднань (Socket for listening for incoming connections)
    SOCKET clientSocket = INVALID_SOCKET; // Сокет для зв'язку з клієнтом (Socket for communicating with the client)
    struct sockaddr_in server, client; // Структури для зберігання адрес сервера та клієнта (Structures to hold server and client addresses)
    int addressLength = sizeof(struct sockaddr_in); //  Довжина адресної структури (Length of the address structure)
    int result; // Змінна для зберігання результатів виклику функції (Variable to hold function call results)
    char receivingBuffer[defaultBufferLength]; // Буфер для отримання даних від клієнта (Buffer for receiving data from the client)
    int receivingBufferLength = defaultBufferLength; // Довжина буфера отриманих даних (Length of the received data buffer)

    // Ініціалізуємо WinSock (Initializing Winsock)
    result = WSAStartup(MAKEWORD(2, 2), &wsaData); // Запускаємо реалізацію Windows Sockets (Starting up the Windows Sockets implementation)
    if (result != 0) 
    {
        std::cerr << "WSAStartup failed with error: " << result << std::endl; // Print error message if startup fails
        return 1;
    }

    // Створюємо SOCKET для прослуховування вхідних з'єднань (Creating a SOCKET for listening for incoming connections)
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Створюємо TCP сокет (Create a TCP socket)
    if (listenSocket == INVALID_SOCKET) 
    {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl; // Вивести повідомлення про помилку, якщо не вдається створити сокет (Print error message if socket creation fails)
        WSACleanup(); // Очищення ресурсів Winsock (Clean up Winsock resources)
        return 1;
    }

    // Setup the TCP listening socket
    server.sin_family = AF_INET; // Set address family to IPv4
    server.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP address
    server.sin_port = htons(defaultPort); // Set the port number

    result = bind(listenSocket, (struct sockaddr*)&server, sizeof(server)); // Bind the socket to the server address
    if (result == SOCKET_ERROR) 
    {
        std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl; // Print error message if binding fails
        closesocket(listenSocket); // Close the listening socket
        WSACleanup(); // Clean up Winsock resources
        return 1;
    }

    result = listen(listenSocket, SOMAXCONN); // Listen for incoming connections
    if (result == SOCKET_ERROR) 
    {
        std::cerr << "listen failed with error: " << WSAGetLastError() << std::endl; // Print error message if listening fails
        closesocket(listenSocket); // Close the listening socket
        WSACleanup(); // Clean up Winsock resources
        return 1;
    }

    std::cout << "Server is waiting for incoming connections..." << std::endl;

    // Accept a client socket
    clientSocket = accept(listenSocket, (struct sockaddr*)&client, &addressLength); // Accept a connection from a client
    if (clientSocket == INVALID_SOCKET) 
    {
        std::cerr << "accept failed with error: " << WSAGetLastError() << std::endl; // Print error message if accepting fails
        closesocket(listenSocket); // Close the listening socket
        WSACleanup(); // Clean up Winsock resources
        return 1;
    }

    // Receive file name from client
    result = recv(clientSocket, receivingBuffer, receivingBufferLength, 0); // Receive data from the client
    if (result > 0) {
        std::string filename(receivingBuffer, result); // Extract the filename from the received data

        // Open the file to save received data
        std::ofstream file(filename, std::ios::binary); // Open file in binary mode for writing
        if (!file.is_open()) {
            std::cerr << "Unable to open file for writing." << std::endl; // Print error message if file opening fails
            closesocket(clientSocket); // Close the client socket
            closesocket(listenSocket); // Close the listening socket
            WSACleanup(); // Clean up Winsock resources
            return 1;
        }

        // Receive file data from client and save it to the file
        while ((result = recv(clientSocket, receivingBuffer, receivingBufferLength, 0)) > 0) {
            file.write(receivingBuffer, result); // Write received data to the file
        }

        std::cout << "File received successfully: " << filename << std::endl; // Print success message
        file.close(); // Close the file
    }
	else if (result == 0) 
    {
        std::cout << "Connection closing..." << std::endl; // Print message if connection is closing
    }
	else 
    {
        std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl; // Print error message if receiving fails
        closesocket(clientSocket); // Close the client socket
        closesocket(listenSocket); // Close the listening socket
        WSACleanup(); // Clean up Winsock resources
        return 1;
    }

    // Shutdown the connection since we're done
    result = shutdown(clientSocket, SD_SEND); // Shutdown the sending side of the connection
    if (result == SOCKET_ERROR) 
    {
        std::cerr << "shutdown failed with error: " << WSAGetLastError() << std::endl; // Print error message if shutdown fails
        closesocket(clientSocket); // Close the client socket
        closesocket(listenSocket); // Close the listening socket
        WSACleanup(); // Clean up Winsock resources
        return 1;
    }

    // Cleanup
    closesocket(clientSocket); // Close the client socket
    closesocket(listenSocket); // Close the listening socket
    WSACleanup(); // Clean up Winsock resources

    return 0; // Return success
}
