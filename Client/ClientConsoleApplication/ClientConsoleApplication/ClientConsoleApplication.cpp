#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <WS2tcpip.h>
/*
* Встановлення зв'язку з бібліотекою Winsock
* (Linking with the Winsock library)
*/
#pragma comment(lib, "ws2_32.lib")

const int defaultPort = 27015; // Порт для зв'язку за замовчуванням (Default port for communication)
const int defaultBufferLength = 512; // Розмір буфера за замовчуванням для надсилання/приймання даних (Default buffer size for sending/receiving data)

int main(int argc, char* argv[]) {
    /*
    * Перевіряємо, чи вказано правильну кількість аргументів командного рядка
    * (Check if correct number of command line arguments are provided)
    */
    if (argc != 3) 
    { 
        /*
        * Виводимо інструкцію про необхідні параметри для ввод
        * (Print instructions on the required parameters to be entered)
        */
        std::cerr << "Usage: " << argv[0] << " <server_ip> <file_to_send>" << std::endl; 
        return 1;
    }
    /*
    * Структура для зберігання інформації про реалізацію Windows Sockets
    * (Structure to hold information about the Windows Sockets implementation)
    */
    WSADATA wsaData; 
    SOCKET ConnectSocket = INVALID_SOCKET; //Сокет для підключення до сервера (Socket for connecting to server)
    struct sockaddr_in server; // Структура для зберігання адреси сервера (Structure to hold server address)
    int result; // Змінна для зберігання результатів виклику функції (Variable to hold function call results)

    // Ініціалізуємо WinSock (Initialize Winsock)
    result = WSAStartup(MAKEWORD(2, 2), &wsaData); // Запуск реалізації сокетів Windows (Start up the Windows Sockets implementation)
    if (result != 0) 
    {
        std::cerr << "WSAStartup failed with error: " << result << std::endl; //Виводимо помилку, якщо запуск провалився (Print error message if startup fails)
        return 1; // Повертаємо помилку (Return error)
    }

    /*
    * Створення SOCKET для підключення до сервера
    * (Creating a SOCKET for connecting to server)
    */
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Створюємо TCP сокет (Create a TCP socket)
    if (ConnectSocket == INVALID_SOCKET) 
    {
        /*
        * Виводимо помилку, якщо невдалося створити сокет
        * (Print error message if socket creation fails)
        */
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl; 
        WSACleanup(); // Очищуємо Winsock ресурси (Cleaning up Winsock resources)
        return 1; // Повертаємо помилку (Return error)
    }

    /*
    * Налаштовуємо структуру адреси сервера
    * (Setup the server address structure)
    */ 
    server.sin_family = AF_INET; // Визначаємо сімейство адреса як IPv4 (Set address family to IPv4)
    // Визначаємо IP адресу сервера (Set the server IP address)
    if (inet_pton(AF_INET, argv[1], &server.sin_addr) != 1) 
    {
	    std::cerr << "Invalid address supplied" << std::endl;
	    closesocket(ConnectSocket);
	    WSACleanup();
	    return 1;
	}

    server.sin_port = htons(defaultPort); // Визначаємо номер порта(Set the port number)

    result = connect(ConnectSocket, (struct sockaddr*)&server, sizeof(server)); // Під'єднуємось до сервера (Connecting to the server)
    if (result == SOCKET_ERROR) 
    {
        std::cerr << "connect failed with error: " << WSAGetLastError() << std::endl; // Виводимо помилку, якщо невдалося встановити з'єднання (Print error message if connection fails)
        closesocket(ConnectSocket); // Зачиняємо клієнтський сокет (Close the client socket)
        WSACleanup(); // Очищуємо Winsock ресурси (Cleaning up Winsock resources)
        return 1; // Повертаємо помилку (Return error)
    }

    /**
     * Відправляємо ім'я файлу на сервер
     * (Sending file name to server)
     */
    result = send(ConnectSocket, argv[2], strlen(argv[2]), 0); // Відправляємо ім'я файлу на сервер (Sending file name to server)
    if (result == SOCKET_ERROR) 
    {
        std::cerr << "send failed with error: " << WSAGetLastError() << std::endl; // Виводимо помилку, якщо відправлення було невдалим (Print error message if sending fails)
        closesocket(ConnectSocket); // Зачиняємо клієнтський сокет (Close the client socket)
        WSACleanup(); // Очищуємо Winsock ресурси (Cleaning up Winsock resources)
        return 1; // Повертаємо помилку (Return error)
    }

    /**
     * Відкриваємо файл, який буде відправлено
     * (Opening the file to send)
     */
    std::ifstream file(argv[2], std::ios::binary); // Відкриваємо файлу у бінарному форматі для зчитування (Opening a file in binary mode for reading)
    if (!file.is_open()) 
    {
        std::cerr << "Unable to open file for reading." << std::endl; // Виводимо помилку, якщо відкриття файлу було невдалим (Print error message if file opening fails)
        closesocket(ConnectSocket); // Зачиняємо клієнтський сокет (Close the client socket)
        WSACleanup(); // Очищуємо Winsock ресурси (Cleaning up Winsock resources)
        return 1; // Повертаємо помилку (Return error)
    }

    char dataToSendBuffer[defaultBufferLength]; // Буфер для зберігання даних для відправки (Buffer for storing data to send)
    int bytesRead; // Змінна для зберігання кількості байт, прочитаних з файлу (Variable to hold number of bytes read from file)

    /**
     * Відправляємо дані файлу на сервер
     * (Sending file data to server)
     */
    while (!file.eof()) 
    {
        file.read(dataToSendBuffer, defaultBufferLength); // Зчитати дані з файлу в буфер (Read data from the file into the buffer)
        bytesRead = file.gcount(); // Отримати кількість прочитаних байт (Get the number of bytes read)
        result = send(ConnectSocket, dataToSendBuffer, bytesRead, 0); // Надіслати дані на сервер (Send the data to the server)
        if (result == SOCKET_ERROR) 
        {
            std::cerr << "send failed with error: " << WSAGetLastError() << std::endl; //Виводимо помилку, якщо відправлення було нвдалим (Print error message if sending fails)
            file.close(); // Закриваємо файл (Closing the file)
            closesocket(ConnectSocket); // Закриваємо клієнтський сокет (Closing the client socket)
            WSACleanup(); // Очищуємо Winsock ресурси (Cleaning up Winsock resources)
            return 1; // Повертаємо помилку (Return error)
        }
    }

    std::cout << "File sent successfully: " << argv[2] << std::endl; // Друкуємо повідомлення про успіх (Print success message

    /**
     * Проводимо очистку
     * (Cleaning up)
     */
    file.close(); // Закриваємо клієнтський сокет (Close the client socket)
    closesocket(ConnectSocket); // Закриваємо клієнтський сокет (Closing the client socket)
    WSACleanup(); // Очищуємо Winsock ресурси (Cleaning up Winsock resources)

    return 0; //Програма була завершена успішно (The program was completed successfully)
}
