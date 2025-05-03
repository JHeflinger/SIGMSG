#include "network.h"
#include "core/app.h"
#include "util/animations.h"
#include "util/macros.h"
#include <easynet.h>
#include <time.h>
#include <easythreads.h>

#define FORWARD_LOCAL_MESSAGES FALSE

IMPL_ARRLIST(Message);
IMPL_ARRLIST(User);
IMPL_ARRLIST(LinkedClient);
IMPL_ARRLIST(QueuedMessage);

Ipv4 CENTRAL_PEER_IP = {{170, 9, 247, 131}};
Network g_network = { 0 };
ez_Server* g_server = NULL;
EZ_THREAD g_handler_thread;
ARRLIST_LinkedClient g_out_connections = { 0 };
ARRLIST_QueuedMessage g_send_queue = { 0 };
BOOL g_shutdown_network = FALSE;
Destination g_translated_destination = { 0 };

void throw_punch(Destination destination) {
    ez_Buffer* buffer = EZ_GENERATE_BUFFER(sizeof(FistPacket));
    FistPacket fp = { FIST_PACKET };
    EZ_RECORD_BUFFER(buffer, &fp);
    EZ_SERVER_THROW(g_server, destination, buffer);
    EZ_CLEAN_BUFFER(buffer);
}

void handle_message_packet(Destination destination, ez_Buffer* buffer) {
    if (destination.port == 0 || buffer->current_length < sizeof(Header)) return;
    Header header;
    memcpy(&header, buffer->bytes, sizeof(Header));
    switch (header) {
        case MESSAGE_PACKET:
            Message msg = { 0 };
            EZ_TRANSLATE_BUFFER(buffer, &msg);
            if (msg.type != MESSAGE_PACKET) return;
            ez_Buffer* b = EZ_GENERATE_BUFFER(sizeof(AckPacket));
            AckPacket ack = { ACK_PACKET, {msg.id.first, msg.id.second}};
            EZ_RECORD_BUFFER(b, &ack);
            EZ_SERVER_THROW(g_server, destination, b);
            EZ_LOCK_MUTEX((*Lock()));
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
            EZ_CLEAN_BUFFER(b);
            return;
        case PUNCH_PACKET:
            if (buffer->current_length != sizeof(PunchPacket)) {
                EZ_WARN("Wrong punch packet size");
                return;
            }
            PunchPacket p = { 0 };
			EZ_TRANSLATE_BUFFER(buffer, &p);
            throw_punch(p.destination);
            return;
        default: return;
    }
}

BOOL handle_connect_return_packet(Destination destination, ez_Buffer* buffer, PeerPacket* packet) {
    if (destination.port == 0 || buffer->current_length < sizeof(Header)) return FALSE;
    Header header;
    memcpy(&header, buffer->bytes, sizeof(Header));
    switch (header) {
        case MESSAGE_PACKET:
            if (buffer->current_length != sizeof(Message)) {
                EZ_WARN("Wrong message packet size");
                return FALSE;
            }
            handle_message_packet(destination, buffer);
            return FALSE;
        case PEER_PACKET:
            if (buffer->current_length != sizeof(PeerPacket)) {
                EZ_WARN("Wrong peer packet size");
                return FALSE;
            }
            EZ_TRANSLATE_BUFFER(buffer, packet);
            return TRUE;
        case ACK_PACKET:
            if (buffer->current_length != sizeof(AckPacket)) {
                EZ_WARN("Wrong ack packet size");
                return FALSE;
            }
            return FALSE;
        case PUNCH_PACKET:
            if (buffer->current_length != sizeof(PunchPacket)) {
                EZ_WARN("Wrong punch packet size");
                return FALSE;
            }
            PunchPacket p = { 0 };
			EZ_TRANSLATE_BUFFER(buffer, &p);
            throw_punch(p.destination);
            return FALSE;
        case FIST_PACKET:
            if (buffer->current_length != sizeof(FistPacket)) {
                EZ_WARN("Wrong fist packet size");
                return FALSE;
            }
            return FALSE;
        default: break;
    }
    return FALSE;
}

BOOL handle_send_return_packet(Destination destination, ez_Buffer* buffer, QueuedMessage qm) {
    if (destination.port == 0 || buffer->current_length < sizeof(Header)) return FALSE;
    Header header;
    memcpy(&header, buffer->bytes, sizeof(Header));
    switch (header) {
        case MESSAGE_PACKET:
            if (buffer->current_length != sizeof(Message)) {
                EZ_WARN("Wrong message packet size");
                return FALSE;
            }
            handle_message_packet(destination, buffer);
            return FALSE;
        case PEER_PACKET:
            if (buffer->current_length != sizeof(PeerPacket)) {
                EZ_WARN("Wrong peer packet size");
                return FALSE;
            }
            return FALSE;
        case ACK_PACKET:
            if (buffer->current_length != sizeof(AckPacket)) {
                EZ_WARN("Wrong ack packet size");
                return FALSE;
            }
            AckPacket packet;
            EZ_TRANSLATE_BUFFER(buffer, &packet);
            if (uuideq(packet.id, qm.message->id)) {
                return TRUE;
            }
            return FALSE;
        case PUNCH_PACKET:
            if (buffer->current_length != sizeof(PunchPacket)) {
                EZ_WARN("Wrong punch packet size");
                return FALSE;
            }
            PunchPacket p = { 0 };
			EZ_TRANSLATE_BUFFER(buffer, &p);
            throw_punch(p.destination);
            return FALSE;
        case FIST_PACKET:
            if (buffer->current_length != sizeof(FistPacket)) {
                EZ_WARN("Wrong fist packet size");
                return FALSE;
            }
            return FALSE;
        default: break;
    }
    return FALSE;
}

EZ_THREAD_RETURN_TYPE network_thread(EZ_THREAD_PARAMETER_TYPE params) {
    ez_Buffer* ebuffer = EZ_GENERATE_BUFFER(sizeof(Message));
    Destination punch_dest;
    punch_dest.port = SIGMSG_PORT;
    punch_dest.address = CENTRAL_PEER_IP;
    while (1) {
        EZ_LOCK_MUTEX((*Lock()));
        if (g_shutdown_network) {
            EZ_RELEASE_MUTEX((*Lock()));
            EZ_CLEAN_BUFFER(ebuffer);
            return 0;
        }
        EZ_RELEASE_MUTEX((*Lock()));
        if (!g_network.online) {
            if (g_network.id.first == 0 && g_network.id.second == 0) {
                Wait(100);
                continue;
            }
            RegisterPacket reg = { 0 };
            reg.type = REGISTER_PACKET;
            reg.peer = g_network.id;
            reg.private_dest.port = SIGMSG_PORT;
            reg.private_dest.address = EZ_GET_MY_IP();
            EZ_RECORD_BUFFER(ebuffer, &reg);
            EZ_SERVER_THROW(g_server, punch_dest, ebuffer);
            Destination back = EZ_SERVER_RECIEVE_FROM_TIMED(g_server, ebuffer, 100000);
            if (back.port != 0 && ebuffer->current_length == sizeof(TranslatePacket)) {
                TranslatePacket ack;
                EZ_TRANSLATE_BUFFER(ebuffer, &ack);
                if (ack.type == TRANSLATE_PACKET) {
                    EZ_LOCK_MUTEX((*Lock()));
                    g_network.online = TRUE;
                    g_translated_destination = ack.translation;
                    AppState curr_state = GetState();
                    Event e = { 0 };
                    e.recieve = TRUE;
                    curr_state(e);
                    EZ_RELEASE_MUTEX((*Lock()));
                    continue;
                }
            }
        } else {
            // send queued messages
            EZ_LOCK_MUTEX((*Lock()));
            ARRLIST_QueuedMessage qm_copy = { 0 };
            for (size_t i = 0; i < g_send_queue.size; i++) ARRLIST_QueuedMessage_add(&qm_copy, g_send_queue.data[i]);
            ARRLIST_QueuedMessage_clear(&g_send_queue);
            EZ_RELEASE_MUTEX((*Lock()));
            for (size_t i = 0; i < qm_copy.size; i++) {
                QueuedMessage qm = qm_copy.data[i];
                int state = 0;
                LinkedClient lc = { 0 };
			    int failsafe = 0;
                while (1) { // state machine
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
                            for (int j = 0; j < MAX_SEND_ATTEMPTS; j++) {
                                //printdest(lc.destination);
                                EZ_SERVER_THROW(g_server, lc.destination, ebuffer);
                                Destination dest = EZ_SERVER_RECIEVE_FROM_TIMED(g_server, ackbuffer, 100000);
                                while (dest.port != 0) {
                                    if (handle_send_return_packet(dest, ackbuffer, qm)) {
                                        breakout = TRUE;
                                        break;
                                    }
                                    dest = EZ_SERVER_RECIEVE_FROM_TIMED(g_server, ackbuffer, 100000);
                                }
                                if (breakout) break;
                            }
                            if (!breakout) {
                                for (size_t j = 0; j < g_out_connections.size; j++) {
                                    if (lc.user == g_out_connections.data[j].user) {
                                        ARRLIST_LinkedClient_remove(&g_out_connections, j);
                                        break;
                                    }
                                }
                                state = 2;
                                failsafe++;
                            }
                            EZ_CLEAN_BUFFER(ebuffer);
                            EZ_CLEAN_BUFFER(ackbuffer);
                            break;
                        case 2: // attempt connection state
                            if (FORWARD_LOCAL_MESSAGES && uuideq(qm.message->to, g_network.id)) {
                                lc.destination.port = SIGMSG_PORT;
                                lc.user = qm.user;
                                lc.destination.address = (Ipv4){{127,0,0,1}};
                                state = 1;
                            } else {
                                ez_Buffer* cnbuffer = EZ_GENERATE_BUFFER(sizeof(Message));
                                ez_Buffer* pbuffer = EZ_GENERATE_BUFFER(sizeof(Message));
                                ConnectPacket cnp;
                                cnp.type = CONNECT_PACKET;
                                cnp.to = qm.message->to;
                                EZ_RECORD_BUFFER(cnbuffer, &cnp);
                                state = 3;
                                for (int j = 0; j < MAX_SEND_ATTEMPTS; j++) {
                                    EZ_SERVER_THROW(g_server, punch_dest, cnbuffer);
                                    Destination dest = EZ_SERVER_RECIEVE_FROM_TIMED(g_server, pbuffer, 100000);
                                    while (dest.port != 0) {
                                        PeerPacket packet;
                                        if (handle_connect_return_packet(dest, pbuffer, &packet)) {
                                            if (packet.destination.port != 0) {
                                                if (ipeq(packet.destination.address, g_translated_destination.address))
                                                    lc.destination = packet.private_dest;
                                                else
                                                    lc.destination = packet.destination;
                                                lc.user = qm.user;
                                                ARRLIST_LinkedClient_add(&g_out_connections, lc);
                                                state = 1;
                                                break;
                                            } else {
                                                state = 3;
                                                break;
                                            }
                                        }
                                        dest = EZ_SERVER_RECIEVE_FROM_TIMED(g_server, pbuffer, 100000);
                                    }
                                    if (state == 1) break;
                                }
                                EZ_CLEAN_BUFFER(cnbuffer);
                                EZ_CLEAN_BUFFER(pbuffer);
                            }
                            break;
                        case 3: // send to central
                            EZ_WARN("FAILURE FAILURE NOT IMPLEMENTED STORE MESSAGES YET");
                            breakout = TRUE;
                            break;
                        default: break;
                    }
                    if (failsafe > MAX_SEND_ATTEMPTS) state = 3;
                    if (breakout) break;
                }
            }
            ARRLIST_QueuedMessage_clear(&qm_copy);

            // handle received messages
            Destination dest = EZ_SERVER_RECIEVE_FROM_TIMED(g_server, ebuffer, 100000);
            handle_message_packet(dest, ebuffer);
        }
    }
    EZ_CLEAN_BUFFER(ebuffer);
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
    g_network.online = FALSE;

    User localhost = { 0 };
    strcpy(localhost.name, "Me");
    ARRLIST_User_add(&(g_network.friends), localhost);

    g_server = EZ_GENERATE_SERVER();
    g_server->udp = TRUE;
    EZ_OPEN_SERVER(g_server, SIGMSG_PORT);
    EZ_CREATE_THREAD(g_handler_thread, network_thread, NULL);
}

Network* NetworkRef() {
    return &g_network;
}

void CleanNetwork() {
    EZ_LOCK_MUTEX((*Lock()));
    g_shutdown_network = TRUE;
    EZ_RELEASE_MUTEX((*Lock()));
    EZ_WAIT_THREAD(g_handler_thread);
    EZ_CLOSE_SERVER(g_server);
    EZ_CLEAN_SERVER(g_server);
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
}
