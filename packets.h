#ifndef _packet
#define _packet

#include "SDL_net.h"

class Client; //foward declaration
typedef struct { unsigned char id;} p_generic;

int packets_thread(Client *client);
bool write_packet(TCPsocket socket, p_generic *p);

typedef struct { short len; char *str; } string8;
typedef struct { short len; char *str; } string16;
typedef struct { /* yep, doesn't do anything yet */ } metadata;

typedef struct { unsigned char id; } p_keep_alive;
typedef struct { unsigned char id; int Version; string16 Username; long MapSeed; char Dimension;} p_login_request_cts;
typedef struct { unsigned char id; int EntityID; string16 Unknown; long MapSeed; char Dimension;} p_login_request_stc;
typedef struct { unsigned char id; string16 Username;} p_handshake_cts;
typedef struct { unsigned char id; string16 ConnectionHash;} p_handshake_stc;
typedef struct { unsigned char id; string16 Message;} p_chat_message;
typedef struct { unsigned char id; long Time;} p_time_update;
typedef struct { unsigned char id; int EntityID; short Slot; short ItemID; short Damage;} p_entity_equipment;
typedef struct { unsigned char id; int X; int Y; int Z;} p_spawn_position;
typedef struct { unsigned char id; int UserEID;} p_use_entity;
typedef struct { unsigned char id; short Health;} p_update_health;
typedef struct { unsigned char id; char World;} p_respawn;
typedef struct { unsigned char id; bool OnGround;} p_player;
typedef struct { unsigned char id; double X; double Y; double Stance; double Z; bool OnGround;} p_player_position;
typedef struct { unsigned char id; float Yaw; float Pitch; bool OnGround;} p_player_look;
typedef struct { unsigned char id; double X; double Y; double Stance; double Z; float Yaw; float Pitch; bool OnGround;} p_player_position_and_look_cts;
typedef struct { unsigned char id; double X; double Stance; double Y; double Z; float Yaw; float Pitch; bool OnGround;} p_player_position_and_look_stc;
typedef struct { unsigned char id; char Status; int X; char Y; int Z; char Face;} p_player_digging;
typedef struct { unsigned char id; int X; char Y; int Z; char Direction; short BlockID; char Amount; short Damage;} p_player_block_placement;
typedef struct { unsigned char id; short SlotID;} p_holding_change;
typedef struct { unsigned char id; int EntityID; char InBed; int X; char Y; int Z;} p_use_bed;
typedef struct { unsigned char id; int EntityID; char Animate;} p_animate;
typedef struct { unsigned char id; int EntityID; char Action;} p_act;
typedef struct { unsigned char id; int EID; string16 Name; int X; int Y; int Z; char Yaw; char Pitch; short CurrentItem;} p_spawn_player;
typedef struct { unsigned char id; int EntityID; short Item; char Count; short Damage; int X; int Y; int Z; char Yaw; char Pitch; char Roll;} p_pickup_spawn;
typedef struct { unsigned char id; int CollectedEID; int CollectorEID;} p_collect_item;
typedef struct { unsigned char id; int EntityID; char Type; int X; int Y; int Z; int Flag; short Xmap; short Ymap; short Zmap;} p_addobject;
typedef struct { unsigned char id; int EntityID; char Type; int X; int Y; int Z; char Yaw; char Pitch; metadata *Metadata;} p_spawn_mob;
typedef struct { unsigned char id; int EntityID; string16 Title; int X; int Y; int Z; int Direction;} p_painting;
typedef struct { unsigned char id; float A; float B; float C; float D; bool E; bool F;} p_stance_update;
typedef struct { unsigned char id; int EntityID;} p_entity_velocity;
typedef struct { unsigned char id; int EntityID;} p_destroy_entity;
typedef struct { unsigned char id; int EntityID;} p_entity;
typedef struct { unsigned char id; int EntityID; char dX; char dY; char dZ;} p_entity_relative_move;
typedef struct { unsigned char id; int EntityID; char Yaw; char Pitch;} p_entity_look;
typedef struct { unsigned char id; int EntityID; char dX; char dY; char dZ; char Yaw; char Pitch;} p_entity_look_and_relative_move;
typedef struct { unsigned char id; int EntityID; int X; int Y; int Z; char Yaw; char Pitch;} p_entity_teleport;
typedef struct { unsigned char id; int EntityID; char Status;} p_entity_status;
typedef struct { unsigned char id; int EntityID;} p_attach_entity;
typedef struct { unsigned char id; int EntityID; metadata *Metadata;} p_entity_metadata;
typedef struct { unsigned char id; int X; int Z; bool Mode;} p_prechunk;
typedef struct { unsigned char id; int X; short Y; int Z; char SizeX; char SizeY; char SizeZ; int CompressedSize; char* CompressedData;} p_map_chunk;
typedef struct { unsigned char id; int ChunkX; int ChunkZ; short ArraySize; short* CoordinateArray; char* TypeArray; char* MetadataArray;} p_multi_block_change;
typedef struct { unsigned char id; int X; char Y; int Z; char Type; char Metadata;} p_block_change;
typedef struct { unsigned char id; int X; short Y; int Z; char DataA; char DataB;} p_block_action;
//typedef struct { unsigned char id; double X; double Y; double Z; float Unknown; int RecordCount; (char, char, char) × count Records;} p_explosion;
typedef struct { unsigned char id; int EffectID; int X; char Y; int Z; int SoundData;} p_sound_effect;
typedef struct { unsigned char id; char Reason;} p_new_state;
typedef struct { unsigned char id; int EntityID; bool Unknown; int X; int Y; int Z;} p_thunderbolt;
typedef struct { unsigned char id; char WindowID; char InventoryType; string8 WindowTitle; char NumberOfSlots;} p_open_window;
typedef struct { unsigned char id; char WindowID;} p_close_window;
typedef struct { unsigned char id; char WindowID; short Slot; char RightClick; short ActionNumber; bool Shift; short ItemID; char ItemCount; short ItemUses;} p_window_click;
typedef struct { unsigned char id; char WindowID; short Slot; short ItemID; char ItemCount; short ItemUses;} p_set_slot;
//typedef struct { unsigned char id; char WindowID; short Count; … Payload;} p_window_items;
typedef struct { unsigned char id; char WindowID; short ProgressBar; short Value;} p_update_progress_bar;
typedef struct { unsigned char id; char WindowID; short ActionNumber; bool Accepted;} p_transaction;
typedef struct { unsigned char id; int X; short Y; int Z; string16 Text1; string16 Text2; string16 Text3; string16 Text4;} p_update_sign;
typedef struct { unsigned char id; short UnknownA; short UnknownB; unsigned char TextLength; char* Text;} p_map_data;
typedef struct { unsigned char id; int StatisticID; char Amount;} p_increment_statistic;
typedef struct { unsigned char id; string16 Message;} p_kick;

#endif

