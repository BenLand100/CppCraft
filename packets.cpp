#include <iostream>
#include "packets.h"
#include "client.h"
#include "SDL_endian.h"

class SocketIO {
    public:
        bool working;
    private:
        TCPsocket socket;
        unsigned char buffer[16];
        inline void read(int len) {
            if (SDLNet_TCP_Recv(socket, buffer, len) != len) {
                working = false;
                for (int i = 0; i < 16; i++) buffer[i] = 0;
            }
        }
        
    public:
        SocketIO(TCPsocket _socket) : socket(_socket), working(true) { }
        inline unsigned char r_ubyte() {
            read(1);
            return buffer[0];
        }
        inline char r_byte() {
            read(1);
            return (char) buffer[0];
        }
        inline short r_short() {
            read(2);
            return SDL_SwapBE16(*(Uint16*)buffer);
        }
        inline int r_int() {
            read(4);
            return SDL_SwapBE32(*(Uint32*)buffer);
        }
        inline long long r_long() {
            read(8);
            return SDL_SwapBE64(*(Uint64*)buffer);
        }
        inline float r_float() {
            read(4);
            union { float f; Uint32 i; } swap;
            swap.i = SDL_Swap32(*(Uint32*)buffer);
            return swap.f;
        }
        inline double r_double() {
            read(8);
            union { double d; Uint64 l; } swap;
            swap.l = SDL_Swap64(*(Uint64*)buffer);
            return swap.d;
        }
        inline string8 r_string8() {
            string8 res;
            res.len = r_short();
            res.str = new char[res.len];
            if (SDLNet_TCP_Recv(socket, res.str, res.len) != res.len) {
                working = false;
            }
            return res;
        }
        inline string16 r_string16() {
            string16 res;
            res.len = r_short();
            res.str = new char[res.len*2];
            if (SDLNet_TCP_Recv(socket, res.str, res.len*2) != res.len*2) {
                working = false;
            }
            return res;
        }
        inline bool r_bool() {
            read(1);
            return buffer[0];
        }
        
        //Currently, this returns nothing useful
        inline metadata* r_metadata() {
            metadata *result = new metadata;
            unsigned char id;
            while ((id = r_ubyte()) != 0x7F && working) {
                switch (id >> 5) {
                    case 0:
                        r_byte();
                        break;
                    case 1:
                        r_short();
                        break;
                    case 2:
                        r_int();
                        break;
                    case 3:
                        r_float();
                        break;
                    case 4:
                        r_string16();
                        break;
                    case 5:
                        r_short();
                        r_byte();
                        r_short();
                        break;
                    case 6:
                        r_int();
                        r_int();
                        r_int();
                        break;
                    default:
                        std::cout << "Unknown metadata id!\n";
                        working = false;
                }
            }
            return result;
        }
        
        inline char* r_bytearray(int len) {
            char *array = new char[len];
            if (SDLNet_TCP_Recv(socket, array, len) != len) {
                working = false;
            }
            return array;
        }
        inline short* r_shortarray(int len) {
            short *array = new short[len];
            for (int i = 0; i < len; i++) {
                array[i] = r_short();
            }
            return array;
        }



};

int packets_thread(Client *client) {
    SocketIO io(client->socket);
    while (client->doPackets && io.working) {
        unsigned char pid = io.r_ubyte();
        if (!io.working) break;
        switch (pid) {
            case 0x00:{
                p_keep_alive *p = new p_keep_alive;
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x01:{
                p_login_request_stc *p = new p_login_request_stc;
                p->EntityID = io.r_int();
                p->Unknown = io.r_string16();
                p->MapSeed = io.r_long();
                p->Dimension = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x02:{
                p_handshake_stc *p = new p_handshake_stc;
                p->ConnectionHash = io.r_string16();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x03:{
                p_chat_message *p = new p_chat_message;
                p->Message = io.r_string16();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x04:{
                p_time_update *p = new p_time_update;
                p->Time = io.r_long();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x05:{
                p_entity_equipment *p = new p_entity_equipment;
                p->EntityID = io.r_int();
                p->Slot = io.r_short();
                p->ItemID = io.r_short();
                p->Damage = io.r_short();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x06:{
                p_spawn_position *p = new p_spawn_position;
                p->X = io.r_int();
                p->Y = io.r_int();
                p->Z = io.r_int();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x07:{
                p_use_entity *p = new p_use_entity;
                p->UserEID = io.r_int();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x08:{
                p_update_health *p = new p_update_health;
                p->Health = io.r_short();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x09:{
                p_respawn *p = new p_respawn;
                p->World = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x0A:{
                p_player *p = new p_player;
                p->OnGround = io.r_bool();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x0B:{
                p_player_position *p = new p_player_position;
                p->X = io.r_double();
                p->Y = io.r_double();
                p->Stance = io.r_double();
                p->Z = io.r_double();
                p->OnGround = io.r_bool();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x0C:{
                p_player_look *p = new p_player_look;
                p->Yaw = io.r_float();
                p->Pitch = io.r_float();
                p->OnGround = io.r_bool();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x0D:{
                p_player_position_and_look_stc *p = new p_player_position_and_look_stc;
                p->X = io.r_double();
                p->Stance = io.r_double();
                p->Y = io.r_double();
                p->Z = io.r_double();
                p->Yaw = io.r_float();
                p->Pitch = io.r_float();
                p->OnGround = io.r_bool();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x0E:{
                p_player_digging *p = new p_player_digging;
                p->Status = io.r_byte();
                p->X = io.r_int();
                p->Y = io.r_byte();
                p->Z = io.r_int();
                p->Face = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x0F:{
                p_player_block_placement *p = new p_player_block_placement;
                p->X = io.r_int();
                p->Y = io.r_byte();
                p->Z = io.r_int();
                p->Direction = io.r_byte();
                p->BlockID = io.r_short();
                if (p->BlockID >= 0) {
                    p->Amount = io.r_byte();
                    p->Damage = io.r_short();
                } else {
                    p->Amount = -1;
                    p->Damage = -1;
                }
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x10:{
                p_holding_change *p = new p_holding_change;
                p->SlotID = io.r_short();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x11:{
                p_use_bed *p = new p_use_bed;
                p->EntityID = io.r_int();
                p->InBed = io.r_byte();
                p->X = io.r_int();
                p->Y = io.r_byte();
                p->Z = io.r_int();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x12:{
                p_animate *p = new p_animate;
                p->EntityID = io.r_int();
                p->Animate = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x13:{
                p_act *p = new p_act;
                p->EntityID = io.r_int();
                p->Action = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x14:{
                p_spawn_player *p = new p_spawn_player;
                p->EID = io.r_int();
                p->Name = io.r_string16();
                p->X = io.r_int();
                p->Y = io.r_int();
                p->Z = io.r_int();
                p->Yaw = io.r_byte();
                p->Pitch = io.r_byte();
                p->CurrentItem = io.r_short();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x15:{
                p_pickup_spawn *p = new p_pickup_spawn;
                p->EntityID = io.r_int();
                p->Item = io.r_short();
                p->Count = io.r_byte();
                p->Damage = io.r_short();
                p->X = io.r_int();
                p->Y = io.r_int();
                p->Z = io.r_int();
                p->Yaw = io.r_byte();
                p->Pitch = io.r_byte();
                p->Roll = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x16:{
                p_collect_item *p = new p_collect_item;
                p->CollectedEID = io.r_int();
                p->CollectorEID = io.r_int();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x17:{
                p_addobject *p = new p_addobject;
                p->EntityID = io.r_int();
                p->Type = io.r_byte();
                p->X = io.r_int();
                p->Y = io.r_int();
                p->Z = io.r_int();
                p->Flag = io.r_int();
                if (p->Flag > 0) {
                    p->Xmap = io.r_short();
                    p->Ymap = io.r_short();
                    p->Zmap = io.r_short();
                } else {
                    p->Xmap = 0;
                    p->Ymap = 0;
                    p->Zmap = 0;
                }
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x18:{
                p_spawn_mob *p = new p_spawn_mob;
                p->EntityID = io.r_int();
                p->Type = io.r_byte();
                p->X = io.r_int();
                p->Y = io.r_int();
                p->Z = io.r_int();
                p->Yaw = io.r_byte();
                p->Pitch = io.r_byte();
                p->Metadata = io.r_metadata();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x19:{
                p_painting *p = new p_painting;
                p->EntityID = io.r_int();
                p->Title = io.r_string16();
                p->X = io.r_int();
                p->Y = io.r_int();
                p->Z = io.r_int();
                p->Direction = io.r_int();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x1B:{
                p_stance_update *p = new p_stance_update;
                p->A = io.r_float();
                p->B = io.r_float();
                p->C = io.r_float();
                p->D = io.r_float();
                p->E = io.r_bool();
                p->F = io.r_bool();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x1C:{
                p_entity_velocity *p = new p_entity_velocity;
                p->EntityID = io.r_int();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x1D:{
                p_destroy_entity *p = new p_destroy_entity;
                p->EntityID = io.r_int();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x1E:{
                p_entity *p = new p_entity;
                p->EntityID = io.r_int();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x1F:{
                p_entity_relative_move *p = new p_entity_relative_move;
                p->EntityID = io.r_int();
                p->dX = io.r_byte();
                p->dY = io.r_byte();
                p->dZ = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x20:{
                p_entity_look *p = new p_entity_look;
                p->EntityID = io.r_int();
                p->Yaw = io.r_byte();
                p->Pitch = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x21:{
                p_entity_look_and_relative_move *p = new p_entity_look_and_relative_move;
                p->EntityID = io.r_int();
                p->dX = io.r_byte();
                p->dY = io.r_byte();
                p->dZ = io.r_byte();
                p->Yaw = io.r_byte();
                p->Pitch = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x22:{
                p_entity_teleport *p = new p_entity_teleport;
                p->EntityID = io.r_int();
                p->X = io.r_int();
                p->Y = io.r_int();
                p->Z = io.r_int();
                p->Yaw = io.r_byte();
                p->Pitch = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x26:{
                p_entity_status *p = new p_entity_status;
                p->EntityID = io.r_int();
                p->Status = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x27:{
                p_attach_entity *p = new p_attach_entity;
                p->EntityID = io.r_int();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x28:{
                p_entity_metadata *p = new p_entity_metadata;
                p->EntityID = io.r_int();
                p->Metadata = io.r_metadata();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x32:{
                p_prechunk *p = new p_prechunk;
                p->X = io.r_int();
                p->Z = io.r_int();
                p->Mode = io.r_bool();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x33:{
                p_map_chunk *p = new p_map_chunk;
                p->X = io.r_int();
                p->Y = io.r_short();
                p->Z = io.r_int();
                p->SizeX = io.r_byte();
                p->SizeY = io.r_byte();
                p->SizeZ = io.r_byte();
                p->CompressedSize = io.r_int();
                p->CompressedData = io.r_bytearray(p->CompressedSize);
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x34:{
                p_multi_block_change *p = new p_multi_block_change;
                p->ChunkX = io.r_int();
                p->ChunkZ = io.r_int();
                p->ArraySize = io.r_short();
                p->CoordinateArray = io.r_shortarray(p->ArraySize);
                p->TypeArray = io.r_bytearray(p->ArraySize);
                p->MetadataArray = io.r_bytearray(p->ArraySize);
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x35:{
                p_block_change *p = new p_block_change;
                p->X = io.r_int();
                p->Y = io.r_byte();
                p->Z = io.r_int();
                p->Type = io.r_byte();
                p->Metadata = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x36:{
                p_block_action *p = new p_block_action;
                p->X = io.r_int();
                p->Y = io.r_short();
                p->Z = io.r_int();
                p->DataA  = io.r_byte();
                p->DataB = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            /*case 0x3C:{
                p_explosion *p = new p_explosion;
                p->X = io.r_double();
                p->Y = io.r_double();
                p->Z = io.r_double();
                p->Unknown = io.r_float();
                p->RecordCount = io.r_int();
                p->Records = io.r_(byte, byte, byte) × count();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;*/
            case 0x3D:{
                p_sound_effect *p = new p_sound_effect;
                p->EffectID = io.r_int();
                p->X = io.r_int();
                p->Y = io.r_byte();
                p->Z = io.r_int();
                p->SoundData = io.r_int();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x46:{
                p_new_state *p = new p_new_state;
                p->Reason = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x47:{
                p_thunderbolt *p = new p_thunderbolt;
                p->EntityID = io.r_int();
                p->Unknown = io.r_bool();
                p->X = io.r_int();
                p->Y = io.r_int();
                p->Z = io.r_int();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x64:{
                p_open_window *p = new p_open_window;
                p->WindowID = io.r_byte();
                p->InventoryType = io.r_byte();
                p->WindowTitle = io.r_string8();
                p->NumberOfSlots = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x65:{
                p_close_window *p = new p_close_window;
                p->WindowID = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x66:{
                p_window_click *p = new p_window_click;
                p->WindowID = io.r_byte();
                p->Slot = io.r_short();
                p->RightClick = io.r_byte();
                p->ActionNumber = io.r_short();
                p->Shift = io.r_bool();
                p->ItemID = io.r_short();
                if (p->ItemID != -1) {
                    p->ItemCount = io.r_byte();
                    p->ItemUses = io.r_short();
                } else {
                    p->ItemCount = -1;
                    p->ItemUses = -1;
                }
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x67:{
                p_set_slot *p = new p_set_slot;
                p->WindowID = io.r_byte();
                p->Slot = io.r_short();
                p->ItemID = io.r_short();
                p->ItemCount = io.r_byte();
                p->ItemUses = io.r_short();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            /*case 0x68:{
                p_window_items! *p = new p_window_items!;
                p->WindowID = io.r_byte();
                p->Count = io.r_short();
                p->Payload = io.r_…();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;*/
            case 0x69:{
                p_update_progress_bar *p = new p_update_progress_bar;
                p->WindowID = io.r_byte();
                p->ProgressBar = io.r_short();
                p->Value = io.r_short();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x6A:{
                p_transaction *p = new p_transaction;
                p->WindowID = io.r_byte();
                p->ActionNumber = io.r_short();
                p->Accepted = io.r_bool();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x82:{
                p_update_sign *p = new p_update_sign;
                p->X = io.r_int();
                p->Y = io.r_short();
                p->Z = io.r_int();
                p->Text1 = io.r_string16();
                p->Text2 = io.r_string16();
                p->Text3 = io.r_string16();
                p->Text4 = io.r_string16();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0x83:{
                p_map_data *p = new p_map_data;
                p->UnknownA = io.r_short();
                p->UnknownB = io.r_short();
                p->TextLength = io.r_ubyte();
                p->Text = io.r_bytearray(p->TextLength);
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0xC8:{
                p_increment_statistic *p = new p_increment_statistic;
                p->StatisticID = io.r_int();
                p->Amount = io.r_byte();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            case 0xFF:{
                p_kick *p = new p_kick;
                p->Message = io.r_string16();
                if (!io.working) break;
                client->packet((p_generic*)p);
            } break;
            default:
                std::cout << "Unhandled packet: 0x" << std::hex << (int)pid << '\n';
                client->disconnect();
                return 1;
        }
    }
    if (!io.working) {
        std::cout << "Socket Error\n";
        client->disconnect();
    }
    return 0;
}

void free_packet(p_generic *p) {
    switch (p->id) {
        case 0x01:
            delete ((p_login_request_stc*)p)->Unknown.str;
            break;
        case 0x02:
            delete ((p_handshake_stc*)p)->ConnectionHash.str;
            break;
        case 0x03:
            delete ((p_chat_message*)p)->Message.str;
            break;
        case 0x14:
            delete ((p_spawn_player*)p)->Name.str;
            break;
        case 0x18:
            delete ((p_spawn_mob*)p)->Metadata;
            break;
        case 0x19:
            delete ((p_painting*)p)->Title.str;
            break;
        case 0x28:
            delete ((p_entity_metadata*)p)->Metadata;
            break;
        case 0x33:
            delete ((p_map_chunk*)p)->CompressedData;
            break;
        case 0x34:
            delete ((p_multi_block_change*)p)->CoordinateArray;
            delete ((p_multi_block_change*)p)->TypeArray;
            delete ((p_multi_block_change*)p)->MetadataArray;
            break;
        case 0x64:
            delete ((p_open_window*)p)->WindowTitle.str;
            break;
        case 0x82:
            delete ((p_update_sign*)p)->Text1.str;
            delete ((p_update_sign*)p)->Text2.str;
            delete ((p_update_sign*)p)->Text3.str;
            delete ((p_update_sign*)p)->Text4.str;
            break;
        case 0x83:
            delete ((p_map_data*)p)->Text;
            break;
        case 0xFF:
            delete ((p_kick*)p)->Message.str;
            break;
    }
    delete p;
}

bool write_packet(TCPsocket socket, p_generic *p) {
    SocketIO io(socket);
    unsigned char pid = io.r_ubyte();
    switch (pid) {
    
    }
    return io.working;
}

