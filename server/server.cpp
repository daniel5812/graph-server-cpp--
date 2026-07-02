//Daniel Dahan 318840196
//Oleg Muraviov 321163446
//Ronen Yakoubov 207688581 
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

// Define a structure to represent a graph
struct Graph {
    unordered_map<int, vector<int>> adjList; // Adjacency list representation
};

// Global variables for caching
const int CACHE_SIZE = 10;
unordered_map<string, vector<int>> cache; // Cache to store computed paths
queue<string> cacheOrder; // Queue to track the order of cached items
mutex cacheMutex; // Mutex for thread-safe access to cache

// Function to read a graph from a file
Graph readGraphFromFile(const string& fileName){
    ifstream file(fileName);
    Graph graph;

    // Open the file
    if(file.is_open()) {
        int node1, node2;
        // Read edges from the file and populate the graph
        while (file >> node1 >> node2){
            graph.adjList[node1].push_back(node2);
            graph.adjList[node2].push_back(node1);
        }
        file.close();
    }  else{
        cerr << "Error: Unable to open file " << fileName << endl;
    }
    return graph;
}

// Function to perform Breadth First Search to find the shortest path
vector<int> BFS(const Graph& graph, int start, int end){
    queue<vector<int>> q; // Queue of paths
    q.push({start}); // Initialize with the start node
    vector<bool> visited(graph.adjList.size(), false); // Track visited nodes

    while (!q.empty()){
        vector<int> path = q.front(); // Current path
        q.pop();
        int node = path.back(); // Last node in the path

        // If the last node is the destination, return the path
        if(node == end)
            return path;

        // If the node hasn't been visited, explore its neighbors
        if(!visited[node]){
            visited[node] = true;
            // Iterate over neighbors
            for(int neighbor : graph.adjList.at(node)){
                // Create a new path by appending the neighbor to the current path
                if(!visited[neighbor]) {
                    vector<int> newPath = path;
                    newPath.push_back(neighbor);
                    q.push(newPath);
                }
            }
        }
    }
    return {}; // No path found
}

// Function to handle client requests
void handleClient(int clientSocket, const Graph& graph){
    char buffer[256];
    int n;

    while (true){
        // Receive data from client
        n = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (n <= 0){
            cerr << "Error: Connection closed by client" << endl;
            close(clientSocket);
            return;
        }

        // Process client request
        int start, end;
        sscanf(buffer, "%d %d", &start, &end);

        // Check cache for a cached path
        string key = to_string(start) + "-" + to_string(end);
        vector<int> cachedPath;
        bool useCache = false; // Flag to check if cached path is found

        {
            lock_guard<mutex> lock(cacheMutex);
            if (cache.find(key) != cache.end()) {
                cachedPath = cache[key];
                useCache = true;
            }
        }

        // Calculate path or use cache
        vector<int> path;
        if(!useCache){
            // If path not found in cache, compute using BFS
            path = BFS(graph, start, end);

            // Update cache with the computed path
            {
                lock_guard<mutex> lock(cacheMutex);
                cache[key] = path;
                cacheOrder.push(key);
                // If cache size exceeds CACHE_SIZE, remove the oldest item
                if(cacheOrder.size() > CACHE_SIZE){
                    string oldestKey = cacheOrder.front();
                    cacheOrder.pop();
                    cache.erase(oldestKey);
                }
            }
        }  else {
            // If cached path found, use it
            path = cachedPath;
        }

        // Send response to client
        if(path.empty()){
            send(clientSocket, "No path found", sizeof("No path found"), 0);
        } else{
            // Convert path to string and send to client
            string response;
            for(int node : path){
                response += to_string(node) + " ";
            }
            send(clientSocket, response.c_str(), response.size(), 0);
        }
    }
}

// Function to run the server
void runServer(int port, const string& filename){

    // Read graph from file
    Graph graph = readGraphFromFile(filename);

    // Create server socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket < 0){
        cerr << "Error: Unable to create server socket" << endl;
        return;
    }

    // Bind server socket
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    int n = bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(n<0){
        cerr << "Error unable to bind server socket" << endl;
        close(serverSocket);
        return;
    }

    // Listen for connections
    listen(serverSocket, 5);
    cout << "Server listening to port "<< port <<endl;

    // Accept and handle client connections
    while(true){
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

        if(clientSocket <0){
            cerr << "Error: Unable to accept client connection" << endl;
            continue;
        }
        cout << "Client connected" << endl;

        // Handle client in a separate thread
        thread clientThread(handleClient, clientSocket, ref(graph));
        clientThread.detach(); // Detach thread to allow it to run independently
    }

    // Close server socket
    close(serverSocket);
}

// Main function
int main(int argc, char* argv[]){   
    if(argc != 3){
        cerr << "Usage: " << argv[0] << " <filename> <port>" << endl;
        return 1;
    }

    string filename = argv[1];
    int port = atoi(argv[2]);

    // Run the server
    runServer(port, filename);

    return 0;
}
