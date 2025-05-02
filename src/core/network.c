#include "network.h"
#include "core/app.h"
#include "util/animations.h"
#include "util/macros.h"
#include <easynet.h>
#include <time.h>
#include <easythreads.h>

IMPL_ARRLIST(Message);
IMPL_ARRLIST(User);
IMPL_ARRLIST(LinkedClient);
IMPL_ARRLIST(QueuedMessage);

Ipv4 CENTRAL_PEER_IP = {{170, 9, 247, 131}};
Network g_network = { 0 };
EZ_COND g_send_condition;
EZ_THREAD g_accept_thread;
EZ_THREAD g_listen_thread;
EZ_THREAD g_sender_thread;
ez_Server* g_listener = NULL;
ez_Server* g_thrower = NULL;
ARRLIST_LinkedClient g_out_connections = { 0 };
ARRLIST_QueuedMessage g_send_queue = { 0 };
BOOL g_shutdown_network = FALSE;

EZ_THREAD_RETURN_TYPE listen_thread(EZ_THREAD_PARAMETER_TYPE params) {
    ez_Buffer* ebuffer = EZ_GENERATE_BUFFER(sizeof(Message));
    while (1) {
        EZ_LOCK_MUTEX((*Lock()));
        if (g_shutdown_network) {
            EZ_CLEAN_BUFFER(ebuffer);
            EZ_RELEASE_MUTEX((*Lock()));
            return 0;
        }
        EZ_RELEASE_MUTEX((*Lock()));
        if (g_network.id.first == 0 && g_network.id.second == 0) {
            Wait(100);
            continue;
        }
        RegisterPacket reg = { 0 };
        reg.type = REGISTER_PACKET;
        reg.peer = g_network.id;
        EZ_RECORD_BUFFER(ebuffer, &reg);
        Destination punch_dest;
        punch_dest.port = SIGMSG_PORT;
        punch_dest.address = CENTRAL_PEER_IP;
        EZ_SERVER_THROW(g_listener, punch_dest, ebuffer);
        Destination back = EZ_SERVER_RECIEVE_FROM_TIMED(g_listener, ebuffer, 100000);
        if (back.port != 0) {
            EZ_LOCK_MUTEX((*Lock()));
            g_network.online = TRUE;
            AppState curr_state = GetState();
            Event e = { 0 };
            e.recieve = TRUE;
            curr_state(e);
            EZ_RELEASE_MUTEX((*Lock()));
            break;
        }
    }
    while (1) {
        EZ_LOCK_MUTEX((*Lock()));
        if (g_shutdown_network) {
            EZ_CLEAN_BUFFER(ebuffer);
            EZ_RELEASE_MUTEX((*Lock()));
            return 0;
        }
        EZ_RELEASE_MUTEX((*Lock()));
        Destination destination = EZ_SERVER_RECIEVE_FROM_TIMED(g_listener, ebuffer, 100000);
        if (destination.port != 0) {
            EZ_LOCK_MUTEX((*Lock()));
            Message msg = { 0 };
            EZ_TRANSLATE_BUFFER(ebuffer, &msg);
            AckPacket ack = { ACK_PACKET, {msg.id.first, msg.id.second}};
            EZ_RECORD_BUFFER(ebuffer, &ack);
            EZ_SERVER_THROW(g_listener, destination, ebuffer);
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
            EZ_RELEASE_MUTEX((*Lock()));
        }
    }
    EZ_CLEAN_BUFFER(ebuffer);
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
            ConnectPacket cnp;
            cnp.type = CONNECT_PACKET;
            Destination punch_dest;
            punch_dest.port = SIGMSG_PORT;
            punch_dest.address = CENTRAL_PEER_IP;
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
                        ez_Buffer* ebuffer = EZ_GENERATE_BUFFER(sizeof(Message));
                        ez_Buffer* ackbuffer = EZ_GENERATE_BUFFER(sizeof(Message));
                        EZ_RECORD_BUFFER(ebuffer, qm.message);
                        EZ_RELEASE_MUTEX((*Lock()));
                        for (int j = 0; j < MAX_SEND_ATTEMPTS; j++) {
                            EZ_SERVER_THROW(g_thrower, lc.destination, ebuffer);
                            Destination dest = EZ_SERVER_RECIEVE_FROM_TIMED(g_thrower, ackbuffer, 100000);
                            if (dest.port != 0 && ackbuffer->current_length == sizeof(AckPacket)) {
                                AckPacket packet;
                                memcpy(&packet, ackbuffer->bytes, ackbuffer->current_length);
                                if (packet.type == ACK_PACKET && uuideq(packet.id, qm.message->id)) {
                                    breakout = TRUE;
                                    break;
                                }
                            }
                        }
                        EZ_LOCK_MUTEX((*Lock()));
                        if (!breakout) {
                            for (size_t j = 0; j < g_out_connections.size; j++) {
                                if (lc.user == g_out_connections.data[j].user) {
                                    ARRLIST_LinkedClient_remove(&g_out_connections, j);
                                    break;
                                }
                            }
                            state = 2;
                        }
                        EZ_CLEAN_BUFFER(ebuffer);
                        EZ_CLEAN_BUFFER(ackbuffer);
                        break;
                    case 2: // attempt connection state
                        EZ_RELEASE_MUTEX((*Lock()));
                        if (uuideq(qm.message->to, g_network.id)) {
                            lc.destination.port = SIGMSG_PORT;
                            lc.user = qm.user;
                            lc.destination.address = (Ipv4){{127,0,0,1}};
                            state = 1;
                        } else {
                            ez_Buffer* cnbuffer = EZ_GENERATE_BUFFER(sizeof(Message));
                            ez_Buffer* pbuffer = EZ_GENERATE_BUFFER(sizeof(Message));
                            cnp.to = qm.message->to;
                            EZ_RECORD_BUFFER(cnbuffer, &cnp);
                            state = 3;
                            for (int j = 0; j < MAX_SEND_ATTEMPTS; j++) {
                                EZ_SERVER_THROW(g_thrower, punch_dest, cnbuffer);
                                Destination dest = EZ_SERVER_RECIEVE_FROM_TIMED(g_thrower, pbuffer, 100000);
                                if (dest.port != 0 && pbuffer->current_length == sizeof(PeerPacket)) {
                                    PeerPacket packet;
                                    memcpy(&packet, pbuffer->bytes, pbuffer->current_length);
                                    if (packet.type == PEER_PACKET && packet.destination.port != 0) {
                                        lc.destination = packet.destination;
                                        lc.user = qm.user;
                                        ARRLIST_LinkedClient_add(&g_out_connections, lc);
                                        state = 1;
                                        break;
                                    }
                                }
                            }
                            EZ_CLEAN_BUFFER(cnbuffer);
                            EZ_CLEAN_BUFFER(pbuffer);
                        }
                        EZ_LOCK_MUTEX((*Lock()));
                        break;
                    case 3: // send to central
                        EZ_WARN("FAILURE FAILURE NOT IMPLEMENTED STORE MESSAGES YET");
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
    g_thrower = EZ_GENERATE_SERVER();
    g_listener->udp = TRUE;
    g_thrower->udp = TRUE;
    EZ_OPEN_SERVER(g_listener, SIGMSG_PORT);
    EZ_OPEN_SERVER(g_thrower, SIGMSG_PORT + 1);
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
    EZ_CLOSE_SERVER(g_thrower);
    EZ_CLEAN_SERVER(g_thrower);
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
