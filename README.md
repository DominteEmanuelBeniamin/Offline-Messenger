# Project Title: Chat Application

## Description
This project is a simple chat application that enables users to communicate with each other in real-time. It employs a client-server architecture to manage connections and message exchanges. The application also features a graphical user interface for ease of use.

## Features
- Real-time messaging
- User registration and login system
- Mail checking functionality
- Message history retrieval

## Technologies Used
- **C Programming Language**: For the core application logic and network communication.
- **GTK+ 3**: For creating the graphical user interface, making the application user-friendly.
- **SQLite**: For database management, storing user information, mail, and message history.
- **POSIX Threads (pthread)**: For handling multiple connections simultaneously, allowing asynchronous message receiving.
- **Sockets with TCP**: For reliable network communication between the client and server.
- **netinet/in.h, arpa/inet.h**: For Internet address family, used in network communication.

## Communication Protocol: TCP

### Why TCP?
- **Reliability**: TCP ensures the reliable delivery of messages between the client and server. It manages message acknowledgment, retransmission of lost packets, and error detection, which are crucial for a chat application where message integrity is important.
- **Ordered Delivery**: TCP maintains the order of messages as sent. This is essential for a chat application to ensure that messages appear in the correct sequence on the recipient's end.
- **Connection-Oriented**: TCP establishes a dedicated connection between the client and server before data transfer begins. This persistent connection facilitates a continuous flow of messages, making it suitable for real-time communication.
- **Flow Control**: TCP provides flow control mechanisms to avoid overwhelming the network or the receiving application. This ensures that the chat application remains responsive even under heavy network traffic.
- **Congestion Control**: TCP implements congestion control algorithms to minimize network congestion, improving the overall stability and performance of the chat application.

By leveraging TCP's features through the socket programming API in C, this chat application ensures a robust, reliable, and efficient communication channel between clients and the server, providing a seamless user experience.

