# project_vsome-ip

Structure of the Project
project_vsome-ip/
│
├── Src/
│   ├── Server.cpp         # Implements the SendFile service
│   └── Client.cpp         # Implements the client that requests files
│
├── fidl/
│   ├── move.fidl
│   ├── SendFile.fidl      # Service interface definition
│   └── src-gen/           # Auto-generated proxy/stub/deployment files
│
├── data.txt               # Example file to be sent
├── CMakeLists.txt         # CMake build script
└── commonapi-someip.json  # SOME/IP runtime configuration

verview

This project implements a file transfer system where:

Server monitors a file (data.txt) for changes.

When the file changes, the server fires an event (NewDataAvailable) to all subscribed clients.

Clients receive the event and then request the updated file chunk by chunk.

File transfer supports timestamps to ensure only new changes are downloaded.


How It Works (Textual Flow Diagram)
 +----------------+            +----------------+
 |    Server      |            |     Client     |
 +----------------+            +----------------+
 | Monitors file  |            | Waits for event|
 |   data.txt     |            |                |
 |----------------|            |----------------|
 | File changed?  |----Event-->| Receives event |
 |   yes ->       |            | (version+ts)   |
 | Fire Event     |            |----------------|
 |----------------|            | RequestDownload|

 
 | Wait 1 sec     |            | Async call     |
 +----------------+            |----------------|
                               | Loop over chunks|
                               | RequestData()   |
                               | Write to file   |
                               +----------------+

Key points:

The server sends events only when the file changes.

The client compares the event timestamp with its local timestamp.

Only if the server timestamp is newer, the client downloads the file.

File is downloaded chunk by chunk to handle large files.


File Details
1️⃣ SendFile.fidl (Service Interface)

Defines:

RequestDownloadFile method:

Inputs: DataVersion, ClientTimestamp

Outputs: accepted, totalBytes, chunkSize, ServerTimestamp

RequestData method:

Inputs: chunkIndex, DataVersion

Outputs: data, isLastChunk, chunkIndex

NewDataAvailable broadcast event:

Outputs: DataVersion, Timestamp



                               
