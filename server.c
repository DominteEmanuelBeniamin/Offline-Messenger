#include <sys/types.h>
		#include <sys/socket.h>
		#include <sys/time.h>
		#include <netinet/in.h>
		#include <unistd.h>
		#include <errno.h>
		#include <stdio.h>
		#include <arpa/inet.h>
		#include <string.h>
		#include<stdlib.h>
		#include<ctype.h>
		#include<string.h>
		#include<sys/wait.h>
		#include<fcntl.h>
		#include <sqlite3.h>

		#define PORT 2728

		extern int errno;
		char * conv_addr (struct sockaddr_in address)
		{
		static char str[25];
		char port[7];

		strcpy (str, inet_ntoa (address.sin_addr));
	    memset(port, 0, 7);
		sprintf (port, ":%d", ntohs (address.sin_port));
		strcat (str, port);
		return (str);
		}
		
char lastRecipient[20] = "";

//function to create history table
int createHisotryTable(sqlite3 *db);
//function to create mail table
int createMailTable(sqlite3 *db);
//function to create users table
int createUsersTable(sqlite3 *db);
//function to login client
int loginClient(const char *nume);
//function to register client
int registerClient(const char *nume);
//function to check mail    
void checkMail(int descriptor, char nume[20]);
//function to send mail
void addMail(char nume[20], char mesaj[2000]);
//function to check history
void checkHistory(char nume1[20], char nume2[20], int descriptor);
//function to add history
void addHistory(char nume1[20], char nume2[20], char mesaj[2000]);
//function to handle logged in user
void handleLoggedInUser(int fd, int index, int numberOfUsers, int users[100], int loggedUsers[100], char nameUsers[20][20], char mesaj[1000], char response[1000], char commands[10][20]);
//function to handle not logged in user
void handleNotLoggedInUser(int fd, int index, int numberOfUsers, int loggedUsers[100], char nameUsers[20][20], char mesaj[1000], char response[1000], char commands[10][20]);
//function to handle messages
int handleMessages(int fd, int index, int numberOfUsers, int users[100], int loggedUsers[100], char nameUsers[20][20]);
//function to handle mail command
void handleMailCommand(int fd, char nameUsers[20][20], char response[1000], int index);
//function to handle history command
void handleHistoryCommand(char response[1000], char nameUsers[20][20], char commands[10][20], int index, int fd);
//function to handle login command
void handleHelpCommand(char response[1000]);
//function to handle register command
void handleReplyCommand(int fd, int index, int numberOfUsers, int users[100], char nameUsers[20][20], char mesaj[1000], char response[1000], char commands[10][20]);
//function to handle invalid command
void handleSendCommand(int fd, int index, int numberOfUsers, int users[100], char nameUsers[20][20], char mesaj[1000], char response[1000], char commands[10][20], int name);
//function to handle invalid command
void handleInvalidCommand(char response[1000]);
//function to find client
int findClient(int users[100], int numberOfUsers, int descriptor);
//function to find client by name
int findClientByName(char nameUsers[20][20], int numberOfUsers, char nume[20]);
//function to help
void help(char response[1000]);
//function to handle help command
int setupServer();
//function to accept new connection
int acceptNewConnection(int serverSocket, fd_set *activeSockets, int *maxDescriptor, int *users, int *loggedUsers, char (*nameUsers)[20], int *numberOfUsers);
//function to handle messages and disconnect
void handleMessagesAndDisconnect(int fd, int *users, int *loggedUsers, char (*nameUsers)[20], int *numberOfUsers, fd_set *activeSockets);
//function to clean up disconnected user
void cleanUpDisconnectedUser(int fd, int *users, int *loggedUsers, char (*nameUsers)[20], int *numberOfUsers, fd_set *activeSockets);


int createHistoryTable(sqlite3 *db) {
printf("Debug statement\n");
    char *err_msg = 0;
    const char *sql = "CREATE TABLE IF NOT EXISTS history ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "sender TEXT NOT NULL,"
                      "receiver TEXT NOT NULL,"
                      "message TEXT NOT NULL"
                      ");";
//create the create table query
//execute the query
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

//check if the query was executed successfully
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la crearea tabelului: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
//return the result
    return rc;
}
int createMailTable(sqlite3 *db) {
    char *err_msg = 0;
    const char *sql = "CREATE TABLE IF NOT EXISTS mail ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "receiver TEXT NOT NULL,"
                      "message TEXT NOT NULL"
                      ");";
//create the create table query
//execute the query
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
//check if the query was executed successfully
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la crearea tabelului mail: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
//return the result
    return rc;
}
	int createUsersTable(sqlite3 *db) {
    char *err_msg = 0;
    const char *sql = "CREATE TABLE IF NOT EXISTS users ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "username TEXT NOT NULL"
                      ");";
//create the create table query
//execute the query
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
//check if the query was executed successfully
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la crearea tabelului: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
//return the result
    return rc;
}

	int loginClient(const char *nume) {
    sqlite3 *db;//
    char *err_msg = 0;
    int rc;
//open the database
    rc = sqlite3_open("usernames.db", &db);
//check if the database was opened successfully
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la deschiderea bazei de date: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0; // Întoarce 0 pentru că verificarea a eșuat din cauza erorii de deschidere a bazei de date
    }
//create the users table if it doesn't exist
    const char *sql = "SELECT COUNT(*) FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;
    int rezultat = 0;
//create the select query
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la pregătirea interogării: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0; // Întoarce 0 pentru că pregătirea interogării a eșuat
    }
//bind the value
    rc = sqlite3_bind_text(stmt, 1, nume, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la legarea valorii: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0; // Întoarce 0 pentru că legarea valorii a eșuat
    }
//execute the query
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        rezultat = sqlite3_column_int(stmt, 0);
    }
//finalize the statement
    sqlite3_finalize(stmt);
    sqlite3_close(db);
//return the result
    return rezultat > 0 ? 1 : 0; // Întoarce 1 dacă numele de utilizator a fost găsit, altfel întoarce 0
}



int registerClient(const char *nume) {
    sqlite3 *db;
    char *err_msg = 0;
    int rc;
//open the database
    rc = sqlite3_open("usernames.db", &db);
//check if the database was opened successfully
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la deschiderea sau crearea bazei de date: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }
//create the users table if it doesn't exist
    rc = createUsersTable(db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return rc;
    }
//create the insert query
    const char *sql = "INSERT INTO users (username) VALUES (?);";
    sqlite3_stmt *stmt;
//create the insert query
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la pregătirea interogării: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }
//bind the value
    rc = sqlite3_bind_text(stmt, 1, nume, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server][server]Eroare la legarea valorii: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return rc;
    }
//execute the query
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "[server]Eroare la executarea interogării: %s\n", sqlite3_errmsg(db));
    }
//finalize the statement
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rc;
}
void checkMail(int descriptor, char nume[20]) {
    sqlite3 *db;
    // Open the database
    int rc = sqlite3_open("mail.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server][server]Eroare la deschiderea bazei de date: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
// Create the mail table if it doesn't exist
    rc = createMailTable(db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return;
    }
// Create the select query
    char query[1000];
    sprintf(query, "SELECT message FROM mail WHERE receiver='%s';", nume);
// Execute the query
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
// Check if the query was executed successfully
    if (rc == SQLITE_OK) {
        char buffer[2000];
        memset(buffer, 0, 2000);
// Add the mail header
        strcat(buffer, "Mail:\n");
// Add the mail messages
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            strcat(buffer, (const char *)sqlite3_column_text(stmt, 0));
            strcat(buffer, "\n");
        }
// Send the mail messages
        if (write(descriptor, buffer, strlen(buffer)) < 0)
            perror("[server]Eroare la write\n");
// Finalize the statement
        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "[server]Eroare la pregătirea interogării: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_close(db);
}


void addMail(char nume[20], char mesaj[2000]) {
    sqlite3 *db;
    int rc = sqlite3_open("mail.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la deschiderea bazei de date: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
// Create the mail table if it doesn't exist
    rc = createMailTable(db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return;
    }
// Create the insert query
    char query[1000];
    sprintf(query, "INSERT INTO mail (receiver, message) VALUES ('%s', '%s');", nume, mesaj);
// Execute the query
    rc = sqlite3_exec(db, query, 0, 0, NULL);
// Check if the query was executed successfully
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la trimiterea mail-ului: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_close(db);
}

		void checkHistory(char nume1[20], char nume2[20], int descriptor) {
    sqlite3 *db;
    char query[1000];
// Open the database
    int rc = sqlite3_open("history.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la deschiderea bazei de date: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
// Create the history table if it doesn't exist
    sprintf(query, "SELECT message FROM history WHERE (sender='%s' AND receiver='%s') OR (sender='%s' AND receiver='%s');",
            nume1, nume2, nume2, nume1);
// Execute the query
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
// Check if the query was executed successfully
    if (rc == SQLITE_OK) {
        char buffer[2000];
        memset(buffer, 0, 2000);

// Add the history header
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            strcat(buffer, (const char *)sqlite3_column_text(stmt, 0));
            strcat(buffer, "\n");
        }
// Send the history messages
        if (strlen(buffer) > 0) {
            if (write(descriptor, buffer, strlen(buffer)) < 0)
                perror("[server][server]Eroare la write\n");
        } else {
            if (write(descriptor, "Istoric gol", 11) < 0)
                perror("[server]Eroare la write\n");
        }
// Finalize the statement
        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "[server]Eroare la pregătirea interogării: %s\n", sqlite3_errmsg(db));
    }
// Close the database
    sqlite3_close(db);
}

void addHistory(char nume1[20], char nume2[20], char mesaj[2000]) {
    sqlite3 *db;
    char query[1000];
// Open the database
    int rc = sqlite3_open("history.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la deschiderea bazei de date: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
//  Create the history table if it doesn't exist
    rc = createHistoryTable(db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return;
    }
// Create the insert query
    sprintf(query, "INSERT INTO history (sender, receiver, message) VALUES ('%s', '%s', '%s');",
            nume1, nume2, mesaj);
// Execute the query
    rc = sqlite3_exec(db, query, 0, 0, NULL);
// Check if the query was executed successfully
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[server]Eroare la adăugarea în istoric: %s\n", sqlite3_errmsg(db));
    }
// Close the database

    sqlite3_close(db);
}void handleLoggedInUser(int fd, int index, int numberOfUsers, int users[100], int loggedUsers[100], char nameUsers[20][20], char mesaj[1000], char response[1000], char commands[10][20]) {
int name;
name = findClientByName(nameUsers, numberOfUsers, commands[1]);
	if (strcmp(commands[0], "/help") == 0) {
        handleHelpCommand(response);//help
    } else if (strcmp(commands[0], "/send") == 0) {
        handleSendCommand(fd, index, numberOfUsers, users, nameUsers, mesaj, response, commands, name);//send
    }else if (strcmp(commands[0], "/reply") == 0) {
        handleReplyCommand(fd, index, numberOfUsers, users, nameUsers, mesaj, response, commands);//reply
    } else if (strcmp(commands[0], "/mail") == 0) {
        handleMailCommand(fd, nameUsers, response, index);//mail
    } else if (strcmp(commands[0], "/history") == 0) {
        handleHistoryCommand(response, nameUsers, commands, index, fd);//history
    } else {
        handleInvalidCommand(response);//invalid
    }

    if (write(fd, response, strlen(response)) < 0)
        perror("[server]Eroare la write\n");
}
void handleNotLoggedInUser(int fd, int index, int numberOfUsers, int loggedUsers[100], char nameUsers[20][20], char mesaj[1000], char response[1000], char commands[10][20]) {
   
    if (strcmp(commands[0], "/login") == 0) {
        handleLoginCommand(fd, index, numberOfUsers, loggedUsers, nameUsers, mesaj, response, commands);//login
    } else if (strcmp(commands[0], "/register") == 0) {
        handleRegisterCommand(fd, response, commands);//register
    } else if (strcmp(commands[0], "/help") == 0) {
        help(response);}//help
    else {
        handleInvalidCommand(response);//invalid
    }
// Send the response
    if (write(fd, response, strlen(response)) < 0)
        perror("[server]Eroare la write\n");
}
int handleMessages(int fd, int index, int numberOfUsers, int users[100], int loggedUsers[100], char nameUsers[20][20]) {
    char mesaj[1000], response[1000], commands[10][20];//mesaj, raspuns, comenzi
    memset(commands, 0, sizeof(commands));//set commands to 0
    memset(mesaj, 0, sizeof(mesaj));//set mesaj to 0
    memset(response, 0, sizeof(response));//set response to 0
// Read the message
    if (read(fd, mesaj, 1000) <= 0) {
        perror("[server]Eroare la read() de la user.\n");
        return 0;
    }
// Parse the message
    int N = 0, j = 0;
    for (int i = 0; i < strlen(mesaj); i++) {
        if (mesaj[i] == ' ' || mesaj[i] == '\n' || mesaj[i] == '\t') {
            commands[N][j] = '\0';
            j = 0;
            N++;
        } else {
            commands[N][j++] = mesaj[i];
        }
    }
// Handle the message
    if (loggedUsers[index]) {
        handleLoggedInUser(fd, index, numberOfUsers, users, loggedUsers, nameUsers, mesaj, response, commands);//logged in
    } else {
        handleNotLoggedInUser(fd, index, numberOfUsers, loggedUsers, nameUsers, mesaj, response, commands);//not logged in
    }
    return 1;
}
void handleMailCommand(int fd, char nameUsers[20][20], char response[1000], int index) {
    checkMail(fd, nameUsers[index]);//check mail
}

void handleHistoryCommand(char response[1000], char nameUsers[20][20], char commands[10][20], int index, int fd) {
    if (!loginClient(commands[1]))
        strcat(response, "<<HOMING-SEGFULL :  User inexistent>>");
    else
        checkHistory(nameUsers[index], commands[1], fd);//check history
}




void handleLoginCommand(int fd, int index, int numberOfUsers, int loggedUsers[100], char nameUsers[20][20], char mesaj[1000], char response[1000], char commands[10][20]) {
    char aux[30];//aux
    bzero(aux, 30);//set aux to 0
if (loginClient(commands[1]) && findClientByName(nameUsers, numberOfUsers, commands[1]) == -1) {
        loggedUsers[index] = 1;//logged in
        strcpy(nameUsers[index], commands[1]);//copy name
strcat(response, "Bine ai revenit pe HOMING-SEGFULL!\nNu uita să verifici mail-ul folosind comanda /history.\nPentru commandsle acceptate, încearcă /help!\n");
        //wait(5);
        //checkMail(fd, commands[1]);
        //VERIFICA CA AICI E BUBA SI NU MERGE SI NU IMI PLACE
        //INTREABA PE PROFA CA ESTI PROST
    } else {
        strcat(response, "Username inexistent, ti-ai uitat username-ul sau esti deja logat");
    }
}


void handleRegisterCommand(int fd, char response[1000], char commands[10][20]) {
    if (!loginClient(commands[1])) {
        registerClient(commands[1]);
        strcat(response, "Bine ai venit pe HOMING-SEGFULL, continua prin logare");
    } else {
        strcat(response, "Username-ul este deja folosit, incearca altul");
    }
}


void handleInvalidCommand(char response[1000]) {
    strcat(response, "<<HOMING-SEGFULL : Comanda invalida>>");
}

// ------------------------------
// ------------------------------
// ------------------------------
// ------------------------------
// ------------------------------
// ------------------------------
		int findClient(int users[100],int numberOfUsers,int descriptor)
			{
			for(int i=0;i<numberOfUsers;i++)// for each user
				if(users[i]==descriptor) // if the descriptor is the same as the one we are looking for
				return i;//return the index
			return -1;
			}

		int findClientByName(char nameUsers[20][20], int numberOfUsers,char nume[20])
			{
			for(int i=0;i<numberOfUsers;i++)// for each user
				{
				if(strcmp(nume,nameUsers[i])==0)// if the name is the same as the one we are looking for
					{
					return i;//return the index
					}
				}
			return -1;
			}

		void help(char response[1000]) {
    strcat(response, "<<HOMING-SEGFULL : Help:\n>>");
    strcat(response, "/register <nume>            - Inregistrare\n");
    strcat(response, "/login <nume>               - Logare\n");
    strcat(response, "/send <nume> <mesaj>        - Trimitere mesaj\n");
    strcat(response, "/mail                       - Acces mail\n");
	strcat(response, "/reply <mesaj>              - Trimitere mesaj ultimului user\n");
    strcat(response, "/history <nume>             - Vezi istoricul cu un user\n");
}
void handleHelpCommand ( char response[1000]) {
    help(response);
}
void handleReplyCommand(int fd, int index, int numberOfUsers, int users[100], char nameUsers[20][20], char mesaj[1000], char response[1000], char commands[10][20]) {
    char aux[1000];
    bzero(aux, 1000);
    strncpy(aux, mesaj + 7, sizeof(aux) - 1);//copy the message
aux[sizeof(aux) - 1] = '\0';//set the last character to 0
    strcat(response, nameUsers[index]);//add the name
    strcat(response, ": ");//add :
    strcat(response, aux);//add the message

    if (strcmp(lastRecipient, "") == 0) {
        strcat(response, "<<HOMING-SEGFULL : Nu exista un utilizator anterior pentru a raspunde.>>");
    } else {
        int name = findClientByName(nameUsers, numberOfUsers, lastRecipient);
// Check if the user is logged in
        if (name == -1) {
            strcat(response, "<<HOMING-SEGFULL : Utilizatorul anterior nu exista.>>");
        } else {
            if (write(users[name], response, 1000) < 0)
                perror("[server]Eroare la write\n");
            addHistory(nameUsers[index], lastRecipient, response);//add the history
            bzero(response, 1000);//set response to 0
        }
    }
}

void handleSendCommand(int fd, int index, int numberOfUsers, int users[100], char nameUsers[20][20], char mesaj[1000], char response[1000], char commands[10][20], int name) {
    char aux[1000];
    bzero(aux, 1000);
    bzero(lastRecipient, 1000);
    strncpy(aux, mesaj + 7, sizeof(aux) - 1);//copy the message
aux[sizeof(aux) - 1] = '\0';//set the last character to 0
    strcat(response, nameUsers[index]);//add the name
    strcat(response, ": ");//add :
    strcat(response, aux);//add the message

    if (name == -1) {
        if (!loginClient(commands[1])) {
            bzero(response, 1000);
            strcat(response, "<<HOMING-SEGFULL :  User inexistent>>");
        } else {
            addMail(commands[1], response);
            addHistory(nameUsers[index], commands[1], response);
            
            strcpy(lastRecipient, commands[1]); // Actualizeaza lastRecipient
            bzero(response, 1000);
        }
    } else {
        if (write(users[name], response, 1000) < 0)
            perror("[server]Eroare la write\n");
        addHistory(nameUsers[index], commands[1], response);
        strcpy(lastRecipient, commands[1]); // Actualizeaza lastRecipient
        bzero(response, 1000);
    }
}


		
int main() {
    int serverSocket = setupServer();///setup server

    fd_set activeSockets;//active sockets
    fd_set readSockets;//read sockets
    FD_ZERO(&activeSockets);//set active sockets to 0
    FD_SET(serverSocket, &activeSockets);//set server socket to active sockets

    int maxDescriptor = serverSocket;//max descriptor
    int users[100];//users
    int loggedUsers[100];//logged users
    char nameUsers[20][20];//name users
    int numberOfUsers = 0;//number of users

    printf("[server] Waiting on port %d...\n", PORT);
    fflush(stdout);

    while (1) {
        readSockets = activeSockets;//read sockets

        if (select(maxDescriptor + 1, &readSockets, NULL, NULL, NULL) < 0) {
            perror("[server] Error in select().\n");//
            return errno;
        }

        if (FD_ISSET(serverSocket, &readSockets)) {
            // A new connection is available
            acceptNewConnection(serverSocket, &activeSockets, &maxDescriptor, users, loggedUsers, nameUsers, &numberOfUsers);//accept new connection
        }
        int fd;//fd
        for (fd = 0; fd <= maxDescriptor; fd++) {
            // Check which sockets have data available to read
            if (fd != serverSocket && FD_ISSET(fd, &readSockets)) {
                // A message is available
                handleMessagesAndDisconnect(fd, users, loggedUsers, nameUsers, &numberOfUsers, &activeSockets);
            }
        }
    }

    close(serverSocket);//close server socket
    return 0;//return 0
}

int setupServer() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);//server socket
    if (serverSocket == -1) {
        // Error creating socket
        perror("[server] Error creating socket.\n");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));//set socket options

    struct sockaddr_in server;//server
    memset(&server, 0, sizeof(server));//set server to 0
    server.sin_family = AF_INET;//set server family to AF_INET
    server.sin_addr.s_addr = htonl(INADDR_ANY);//set server address
    server.sin_port = htons(PORT);//set server port

    if (bind(serverSocket, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        // Error binding socket
        perror("[server] Error binding socket.\n");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) == -1) {
        // Error listening on socket
        perror("[server] Error listening on socket.\n");
        exit(EXIT_FAILURE);
    }

    return serverSocket;//return server socket
}


int acceptNewConnection(int serverSocket, fd_set *activeSockets, int *maxDescriptor, int *users, int *loggedUsers, char (*nameUsers)[20], int *numberOfUsers) {
    struct sockaddr_in clientAddr;//client address
    socklen_t addrLen = sizeof(clientAddr);//address length
    int newSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);//new socket
// Check if the connection was accepted
    if (newSocket < 0) {
        perror("[server] Error accepting connection.\n");
        return -1;
    }
// Add the new connection to the list of active sockets
    FD_SET(newSocket, activeSockets);
// Update the maximum descriptor
    if (newSocket > *maxDescriptor) {
        *maxDescriptor = newSocket;
    }
// Add the new user to the list of users
    printf("[server] New connection from %s on socket %d\n", inet_ntoa(clientAddr.sin_addr), newSocket);

    users[*numberOfUsers] = newSocket;//add the new user
    loggedUsers[*numberOfUsers] = 0;//set logged users to 0
    (*numberOfUsers)++;//increment number of users


    return newSocket;//return new socket
}


void handleMessagesAndDisconnect(int fd, int *users, int *loggedUsers, char (*nameUsers)[20], int *numberOfUsers, fd_set *activeSockets) {

    if (handleMessages(fd, findClient(users, *numberOfUsers, fd), *numberOfUsers, users, loggedUsers, nameUsers) == 0) {
        // The user disconnected
        cleanUpDisconnectedUser(fd, users, loggedUsers, nameUsers, numberOfUsers, activeSockets);
    }
}

void cleanUpDisconnectedUser(int fd, int *users, int *loggedUsers, char (*nameUsers)[20], int *numberOfUsers, fd_set *activeSockets) {
    int position = findClient(users, *numberOfUsers, fd);
    fflush(stdout);
    close(fd);

    memmove(users + position, users + position + 1, (*numberOfUsers - position - 1) * sizeof(int));//move users
    memmove(loggedUsers + position, loggedUsers + position + 1, (*numberOfUsers - position - 1) * sizeof(int));//move logged users
    memmove(nameUsers + position, nameUsers + position + 1, (*numberOfUsers - position - 1) * sizeof(char[20]));//move name users

    (*numberOfUsers)--;//decrement number of users
    FD_CLR(fd, activeSockets);//clear fd
}



