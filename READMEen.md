C++ KV Store — Persistent, Concurrent, Networked Key-Value Store

A from-scratch implementation, in C++20, of a Redis-inspired key-value storage system: a custom data structure (no std::unordered_map), disk persistence via a write-ahead log, a TCP server built directly on raw Winsock sockets, and support for multiple simultaneous clients through threads and mutex-based synchronization.

This project was built as a deep-learning exercise in systems programming, memory management, networking, and concurrency in C++ — prioritizing a real understanding of each low-level piece over relying on libraries that abstract the problem away.

Features
Hash table with chaining, implemented from scratch (no STL containers for the core storage)
Real persistence via a write-ahead log — data survives a process close or crash
TCP server built on Winsock, with a custom plain-text protocol (SET / GET / DEL)
Real concurrency: multiple clients handled simultaneously, each on its own thread, safely sharing the same data store
Input validation against malformed commands (explicit protection against an overflow bug that was found and fixed during development — see the bug section below)
Architecture
Client (PowerShell / any TCP client)
        │
        │  plain text: "SET dog labrador"
        ▼
┌───────────────────────┐
│   TCP Server            │  Winsock: socket → bind → listen → accept
│   (Server.h)            │  One thread (std::jthread) per connected client
└───────────┬────────────┘
            │  ParserCommand() → Eject()
            ▼
┌───────────────────────┐
│   HashTable              │  Data structure + persistence
│   (Hash.h)               │  Protected by std::mutex
├───────────────────────┤
│  Buckets: vector<Nodo*>  │  Chaining for collision resolution
│  Write-ahead log (.log)  │  State reconstruction on startup
└───────────────────────┘
How It Works
The Hash Table

Each key is converted into an index using a polynomial hash function:

cpp
hash = hash * 31 + ascii_char
index = hash % table_size

The prime number 31 is used as a multiplier because it distributes keys more evenly across buckets than a simple sum of character codes would — without multiplying by a prime, words containing the same letters in a different order (like "ROMA" and "AMOR") would always collide into the same index.

Collisions are resolved with chaining: each position in the bucket vector is the head of a linked list. Chaining was chosen over open addressing because it's simpler to reason about correctly — it avoids the clustering issues and special deletion handling that open addressing requires, at the cost of slightly worse memory locality.

Persistence — Write-Ahead Log

Every state-changing operation (SET, DEL) is written immediately to a file on disk, in the format:

SET|key|value
DEL|key

On startup, the program re-reads that file line by line and reconstructs the in-memory state by replaying each operation in the exact order it happened — the same approach real database engines use. This ordering is critical: if a key was updated and later deleted, the final state must reflect that exact sequence, not a summarized version.

The separation between the public SET/DEL (which update memory and write to the log) and their memory-only private counterparts was a necessary design decision: without it, the startup reconstruction would rewrite every line already present in the file, causing it to grow unbounded on every restart.

Accepted trade-off: the log grows indefinitely, and every startup re-reads the full history — acceptable for a project of this scope; in a real production system this would be solved with periodic snapshotting (saving the full state and discarding the old log), an extension identified but not implemented in this version.

The TCP Server

Built directly on the Winsock API (no network abstraction libraries), following the standard socket sequence:

WSAStartup → socket → bind → listen → accept → recv/send → closesocket → WSACleanup

The protocol is plain text over TCP: the client sends a line like "GET dog", the server parses it, executes the corresponding operation on the hash table, and responds with the result as text.

Concurrency

Each accepted client is handled on its own thread (std::jthread, detached with .detach() so the main thread never blocks and immediately goes back to accepting new connections). All threads share the same HashTable instance — that's the whole point of a KV store, that data is shared across clients.

That shared access is protected by a std::mutex internal to HashTable, used via std::lock_guard inside SET, GET, and DEL. Think of it as a bodyguard: only one thread at a time can be "talking" to the protected data — any other thread that arrives in the meantime waits its turn, preventing two threads from reconnecting pointers or reading memory that another is modifying at the same instant (a race condition).

Accepted trade-off: the mutex is global to the whole table, not per-bucket — simpler to reason about and guarantees correctness, at the cost of lower real-world parallelism under very high load (two threads operating on different buckets still have to wait on each other). A per-bucket lock would be a reasonable future improvement.

A Real Bug, Found and Fixed

During development, it was discovered that a malformed command (SET dog, missing the value to store) caused a full server crash — not a graceful error, but an Assertion failed from an out-of-bounds access on a std::vector.

Root cause: the parser looked for a second space in the command to delimit the value. When that space didn't exist, std::string::find returned std::string::npos (the maximum representable value of an unsigned integer). Doing arithmetic with that value (npos + 1) silently overflowed, producing invalid memory positions — no exception was thrown until, later on, the code tried to access a command vector element that never actually existed.

Fix: explicit validation of the position returned by find before using it, rejecting the command with a controlled error message ("ERR unknow command") instead of attempting to process it with incomplete data.

This finding is why the project actively validates its input in the parser — any system receiving data from an external source (a network client, in this case) cannot assume that data arrives well-formed.

Technologies and Concepts Applied
Area	Details
Data structures	Custom hash table, chaining, linked lists
Memory management	Raw pointers (new/delete), leak prevention, dangling pointer and use-after-free prevention
Persistence	Write-ahead log, delimiter-based serialization, state reconstruction
Networking	Winsock, TCP sockets, client-server architecture, custom protocol design
Concurrency	std::jthread, std::mutex, std::lock_guard, std::ref
Robustness	Input validation, explicit error handling at every layer
Tooling	Git (branching, merging), GitHub, VS Code + MinGW-w64/GCC, GDB
Building and Running

Requires MinGW-w64 (GCC) with C++20 support and the Winsock ws2_32 library (Windows).

bash
g++ -std=c++20 -g -Wall -Wextra src/*.cpp -Iinclude -o kv-store.exe -lws2_32
./kv-store.exe

The server listens on port 8080 by default. It can be reached by any TCP client that sends plain text terminated by a newline — for example, from PowerShell:

powershell
$socket = New-Object System.Net.Sockets.TcpClient("localhost", 8080)
$stream = $socket.GetStream()
$writer = New-Object System.IO.StreamWriter($stream)
$writer.AutoFlush = $true
$reader = New-Object System.IO.StreamReader($stream)

$writer.WriteLine("SET dog labrador")
$reader.ReadLine()
Supported Commands
Command	Format	Description
SET	SET key value	Creates or updates a key
GET	GET key	Retrieves the value of a key
DEL	DEL key	Deletes a key
Possible Future Extensions
Periodic snapshotting, to avoid re-reading the full log on every startup
Per-bucket locks instead of a global mutex, for greater parallelism
Binary RESP-style protocol (Redis's own) instead of plain text
Key expiration (TTL)
Throughput benchmarking under concurrent load
A Note on the Development Process

This project was built incrementally, phase by phase (data structure → persistence → networking → concurrency), with active code review and debugging at each stage. The commit history reflects that real process, including the fixing of memory bugs (dangling pointers, use-after-free, memory leaks) and robustness issues (the overflow described above) as they were discovered.
