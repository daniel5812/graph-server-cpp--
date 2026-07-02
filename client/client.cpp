//Daniel Dahan 318840196
//Oleg Muraviov 321163446
//Ronen Yakoubov 207688581 
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

// Function to send request to server and receive response
void sendRequest(const char* serverIP, int port, int start, int end){
    // Create client socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0){
        cerr <<"Error: unable to create client socket" << endl;
        return;
    }

    // Connect to server
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // IPv4 address domain
    serverAddr.sin_port = htons(port); // Port number
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr); // Convert server IP address from text to binary form

    int n = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(n < 0){
        cerr << "Error unable to connect server" << endl;
        close(clientSocket);
        return;
    }

    // Send request to server
    char buffer[256];
    sprintf(buffer, "%d %d", start, end); // Format the request as "<start> <end>"
    send(clientSocket, buffer, sizeof(buffer), 0);

    // Receive response from server
    recv(clientSocket, buffer, sizeof(buffer), 0);
    cout << buffer << endl; // Print the shortest path received from the server

    // Close client socket
    close(clientSocket);
}

// Main function
int main(int argc, char* argv[]) {
    // Check if command-line arguments are correct
    if(argc != 5){
        cerr << "Usage: " << argv[0] << " <serverIP> <port> <start> <end>" << endl;
        return 1;
    }

    const char* serverIP = argv[1]; // Server IP address
    int port = atoi(argv[2]); // Server port number
    int start = atoi(argv[3]); // Start node of the path
    int end = atoi(argv[4]); // End node of the path

    // Send request to the server
    sendRequest(serverIP, port, start, end);

    return 0;
};
