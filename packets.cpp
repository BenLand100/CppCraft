/**
 *  Copyright 2011 by Benjamin J. Land (a.k.a. BenLand100)
 *
 *  This file is part of the CppCraft.
 *
 *  CppCraft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CppCraft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CppCraft. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include "packets.h"
#include "client.h"
#include "SDL_endian.h"

inline int ucs2_to_utf8(int ucs2, char *utf8) {
    if (ucs2 < 0x80) {
        utf8[0] = ucs2;
        return 1;
    }
    if (ucs2 >= 0x80  && ucs2 < 0x800) {
        utf8[0] = (ucs2 >> 6)   | 0xC0;
        utf8[1] = (ucs2 & 0x3F) | 0x80;
        return 2;
    }
    if (ucs2 >= 0x800 && ucs2 < 0xFFFF) {
        utf8[0] = ((ucs2 >> 12)       ) | 0xE0;
        utf8[1] = ((ucs2 >> 6 ) & 0x3F) | 0x80;
        utf8[2] = ((ucs2      ) & 0x3F) | 0x80;
        return 3;
    }
    return -1;
}

inline int utf8_to_ucs2(char *input, char **end_ptr) {
    *end_ptr = input;
    if (input[0] == 0)
        return 0;
    if (input[0] < 0x80) {
        *end_ptr = input + 1;
        return input[0];
    }
    if ((input[0] & 0xE0) == 0xE0) {
        if (input[1] == 0 || input[2] == 0)
            return -1;
        *end_ptr = input + 3;
        return (input[0] & 0x0F)<<12 | (input[1] & 0x3F)<<6 | (input[2] & 0x3F);
    }
    if ((input[0] & 0xC0) == 0xC0) {
        if (input[1] == 0)
            return -1;
        *end_ptr = input + 2;
        return (input[0] & 0x1F)<<6  | (input[1] & 0x3F);
    }
    return -1;
}

class SocketIO {
    public:
        bool working;
    private:
        TCPsocket socket;
        unsigned char buffer[16];
        inline void read(unsigned char *buffer, int len) {
            int read = 0, cur;
            for (;;) { //Supposedly SDL should wait for the whole length, in practice however...
                cur = SDLNet_TCP_Recv(socket, &buffer[read], len);
                if (cur == -1 || cur == 0) {
                    for (int i = 0; i < 16; i++) buffer[i] = 0;
                    working = false;
                    return;
                }
                len -= cur;
                if (len == 0) return;
                std::cout << std::dec << cur << " bytes read - readunder\n";
                read += cur;
            }
        }
        inline void read(int len) {
            read(buffer,len);
        }
        inline void write(int len) {
            if (SDLNet_TCP_Send(socket, (void*)buffer, len) != len) {
                working = false;
            }
        }
        
    public:
        SocketIO(TCPsocket _socket) : socket(_socket), working(true) { }
        inline unsigned char r_ubyte() {
            read(1);
            return buffer[0];
        }
        inline void w_ubyte(unsigned char b) {
            buffer[0] = b;
            write(1);
        }
        inline char r_byte() {
            read(1);
            return (char) buffer[0];
        }
        inline void w_byte(char b) {
            buffer[0] = (unsigned char)b;
            write(1);
        }
        inline short r_short() {
            read(2);
            return SDL_SwapBE16(*(Uint16*)buffer);
        }
        inline void w_short(short s) {
            *(short*)buffer = SDL_SwapBE16(s);
            write(2);
        }
        inline int r_int() {
            read(4);
            return SDL_SwapBE32(*(Uint32*)buffer);
        }
        inline void w_int(int i) {
            *(int*)buffer = SDL_SwapBE32(i);
            write(4);
        }
        inline long long r_long() {
            read(8);
            return SDL_SwapBE64(*(Uint64*)buffer);
        }
        inline void w_long(long long l) {
            *(long long*)buffer = SDL_SwapBE64(l);
            write(8);
        }
        inline float r_float() {
            read(4);
            union { float f; Uint32 i; } swap;
            swap.i = SDL_SwapBE32(*(Uint32*)buffer);
            return swap.f;
        }
        inline void w_float(float f) {
            union { float f; Uint32 i; } swap;
            swap.f = f;
            *(int*)buffer = SDL_SwapBE32(swap.i);
            write(4);
        }
        inline double r_double() {
            read(8);
            union { double d; Uint64 l; } swap;
            swap.l = SDL_SwapBE64(*(Uint64*)buffer);
            return swap.d;
        }
        inline void w_double(double d) {
            union { double d; Uint64 l; } swap;
            swap.d = d;
            *(long long*)buffer = SDL_SwapBE64(swap.l);
            write(8);
        }
        inline bool r_bool() {
            read(1);
            return buffer[0];
        }
        inline void w_bool(bool flag) {
            buffer[0] = flag ? '\1' : '\0';
            write(1);
        }
        
        
        inline string8 r_string8() {
            int len = r_short();
            string8 str = new char[len+1];
            if (SDLNet_TCP_Recv(socket, str, len) != len) {
                working = false;
            }
            str[len] = '\0';
            return str;
        }
        inline void w_string8(string8 str) {
            int len = strlen(str);
            w_short(len);
            if (SDLNet_TCP_Send(socket, str, len) != len) {
                working = false;
            }
        }
        inline string16 r_string16() {
            string16 res;
            int len = r_short();
            char *ucs2 = new char[len*2];
            string16 str = new char[len*4+1]; //never allow an overflow
            if (SDLNet_TCP_Recv(socket, ucs2, len*2) != len*2) {
                working = false;
                return NULL;
            }
            char *wchars = ucs2;
            char *utf8chars = str;
            for (int i = 0; i < len; i++, wchars += 2) {
                int c = SDL_SwapBE16(*(Uint16*)wchars);
                int s = ucs2_to_utf8(c,utf8chars);
                if (s == -1) {
                    std::cout << "UFT-8 conversion error\n";
                    working = false;
                    return NULL;
                }   
                utf8chars += s;
            }
            *utf8chars = '\0';
            delete ucs2;
            return str;
        }
        inline void w_string16(string16 str) {
            int len = strlen(str);
            short *ucs2 = new short[len];
            short *chars = ucs2;
            while (true) {
                int c = utf8_to_ucs2(str,&str);
                if (c <= 0) break;
                *(chars++) = SDL_SwapBE16(c);
            }
            int nchars = chars - ucs2;
            w_short(nchars);
            if (SDLNet_TCP_Send(socket, ucs2, nchars*2) != nchars*2) {
                working = false;
            }
            delete ucs2;
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
        //This is even more useless than the reader
        inline void w_metadata(metadata *data) {
            
        }
        
        //Currently, this returns nothing useful
        inline item* r_itemarray(int len) {
            item *result = new item[len];
            for (int i = 0; i < len; i++) {
                result[i].itemid = r_short();
                if (result[i].itemid != -1) {
                    result[i].count = r_byte();
                    result[i].uses = r_short();
                } else {
                    result[i].count = 0;
                    result[i].uses = 0;
                }
            }
            return result;
        }
        //This is even more useless than the reader
        inline void w_itemarray(item *data, int len) {
            for (int i = 0; i < len; i++) {
                w_short(data[i].itemid);
                if (data[i].itemid != -1) {
                     w_byte(data[i].count);
                     w_short(data[i].uses);
                }
            }
        }
        
        inline char* r_bytearray(int len) {
            char *array = new char[len];
            read((unsigned char*)array, len);
            return array;
        }
        inline void w_bytearray(char* array, int len) {
            if (SDLNet_TCP_Send(socket, array, len) != len) {
                working = false;
            }
        }
        inline short* r_shortarray(int len) {
            short *array = new short[len];
            for (int i = 0; i < len; i++) {
                array[i] = r_short();
            }
            return array;
        }
        inline void w_shortarray(short* array, int len) {
            if (SDLNet_TCP_Send(socket, array, len) != len) {
                working = false;
            }
        }
};

int packets_thread(Client *client) {
    SocketIO io(client->socket);
    unsigned char pid;
    while (client->doPackets && io.working) {
        pid = io.r_ubyte();
        //std::cout << "Parsing 0x" << std::hex << (int)pid << '\n';
        if (!io.working) break;
        switch (pid) {
            case 0x00:{
                p_keep_alive *p = new p_keep_alive;
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x01:{
                p_login_request_stc *p = new p_login_request_stc;
                p->EntityID = io.r_int();
                p->Unknown = io.r_string16();
                p->MapSeed = io.r_long();
                p->Dimension = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->Unknown;
                delete p;
            } break;
            case 0x02:{
                p_handshake_stc *p = new p_handshake_stc;
                p->ConnectionHash = io.r_string16();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x03:{
                p_chat_message *p = new p_chat_message;
                p->Message = io.r_string16();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->Message;
                delete p;
            } break;
            case 0x04:{
                p_time_update *p = new p_time_update;
                p->Time = io.r_long();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x05:{
                p_entity_equipment *p = new p_entity_equipment;
                p->EntityID = io.r_int();
                p->Slot = io.r_short();
                p->ItemID = io.r_short();
                p->Damage = io.r_short();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x06:{
                p_spawn_position *p = new p_spawn_position;
                p->X = io.r_int();
                p->Y = io.r_int();
                p->Z = io.r_int();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x07:{
                p_use_entity *p = new p_use_entity;
                p->UserEID = io.r_int();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x08:{
                p_update_health *p = new p_update_health;
                p->Health = io.r_short();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x09:{
                p_respawn *p = new p_respawn;
                p->World = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x0A:{
                p_player *p = new p_player;
                p->OnGround = io.r_bool();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x0B:{
                p_player_position *p = new p_player_position;
                p->X = io.r_double();
                p->Y = io.r_double();
                p->Stance = io.r_double();
                p->Z = io.r_double();
                p->OnGround = io.r_bool();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x0C:{
                p_player_look *p = new p_player_look;
                p->Yaw = io.r_float();
                p->Pitch = io.r_float();
                p->OnGround = io.r_bool();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x0E:{
                p_player_digging *p = new p_player_digging;
                p->Status = io.r_byte();
                p->X = io.r_int();
                p->Y = io.r_byte();
                p->Z = io.r_int();
                p->Face = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x10:{
                p_holding_change *p = new p_holding_change;
                p->SlotID = io.r_short();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x11:{
                p_use_bed *p = new p_use_bed;
                p->EntityID = io.r_int();
                p->InBed = io.r_byte();
                p->X = io.r_int();
                p->Y = io.r_byte();
                p->Z = io.r_int();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x12:{
                p_animate *p = new p_animate;
                p->EntityID = io.r_int();
                p->Animate = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x13:{
                p_act *p = new p_act;
                p->EntityID = io.r_int();
                p->Action = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->Name;
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x16:{
                p_collect_item *p = new p_collect_item;
                p->CollectedEID = io.r_int();
                p->CollectorEID = io.r_int();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->Metadata;
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->Title;
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x1C:{
                p_entity_velocity *p = new p_entity_velocity;
                p->EntityID = io.r_int();
                p->vx = io.r_short();
                p->vy = io.r_short();
                p->vz = io.r_short();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x1D:{
                p_destroy_entity *p = new p_destroy_entity;
                p->EntityID = io.r_int();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x1E:{
                p_entity *p = new p_entity;
                p->EntityID = io.r_int();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x1F:{
                p_entity_relative_move *p = new p_entity_relative_move;
                p->EntityID = io.r_int();
                p->dX = io.r_byte();
                p->dY = io.r_byte();
                p->dZ = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x20:{
                p_entity_look *p = new p_entity_look;
                p->EntityID = io.r_int();
                p->Yaw = io.r_byte();
                p->Pitch = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x26:{
                p_entity_status *p = new p_entity_status;
                p->EntityID = io.r_int();
                p->Status = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x27:{
                p_attach_entity *p = new p_attach_entity;
                p->EntityID = io.r_int();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x28:{
                p_entity_metadata *p = new p_entity_metadata;
                p->EntityID = io.r_int();
                p->Metadata = io.r_metadata();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->Metadata;
                delete p;
            } break;
            case 0x32:{
                p_prechunk *p = new p_prechunk;
                p->X = io.r_int();
                p->Z = io.r_int();
                p->Mode = io.r_bool();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->CompressedData;
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->CoordinateArray;
                delete p->TypeArray;
                delete p->MetadataArray;
                delete p;
            } break;
            case 0x35:{
                p_block_change *p = new p_block_change;
                p->X = io.r_int();
                p->Y = io.r_byte();
                p->Z = io.r_int();
                p->Type = io.r_byte();
                p->Metadata = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x36:{
                p_block_action *p = new p_block_action;
                p->X = io.r_int();
                p->Y = io.r_short();
                p->Z = io.r_int();
                p->DataA  = io.r_byte();
                p->DataB = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x3C:{
                p_explosion *p = new p_explosion;
                p->X = io.r_double();
                p->Y = io.r_double();
                p->Z = io.r_double();
                p->Unknown = io.r_float();
                p->RecordCount = io.r_int();
                p->Records = io.r_bytearray(p->RecordCount*3);
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->Records;
                delete p;
            } break;
            case 0x3D:{
                p_sound_effect *p = new p_sound_effect;
                p->EffectID = io.r_int();
                p->X = io.r_int();
                p->Y = io.r_byte();
                p->Z = io.r_int();
                p->SoundData = io.r_int();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x46:{
                p_new_state *p = new p_new_state;
                p->Reason = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x47:{
                p_thunderbolt *p = new p_thunderbolt;
                p->EntityID = io.r_int();
                p->Unknown = io.r_bool();
                p->X = io.r_int();
                p->Y = io.r_int();
                p->Z = io.r_int();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x64:{
                p_open_window *p = new p_open_window;
                p->WindowID = io.r_byte();
                p->InventoryType = io.r_byte();
                p->WindowTitle = io.r_string8();
                p->NumberOfSlots = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->WindowTitle;
                delete p;
            } break;
            case 0x65:{
                p_close_window *p = new p_close_window;
                p->WindowID = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x67:{
                p_set_slot *p = new p_set_slot;
                p->WindowID = io.r_byte();
                p->Slot = io.r_short();
                p->ItemID = io.r_short();
                if (p->ItemID != -1) {
                    p->ItemCount = io.r_byte();
                    p->ItemUses = io.r_short();
                } else {
                    p->ItemCount = -1;
                    p->ItemUses = -1;
                }
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x68:{
                p_window_items *p = new p_window_items;
                p->WindowID = io.r_byte();
                p->Count = io.r_short();
                p->items = io.r_itemarray(p->Count);
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->items;
                delete p;
            } break;
            case 0x69:{
                p_update_progress_bar *p = new p_update_progress_bar;
                p->WindowID = io.r_byte();
                p->ProgressBar = io.r_short();
                p->Value = io.r_short();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0x6A:{
                p_transaction *p = new p_transaction;
                p->WindowID = io.r_byte();
                p->ActionNumber = io.r_short();
                p->Accepted = io.r_bool();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
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
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->Text1;
                delete p->Text2;
                delete p->Text3;
                delete p->Text4;
                delete p;
            } break;
            case 0x83:{
                p_map_data *p = new p_map_data;
                p->UnknownA = io.r_short();
                p->UnknownB = io.r_short();
                p->TextLength = io.r_ubyte();
                p->Text = io.r_bytearray(p->TextLength);
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->Text;
                delete p;
            } break;
            case 0xC8:{
                p_increment_statistic *p = new p_increment_statistic;
                p->StatisticID = io.r_int();
                p->Amount = io.r_byte();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p;
            } break;
            case 0xFF:{
                p_kick *p = new p_kick;
                p->Message = io.r_string16();
                if (!io.working) break;
                p->id = pid;
                client->packet((p_generic*)p);
                delete p->Message;
                delete p;
            } break;
            default:
                std::cout << "Undefined packet: 0x" << std::hex << (int)pid << '\n';
                client->disconnect();
                return 1;
        }
    }
    if (!io.working) {
        std::cout << "Socket Error 0x" << std::hex << (int)pid << '\n';
        client->disconnect();
    }
    return 0;
}

bool write_packet(TCPsocket socket, p_generic *packet) {
    SocketIO io(socket);
    unsigned char pid = packet->id;
    io.w_ubyte(pid);
    //std::cout << "Sending Packet 0x" << std::hex << (int)pid << '\n';
    switch (pid) {
        case 0x00:{
            p_keep_alive *p = (p_keep_alive*)packet;
        } break;
        case 0x01:{
            p_login_request_cts *p = (p_login_request_cts*)packet;
            io.w_int(p->Version);
            io.w_string16(p->Username);
            io.w_long(p->MapSeed);
            io.w_byte(p->Dimension);
        } break;
        case 0x02:{
            p_handshake_cts *p = (p_handshake_cts*)packet;
            io.w_string16(p->Username);
        } break;
        case 0x03:{
            p_chat_message *p = (p_chat_message*)packet;
            io.w_string16(p->Message);
        } break;
        case 0x04:{
            p_time_update *p = (p_time_update*)packet;
            io.w_long(p->Time);
        } break;
        case 0x05:{
            p_entity_equipment *p = (p_entity_equipment*)packet;
            io.w_int(p->EntityID);
            io.w_short(p->Slot);
            io.w_short(p->ItemID);
            io.w_short(p->Damage);
        } break;
        case 0x06:{
            p_spawn_position *p = (p_spawn_position*)packet;
            io.w_int(p->X);
            io.w_int(p->Y);
            io.w_int(p->Z);
        } break;
        case 0x07:{
            p_use_entity *p = (p_use_entity*)packet;
            io.w_int(p->UserEID);
        } break;
        case 0x08:{
            p_update_health *p = (p_update_health*)packet;
            io.w_short(p->Health);
        } break;
        case 0x09:{
            p_respawn *p = (p_respawn*)packet;
            io.w_byte(p->World);
        } break;
        case 0x0A:{
            p_player *p = (p_player*)packet;
            io.w_bool(p->OnGround);
        } break;
        case 0x0B:{
            p_player_position *p = (p_player_position*)packet;
            io.w_double(p->X);
            io.w_double(p->Y);
            io.w_double(p->Stance);
            io.w_double(p->Z);
            io.w_bool(p->OnGround);
        } break;
        case 0x0C:{
            p_player_look *p = (p_player_look*)packet;
            io.w_float(p->Yaw);
            io.w_float(p->Pitch);
            io.w_bool(p->OnGround);
        } break;
        case 0x0D:{
            p_player_position_and_look_cts *p = (p_player_position_and_look_cts*)packet;
            io.w_double(p->X);
            io.w_double(p->Y);
            io.w_double(p->Stance);
            io.w_double(p->Z);
            io.w_float(p->Yaw);
            io.w_float(p->Pitch);
            io.w_bool(p->OnGround);
        } break;
        case 0x0E:{
            p_player_digging *p = (p_player_digging*)packet;
            io.w_byte(p->Status);
            io.w_int(p->X);
            io.w_byte(p->Y);
            io.w_int(p->Z);
            io.w_byte(p->Face);
        } break;
        case 0x0F:{
            p_player_block_placement *p = (p_player_block_placement*)packet;
            io.w_int(p->X);
            io.w_byte(p->Y);
            io.w_int(p->Z);
            io.w_byte(p->Direction);
            io.w_short(p->BlockID);
            if (p->BlockID >= 0) {
                io.w_byte(p->Amount);
                io.w_short(p->Damage);
            }
        } break;
        case 0x10:{
            p_holding_change *p = (p_holding_change*)packet;
            io.w_short(p->SlotID);
        } break;
        case 0x11:{
            p_use_bed *p = (p_use_bed*)packet;
            io.w_int(p->EntityID);
            io.w_byte(p->InBed);
            io.w_int(p->X);
            io.w_byte(p->Y);
            io.w_int(p->Z);
        } break;
        case 0x12:{
            p_animate *p = (p_animate*)packet;
            io.w_int(p->EntityID);
            io.w_byte(p->Animate);
        } break;
        case 0x13:{
            p_act *p = (p_act*)packet;
            io.w_int(p->EntityID);
            io.w_byte(p->Action);
        } break;
        case 0x14:{
            p_spawn_player *p = (p_spawn_player*)packet;
            io.w_int(p->EID);
            io.w_string16(p->Name);
            io.w_int(p->X);
            io.w_int(p->Y);
            io.w_int(p->Z);
            io.w_byte(p->Yaw);
            io.w_byte(p->Pitch);
            io.w_short(p->CurrentItem);
        } break;
        case 0x15:{
            p_pickup_spawn *p = (p_pickup_spawn*)packet;
            io.w_int(p->EntityID);
            io.w_short(p->Item);
            io.w_byte(p->Count);
            io.w_short(p->Damage);
            io.w_int(p->X);
            io.w_int(p->Y);
            io.w_int(p->Z);
            io.w_byte(p->Yaw);
            io.w_byte(p->Pitch);
            io.w_byte(p->Roll);
        } break;
        case 0x16:{
            p_collect_item *p = (p_collect_item*)packet;
            io.w_int(p->CollectedEID);
            io.w_int(p->CollectorEID);
        } break;
        case 0x17:{
            p_addobject *p = (p_addobject*)packet;
            io.w_int(p->EntityID);
            io.w_byte(p->Type);
            io.w_int(p->X);
            io.w_int(p->Y);
            io.w_int(p->Z);
            io.w_int(p->Flag);
            if (p->Flag > 0) {
                io.w_short(p->Xmap);
                io.w_short(p->Ymap);
                io.w_short(p->Zmap);
            }
        
        } break;
        case 0x18:{
            p_spawn_mob *p = (p_spawn_mob*)packet;
            io.w_int(p->EntityID);
            io.w_byte(p->Type);
            io.w_int(p->X);
            io.w_int(p->Y);
            io.w_int(p->Z);
            io.w_byte(p->Yaw);
            io.w_byte(p->Pitch);
            io.w_metadata(p->Metadata);
        } break;
        case 0x19:{
            p_painting *p = (p_painting*)packet;
            io.w_int(p->EntityID);
            io.w_string16(p->Title);
            io.w_int(p->X);
            io.w_int(p->Y);
            io.w_int(p->Z);
            io.w_int(p->Direction);
        } break;
        case 0x1B:{
            p_stance_update *p = (p_stance_update*)packet;
            io.w_float(p->A);
            io.w_float(p->B);
            io.w_float(p->C);
            io.w_float(p->D);
            io.w_bool(p->E);
            io.w_bool(p->F);
        } break;
        case 0x1C:{
            p_entity_velocity *p = (p_entity_velocity*)packet;
            io.w_int(p->EntityID);
            io.w_short(p->vx);
            io.w_short(p->vy);
            io.w_short(p->vz);
        } break;
        case 0x1D:{
            p_destroy_entity *p = (p_destroy_entity*)packet;
            io.w_int(p->EntityID);
        } break;
        case 0x1E:{
            p_entity *p = (p_entity*)packet;
            io.w_int(p->EntityID);
        } break;
        case 0x1F:{
            p_entity_relative_move *p = (p_entity_relative_move*)packet;
            io.w_int(p->EntityID);
            io.w_byte(p->dX);
            io.w_byte(p->dY);
            io.w_byte(p->dZ);
        } break;
        case 0x20:{
            p_entity_look *p = (p_entity_look*)packet;
            io.w_int(p->EntityID);
            io.w_byte(p->Yaw);
            io.w_byte(p->Pitch);
        } break;
        case 0x21:{
            p_entity_look_and_relative_move *p = (p_entity_look_and_relative_move*)packet;
            io.w_int(p->EntityID);
            io.w_byte(p->dX);
            io.w_byte(p->dY);
            io.w_byte(p->dZ);
            io.w_byte(p->Yaw);
            io.w_byte(p->Pitch);
        } break;
        case 0x22:{
            p_entity_teleport *p = (p_entity_teleport*)packet;
            io.w_int(p->EntityID);
            io.w_int(p->X);
            io.w_int(p->Y);
            io.w_int(p->Z);
            io.w_byte(p->Yaw);
            io.w_byte(p->Pitch);
        } break;
        case 0x26:{
            p_entity_status *p = (p_entity_status*)packet;
            io.w_int(p->EntityID);
            io.w_byte(p->Status);
        } break;
        case 0x27:{
            p_attach_entity *p = (p_attach_entity*)packet;
            io.w_int(p->EntityID);
        } break;
        case 0x28:{
            p_entity_metadata *p = (p_entity_metadata*)packet;
            io.w_int(p->EntityID);
            io.w_metadata(p->Metadata);
        } break;
        case 0x32:{
            p_prechunk *p = (p_prechunk*)packet;
            io.w_int(p->X);
            io.w_int(p->Z);
            io.w_bool(p->Mode);
        } break;
        case 0x33:{
            p_map_chunk *p = (p_map_chunk*)packet;
            io.w_int(p->X);
            io.w_short(p->Y);
            io.w_int(p->Z);
            io.w_byte(p->SizeX);
            io.w_byte(p->SizeY);
            io.w_byte(p->SizeZ);
            io.w_int(p->CompressedSize);
            io.w_bytearray(p->CompressedData,p->CompressedSize);
        } break;
        case 0x34:{
            p_multi_block_change *p = (p_multi_block_change*)packet;
            io.w_int(p->ChunkX);
            io.w_int(p->ChunkZ);
            io.w_short(p->ArraySize);
            io.w_shortarray(p->CoordinateArray,p->ArraySize);
            io.w_bytearray(p->TypeArray,p->ArraySize);
            io.w_bytearray(p->MetadataArray,p->ArraySize);
        } break;
        case 0x35:{
            p_block_change *p = (p_block_change*)packet;
            io.w_int(p->X);
            io.w_byte(p->Y);
            io.w_int(p->Z);
            io.w_byte(p->Type);
            io.w_byte(p->Metadata);
        } break;
        case 0x36:{
            p_block_action *p = (p_block_action*)packet;
            io.w_int(p->X);
            io.w_short(p->Y);
            io.w_int(p->Z);
            io.w_byte(p->DataA);
            io.w_byte(p->DataB);
        } break;
        case 0x3C:{
            p_explosion *p = (p_explosion*)packet;
            io.w_double(p->X);
            io.w_double(p->Y);
            io.w_double(p->Z);
            io.w_float(p->Unknown);
            io.w_int(p->RecordCount);
            io.w_bytearray(p->Records,p->RecordCount*3);
        } break;
        case 0x3D:{
            p_sound_effect *p = (p_sound_effect*)packet;
            io.w_int(p->EffectID);
            io.w_int(p->X);
            io.w_byte(p->Y);
            io.w_int(p->Z);
            io.w_int(p->SoundData);
        } break;
        case 0x46:{
            p_new_state *p = (p_new_state*)packet;
            io.w_byte(p->Reason);
        } break;
        case 0x47:{
            p_thunderbolt *p = (p_thunderbolt*)packet;
            io.w_int(p->EntityID);
            io.w_bool(p->Unknown);
            io.w_int(p->X);
            io.w_int(p->Y);
            io.w_int(p->Z);
        } break;
        case 0x64:{
            p_open_window *p = (p_open_window*)packet;
            io.w_byte(p->WindowID);
            io.w_byte(p->InventoryType);
            io.w_string8(p->WindowTitle);
            io.w_byte(p->NumberOfSlots);
        } break;
        case 0x65:{
            p_close_window *p = (p_close_window*)packet;
            io.w_byte(p->WindowID);
        } break;
        case 0x66:{
            p_window_click *p = (p_window_click*)packet;
            io.w_byte(p->WindowID);
            io.w_short(p->Slot);
            io.w_byte(p->RightClick);
            io.w_short(p->ActionNumber);
            io.w_bool(p->Shift);
            io.w_short(p->ItemID);
            if (p->ItemID != -1) {
                io.w_byte(p->ItemCount);
                io.w_short(p->ItemUses);
            }
        } break;
        case 0x67:{
            p_set_slot *p = (p_set_slot*)packet;
            io.w_byte(p->WindowID);
            io.w_short(p->Slot);
            io.w_short(p->ItemID);
            if (p->ItemID != -1) {
                io.w_byte(p->ItemCount);
                io.w_short(p->ItemUses);
            }
        } break;
        case 0x68:{
            p_window_items *p = (p_window_items*)packet;
            io.w_byte(p->WindowID);
            io.w_short(p->Count);
            io.w_itemarray(p->items,p->Count);
        } break;
        case 0x69:{
            p_update_progress_bar *p = (p_update_progress_bar*)packet;
            io.w_byte(p->WindowID);
            io.w_short(p->ProgressBar);
            io.w_short(p->Value);
        } break;
        case 0x6A:{
            p_transaction *p = (p_transaction*)packet;
            io.w_byte(p->WindowID);
            io.w_short(p->ActionNumber);
            io.w_bool(p->Accepted);
        } break;
        case 0x82:{
            p_update_sign *p = (p_update_sign*)packet;
            io.w_int(p->X);
            io.w_short(p->Y);
            io.w_int(p->Z);
            io.w_string16(p->Text1);
            io.w_string16(p->Text2);
            io.w_string16(p->Text3);
            io.w_string16(p->Text4);
        } break;
        case 0x83:{
            p_map_data *p = (p_map_data*)packet;
            io.w_short(p->UnknownA);
            io.w_short(p->UnknownB);
            io.w_ubyte(p->TextLength);
            io.w_bytearray(p->Text,p->TextLength);
        } break;
        case 0xC8:{
            p_increment_statistic *p = (p_increment_statistic*)packet;
            io.w_int(p->StatisticID);
            io.w_byte(p->Amount);
        } break;
        case 0xFF:{
            p_kick *p = (p_kick*)packet;
            io.w_string16(p->Message);
        } break;
    }
    return io.working;
}

