#include "network.h"
#include "core/app.h"
#include "util/animations.h"
#include "util/macros.h"
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
BOOL g_shutdown_network = FALSE;

EZ_THREAD_RETURN_TYPE accept_thread(EZ_THREAD_PARAMETER_TYPE params) {
    while(1) {
        ez_Connection* connection = EZ_SERVER_ACCEPT_TIMED(g_listener, 1000000);
        EZ_LOCK_MUTEX((*Lock()));
        if (g_shutdown_network) {
            EZ_RELEASE_MUTEX((*Lock()));
            return 0;
        }
        if (connection) ARRLIST_ez_ConnectionPtr_add(&g_in_connections, connection);
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
        if (g_shutdown_network) {
            EZ_RELEASE_MUTEX((*Lock()));
            return 0;
        }
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
                    Message msg = { 0 };
                    EZ_TRANSLATE_BUFFER(ebuffer, &msg);
                    for (size_t j = 0; j < g_network.friends.size; j++) {
                        if (uuideq(g_network.friends.data[j].id, msg.from)) {
                            ARRLIST_Message_add(&(g_network.friends.data[j].history), msg);
                            break;
                        }
                    }
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
        EZ_LOCK_MUTEX((*Lock()));
        EZ_WAIT_COND(g_send_condition, (*Lock()));
        if (g_shutdown_network) {
            EZ_RELEASE_MUTEX((*Lock()));
            return 0;
        }
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
                                break;
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
                            break;
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
        EZ_RELEASE_MUTEX((*Lock()));
    }
	return 0;
}

UUID GenerateUUID() {
    srand(time(NULL));
    UUID id = { 0 };
    for (int i = 0; i < 4; i++) {
        id.first = (id.first << 16) | (rand() & 0xFFFF);
    }
    for (int i = 0; i < 4; i++) {
        id.second = (id.second << 16) | (rand() & 0xFFFF);
    }
    return id;
}

void InitializeNetwork() {
    EZ_INIT_NETWORK();
    g_shutdown_network = FALSE;

    User localhost = { 0 };
    strcpy(localhost.name, "Me");
    ARRLIST_User_add(&(g_network.friends), localhost);

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
    EZ_LOCK_MUTEX((*Lock()));
    g_shutdown_network = TRUE;
    EZ_SIGNAL_COND(g_send_condition);
    EZ_RELEASE_MUTEX((*Lock()));
    EZ_WAIT_THREAD(g_accept_thread);
    EZ_WAIT_THREAD(g_listen_thread);
    EZ_WAIT_THREAD(g_sender_thread);
    EZ_CLOSE_SERVER(g_listener);
    EZ_CLEAN_SERVER(g_listener);
    for (size_t i = 0; i < g_in_connections.size; i++) {
        EZ_CLOSE_CONNECTION(g_in_connections.data[i]);
    }
    ARRLIST_ez_ConnectionPtr_clear(&g_in_connections);
    for (size_t i = 0; i < g_out_connections.size; i++) {
        EZ_DISCONNECT_CLIENT(g_out_connections.data[i].client);
    }
    ARRLIST_LinkedClient_clear(&g_out_connections);
    ARRLIST_QueuedMessage_clear(&g_send_queue);
    for (size_t i = 0; i < g_network.friends.size; i++) {
        ARRLIST_Message_clear(&(g_network.friends.data[i].history));
    }
    ARRLIST_User_clear(&(g_network.friends));
    EZ_CLEAN_NETWORK();
}

void SendChat(User* user, const char* chat) {
    Message msg = { 0 };
    msg.type = MESSAGE_PACKET;
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
    msg.from = g_network.id;
    msg.to = user->id;
    ARRLIST_Message_add(&(user->history), msg);
    ARRLIST_QueuedMessage_add(&g_send_queue, (QueuedMessage){ user, &(user->history.data[user->history.size - 1]) });
    EZ_SIGNAL_COND(g_send_condition);
}
