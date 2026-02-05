
## 📁 Structure of the Project

```
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
```

---

## 📝 Overview

This project implements a **file transfer system** where:

* **Server** monitors a file (`data.txt`) for changes.
* When the file changes, the server **fires an event** (`NewDataAvailable`) to all subscribed clients.
* Clients receive the event and then request the updated file **chunk by chunk**.
* File transfer supports **timestamps** to ensure only new changes are downloaded.

---

## 🛠 How It Works (Textual Flow Diagram)

```
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
```

**Key points:**

1. The server sends events **only when the file changes**.
2. The client compares the event timestamp with its local timestamp.
3. Only if the server timestamp is newer, the client downloads the file.
4. File is downloaded **chunk by chunk** to handle large files.

---

## 🧩 File Details

### 1️⃣ `SendFile.fidl` (Service Interface)

Defines:

* `RequestDownloadFile` method:

  * Inputs: `DataVersion`, `ClientTimestamp`
  * Outputs: `accepted`, `totalBytes`, `chunkSize`, `ServerTimestamp`
* `RequestData` method:

  * Inputs: `chunkIndex`, `DataVersion`
  * Outputs: `data`, `isLastChunk`, `chunkIndex`
* `NewDataAvailable` broadcast event:

  * Outputs: `DataVersion`, `Timestamp`

---

### 2️⃣ `SendFileImplStub.hpp` (Server Implementation)

Responsibilities:

* Monitors the file using `std::filesystem::last_write_time`.
* Fires `NewDataAvailable` event when file changes.
* Handles client requests:

  * `RequestDownloadFile` → sends total size, chunk size, and server timestamp.
  * `RequestData` → sends requested chunk of the file.

---

### 3️⃣ `Server.cpp`

Responsibilities:

* Initialize CommonAPI runtime.
* Create `SendFileImplStub` instance with the file path.
* Register the service to CommonAPI runtime.
* Loop:

  * Monitor file changes.
  * Fire events when changes occur.

---

### 4️⃣ `Client.cpp`

Responsibilities:

* Initialize CommonAPI runtime.
* Build proxy to `SendFile` service.
* Subscribe to `NewDataAvailable` event.
* On event:

  * Compare server timestamp with local timestamp.
  * If newer, request download via `RequestDownloadFileAsync`.
  * Loop over chunks using `RequestData`.
  * Write chunks to local file `received.txt`.
* Update local version and timestamp.

---

### 5️⃣ `CMakeLists.txt`

Defines:

* Shared library for generated SOME/IP code.
* Server executable linked with CommonAPI, vsomeip3, and shared library.
* Client executable linked with CommonAPI, vsomeip3, and shared library.

---

### 6️⃣ `commonapi-someip.json`

SOME/IP runtime configuration:

* Unicast address: `127.0.0.1`
* Applications: server (`0x1277`), client (`0x1255`)
* Service: `0x4666`
* Ports: 30500 (unreliable), 30499 (reliable)
* Routing: server
* Service discovery: enabled via multicast `224.224.224.245`

---

## ⚡ Build & Run Instructions

**Step 1: Build the project**

```bash
cd ~/ITI/fady/project_vsome-ip/build
cmake ..
make -j
```

**Step 2: Run the server**

```bash
cd ~/ITI/fady/project_vsome-ip/build

export LD_LIBRARY_PATH=$(pwd):/usr/local/lib/commonapi:/usr/local/lib:$LD_LIBRARY_PATH
export COMMONAPI_CONFIG=/etc/commonapi4someip.ini

./SendFileServer
```

**Step 3: Run the client**

```bash
cd ~/ITI/fady/project_vsome-ip/build

export COMMONAPI_SOMEIP_CONFIG=/etc/commonapi-someip.ini
export LD_LIBRARY_PATH=/usr/local/lib/commonapi:/usr/local/lib:$LD_LIBRARY_PATH

./SendFileClient
```

---
## 🔧 Notes

* **File path must be absolute** in `Server.cpp` for `SendFileImplStub`.
* Event is only fired if **file timestamp changes**.
* Chunk size is **1024 bytes** by default.
* Server does **not send old data** if the client is already up to date.
* Make sure **server runs before client**.

---

## 🖼 ASCII Flow Example

```
Server detects file change
        │
        │
  Fire NewDataAvailable event
        │
        ▼
   Client receives event
        │
Check timestamp -> If newer
        │
        ▼
  RequestDownloadFileAsync
        │
        ▼
Loop RequestData (chunk by chunk)
        │
        ▼
Write to received.txt
        │
        ▼
 Update localTimestamp & localVersion
```
