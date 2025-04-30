#include "network.h"
#include "core/app.h"
#include "util/animations.h"
#include <easynet.h>
#include <time.h>
#include <easythreads.h>

IMPL_ARRLIST(Message);
IMPL_ARRLIST(User);
IMPL_ARRLIST_NAMED(ez_ConnectionPtr, ez_Connection*);
IMPL_ARRLIST(LinkedClient);
IMPL_ARRLIST(QueuedMessage);

Network g_network = { 0 };
EZ_COND g_send_condition;
EZ_THREAD g_accept_thread;
EZ_THREAD g_listen_thread;
EZ_THREAD g_sender_thread;
ARRLIST_ez_ConnectionPtr g_in_connections = { 0 };
ez_Server* g_listener = NULL;
ARRLIST_LinkedClient g_out_connections = { 0 };
ARRLIST_QueuedMessage g_send_queue = { 0 };

EZ_THREAD_RETURN_TYPE accept_thread(EZ_THREAD_PARAMETER_TYPE params) {
    while(1) {
        ez_Connection* connection = EZ_SERVER_ACCEPT(g_listener);
        EZ_LOCK_MUTEX((*Lock()));
        ARRLIST_ez_ConnectionPtr_add(&g_in_connections, connection);
        EZ_RELEASE_MUTEX((*Lock()));
    }
	return 0;
}

EZ_THREAD_RETURN_TYPE listen_thread(EZ_THREAD_PARAMETER_TYPE params) {
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000; // 100 ms
    fd_set read_fds;
    EZ_SOCKET max_fd;
    while (1) {
        EZ_LOCK_MUTEX((*Lock()));
        if (g_in_connections.size == 0) {
            EZ_RELEASE_MUTEX((*Lock()));
            Wait(100);
            continue;
        }
        max_fd = g_in_connections.data[0]->socket;
        FD_ZERO(&read_fds);
        for (size_t i = 0; i < g_in_connections.size; i++) {
            if (g_in_connections.data[i]->socket > max_fd) {
                max_fd = g_in_connections.data[i]->socket;
            }
            FD_SET(g_in_connections.data[i]->socket, &read_fds);
        }
        EZ_RELEASE_MUTEX((*Lock()));
        if (select(max_fd + 1, &read_fds, NULL, NULL, &timeout) > 0) {
            EZ_LOCK_MUTEX((*Lock()));
            ez_Buffer* ebuffer = EZ_GENERATE_BUFFER(sizeof(Message));
            for (size_t i = 0; i < g_in_connections.size; i++) {
                if (EZ_SERVER_ASK(g_in_connections.data[i], ebuffer)) {
                    // TODO: add to proper history
                    Message msg = { 0 };
                    EZ_TRANSLATE_BUFFER(ebuffer, &msg);
                    ARRLIST_Message_add(&(g_network.friends.data[0].history), msg);
                    AppState curr_state = GetState();
                    Event e = { 0 };
                    e.recieve = TRUE;
                    curr_state(e);
                }
            }
            EZ_CLEAN_BUFFER(ebuffer);
            EZ_RELEASE_MUTEX((*Lock()));
        }
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
    }
	return 0;
}

EZ_THREAD_RETURN_TYPE sender_thread(EZ_THREAD_PARAMETER_TYPE params) {
    while (1) {
        EZ_WAIT_COND(g_send_condition, (*Lock()));
        for (size_t i = 0; i < g_send_queue.size; i++) {
            QueuedMessage qm = g_send_queue.data[i];
            int state = 0;
            LinkedClient lc = { 0 };
            while (1) {
                BOOL breakout = FALSE;
                switch (state) {
                    case 0: // sending state
                        for (size_t j = 0; j < g_out_connections.size; j++) {
                            if (g_out_connections.data[j].user == qm.user) {
                                lc = g_out_connections.data[j];
                                state = 1;
                                continue;
                            }
                        }
                        state = 2;
                        break;
                    case 1: // send over connection state
                        ez_Buffer* ebuffer = EZ_GENERATE_BUFFER(sizeof(Message)); //TODO: make this send only as much as needed from message
                        EZ_RECORD_BUFFER(ebuffer, qm.message);
                        if (EZ_CLIENT_SEND(lc.client, ebuffer)) {
                            breakout = TRUE;
                        } else {
                            for (size_t j = 0; j < g_out_connections.size; j++) {
                                if (lc.client == g_out_connections.data[j].client) {
                                    ARRLIST_LinkedClient_remove(&g_out_connections, j);
                                    break;
                                }
                            }
                            state = 2;
                        }
                        EZ_CLEAN_BUFFER(ebuffer);
                        break;
                    case 2: // attempt connection state
                        ez_Client* client = EZ_GENERATE_CLIENT();
                        if (EZ_CONNECT_CLIENT(client, ((Ipv4){{127, 0, 0, 1}}), SIGMSG_PORT)) {
                            lc.user = qm.user;
                            lc.client = client;
                            ARRLIST_LinkedClient_add(&g_out_connections, lc);
                            state = 1;
                            continue;
                        }
                        state = 3;
                        break;
                    case 3: // send to central
                        // TODO:
                        breakout = TRUE;
                        break;
                    default: break;
                }
                if (breakout) break;
            }
        }
        ARRLIST_QueuedMessage_clear(&g_send_queue);
    }
	return 0;
}

void InitializeNetwork() {
    EZ_INIT_NETWORK();

    // TODO: remove
    {
        User u1 = { 0 };
        strcpy(u1.name, "Cameron Lee");
        User u2 = { 0 };
        u2.unread = TRUE;
        strcpy(u2.name, "Localhost");
        User u3 = { 0 };
        strcpy(u3.name, "PoopFart");
        Message m = (Message){
            1,
            (Timestamp){ 25, 4, 12, 12, 45, 1, 0 },
            strlen("Steely Dan Loves Potatoes"),
            "Steely Dan Loves Potatoes\0"};
        ARRLIST_Message_add(&(u1.history), m);
        m = (Message){
            0,
            (Timestamp){ 25, 4, 12, 12, 47, 13, 0 },
            strlen("No he fucking doesn't?? You big weirdo??? Why would you even say that it is such an extremely outlandish opinion its just ridiculous realy."),
            "No he fucking doesn't?? You big weirdo??? Why would you even say that it is such an extremely outlandish opinion its just ridiculous realy.\0"};
        ARRLIST_Message_add(&(u1.history), m);
        m = (Message){
            0,
            (Timestamp){ 25, 4, 12, 12, 30, 12, 0 },
            strlen("I HATE YOU!!!"),
            "I HATE YOU!!!"};
        ARRLIST_Message_add(&(u2.history), m);
        m = (Message){
            1,
            (Timestamp){ 25, 4, 12, 12, 32, 15, 0 },
            strlen("What the hell brah where did this come from"),
            "What the hell brah where did this come from"};
        ARRLIST_Message_add(&(u2.history), m);
        m = (Message){
            1,
            (Timestamp){ 25, 4, 12, 3, 52, 1, 0 },
            strlen("hello hello hello hello"),
            "hello hello hello hello"};
        ARRLIST_Message_add(&(u3.history), m);
        m = (Message){
            1,
            (Timestamp){ 25, 4, 12, 4, 52, 1, 0 },
            strlen("Wake the fuck up dude"),
            "Wake the fuck up dude"};
        ARRLIST_Message_add(&(u3.history), m);
        ARRLIST_User_add(&(g_network.friends), u1);
        ARRLIST_User_add(&(g_network.friends), u2);
        ARRLIST_User_add(&(g_network.friends), u3);
    }

    EZ_CREATE_COND(g_send_condition)
    g_listener = EZ_GENERATE_SERVER();
    EZ_OPEN_SERVER(g_listener, SIGMSG_PORT);
    EZ_CREATE_THREAD(g_accept_thread, accept_thread, NULL);
    EZ_CREATE_THREAD(g_listen_thread, listen_thread, NULL);
    EZ_CREATE_THREAD(g_sender_thread, sender_thread, NULL);
}

Network* NetworkRef() {
    return &g_network;
}

void CleanNetwork() {
    EZ_CLOSE_SERVER(g_listener);
    EZ_CLEAN_SERVER(g_listener);
    for (size_t i = 0; i < g_network.friends.size; i++) {
        ARRLIST_Message_clear(&(g_network.friends.data[i].history));
    }
    ARRLIST_User_clear(&(g_network.friends));
    EZ_CLEAN_NETWORK();
}

void SendChat(User* user, const char* chat) {
    Message msg = { 0 };
    msg.size = strlen(chat);
    strncpy(msg.text, chat, MAX_MESSAGE_SIZE);
    time_t etime = time(NULL);
    struct tm* tm_info = gmtime(&etime);
    msg.time.year = tm_info->tm_year % 100;
    msg.time.month = tm_info->tm_mon + 1;
    msg.time.day = tm_info->tm_mday;
    msg.time.hour = tm_info->tm_hour;
    msg.time.minute = tm_info->tm_min;
    msg.time.second = tm_info->tm_sec;
    ARRLIST_Message_add(&(user->history), msg);
    ARRLIST_QueuedMessage_add(&g_send_queue, (QueuedMessage){ user, &(user->history.data[user->history.size - 1]) });
    EZ_SIGNAL_COND(g_send_condition);
}
