# Offline Messenger

## Overview
Offline Messenger is a client-server messaging application that allows users to send and receive messages while offline. The system utilizes SQLite for message storage, enabling users to retrieve messages upon reconnecting. The server handles user authentication, message storage, and message retrieval, while the client provides an interface for users to communicate.

## Features
* User authentication (login and registration)
* Message storage in SQLite database
* Offline messaging capabilities
* Message history and inbox retrieval
* Support for concurrent clients using multithreading

## Installation
### Prerequisites
* GCC compiler
* SQLite3
* pthread library

### Steps
1. Compile the server:
```
gcc -o server server.c -lpthread -lsqlite3
```
2. Compile the client:
```
gcc -o client client.c
```

## Usage
### Running the server
```
./server
```
### Running the client
```
./client
```

### Commands
* **register** – Register a new user.
* **login** – Log in as an existing user.
* **msg :** – Send a message to another user.
* **inbox** – View unread messages.
* **history** – View message history with a user.
* **reply <message_id> :** – Reply to a specific message.
* **list online** – View currently online users.
* **logout** – Log out from the system.
* **quit** – Exit the client.

## Database Structure
The application stores user and message data in an SQLite database:
* **USER** (ID, NAME, IS_ACTIVE)
* **JUNCTION** (USER_ID, WITH_USER_ID, CONVERSATION_ID)
* **CONVERSATION** (INDEX_MESSAGE, CONVERSATION_ID, FROM_ID, MESSAGE_CONTENT, READ_FLAG, CREATED_ON)
