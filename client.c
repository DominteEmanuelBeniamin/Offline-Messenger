#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_MSG_SIZE 1000

int sd;
GtkWidget *message_view;
GtkWidget *message_entry;

void send_message(GtkWidget *widget, gpointer data) {
    const gchar *message = gtk_entry_get_text(GTK_ENTRY(message_entry));

    if (strlen(message) > 0) {
        if (write(sd, message, strlen(message)) <= 0) {
            perror("[client] Error writing to server.\n");
            exit(EXIT_FAILURE);
        }
    }

    gtk_entry_set_text(GTK_ENTRY(message_entry), "");
}
 
void *receive_messages(void *arg) {
    char msg[MAX_MSG_SIZE];

    while (1) {
        memset(msg, 0, MAX_MSG_SIZE);
        if (read(sd, msg, MAX_MSG_SIZE) <= 0) {
            perror("[client] Error reading from server.\n");
            exit(EXIT_FAILURE);
        }

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(message_view));
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(buffer, &iter);
        gtk_text_buffer_insert(buffer, &iter, msg, -1);
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    struct sockaddr_in server;

    int port = 2728;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[client] Error creating socket.\n");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        perror("[client] Error connecting to server.\n");
        exit(EXIT_FAILURE);
    }

    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) != 0) {
        perror("[client] Error creating receive thread.\n");
        exit(EXIT_FAILURE);
    }

    gtk_init(&argc, &argv);

    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *chat_container;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Chat Client");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    chat_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_name(chat_container, "custom-chat-container");

    message_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(message_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(message_view), FALSE);
    gtk_widget_set_size_request(message_view, 500, 300);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(message_view));
    gtk_text_buffer_set_text(buffer, "", -1);

    gtk_box_pack_start(GTK_BOX(chat_container), message_view, TRUE, TRUE, 0);

    message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Type your message here");
    gtk_entry_set_width_chars(GTK_ENTRY(message_entry), 40);
    gtk_box_pack_start(GTK_BOX(chat_container), message_entry, FALSE, FALSE, 0);

    GtkWidget *send_button = gtk_button_new_with_label("Send");
    g_signal_connect(send_button, "clicked", G_CALLBACK(send_message), NULL);
    gtk_box_pack_start(GTK_BOX(chat_container), send_button, FALSE, FALSE, 0);

    gtk_grid_attach(GTK_GRID(grid), chat_container, 0, 0, 1, 1);

    gtk_widget_show_all(window);

    gtk_main();

    close(sd);
    return 0;
}
