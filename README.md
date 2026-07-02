# Graph Server

A multi-threaded C++ network server that computes shortest paths in graphs using BFS (Breadth-First Search) with built-in path caching for improved performance.

## Features

- **BFS Shortest Path Computation**: Finds the shortest path between two nodes in an undirected graph
- **Multi-threaded Server**: Handles multiple concurrent client connections using thread pools
- **LRU Path Cache**: Caches up to 10 most recent paths for faster retrieval on repeated queries
- **Network Protocol**: Simple TCP socket-based communication for client-server interaction

## Project Structure

```text
project/
├── server/
│   ├── server.cpp      # Server implementation with BFS and caching logic
│   └── db.csv          # Sample graph data (edge list format)
└── client/
    └── client.cpp      # Client implementation for querying the server
```

## Graph Format

The graph is stored as an edge list in plain text format (one edge per line):

```text
node1 node2
node1 node2
...
```

Each line contains two space-separated integers representing an undirected edge between two nodes.

Example (`db.csv`):

```text
0 18427
1 21708
1 22208
```

## Building

### Prerequisites

- C++17 compatible compiler (g++, clang)
- POSIX-compliant system (Linux, macOS)

### Compile

From the project root:

```bash
# Build server
cd project/server
g++ -std=c++17 -pthread -o server server.cpp

# Build client
cd ../client
g++ -std=c++17 -o client client.cpp
```

## Running

### Start the Server

```bash
cd project/server
./server <graph_file> <port>
```

**Arguments:**

- `<graph_file>`: Path to the graph file (e.g., `db.csv`)
- `<port>`: Port number to listen on (e.g., `9999`)

**Example:**

```bash
./server db.csv 9999
```

Output:

```text
Server listening to port 9999
```

### Query from a Client

In another terminal:

```bash
cd project/client
./client <server_ip> <port> <start_node> <end_node>
```

**Arguments:**

- `<server_ip>`: Server IP address (e.g., `127.0.0.1` for localhost)
- `<port>`: Server port (must match server's listening port)
- `<start_node>`: Starting node ID
- `<end_node>`: Destination node ID

**Example:**

```bash
./client 127.0.0.1 9999 0 18427
```

Output:

```text
0 18427
```

The output is the shortest path from the start node to the end node (space-separated node IDs). If no path exists, the server returns `No path found`.

## Example Workflow

**Terminal 1 - Start Server:**

```bash
cd project/server
./server db.csv 8000
# Output: Server listening to port 8000
```

**Terminal 2 - Query 1:**

```bash
cd project/client
./client 127.0.0.1 8000 5 100
# Output: (shortest path from node 5 to node 100)
```

**Terminal 2 - Query 2:**

```bash
./client 127.0.0.1 8000 5 100
# Output: (same path, retrieved from cache)
```

## Implementation Details

### Server (server.cpp)

- **Graph Representation**: Adjacency list using `unordered_map<int, vector<int>>`
- **Shortest Path Algorithm**: BFS (breadth-first search)
- **Caching**: LRU cache storing up to 10 paths
- **Concurrency**: Thread-safe cache using mutex locks
- **Thread Management**: Each client connection spawns a new detached thread

### Client (client.cpp)

- Creates a TCP socket connection to the server
- Sends query as two space-separated integers
- Receives and displays the shortest path
- Closes connection after receiving response

## Performance Considerations

- **Cache Size**: 10 most recent queries are cached (configurable via `CACHE_SIZE` in server.cpp)
- **Thread Safety**: All cache operations are protected by mutex
- **Scalability**: Multi-threaded design allows handling multiple concurrent clients
- **Memory**: Entire graph is loaded into memory on startup

## Compilation Flags

- `-std=c++17`: C++17 standard required for certain language features
- `-pthread`: Link pthread library for multi-threading support
