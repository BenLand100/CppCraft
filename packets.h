#ifndef _packet
#define _packet

#include "SDL_net.h"

class Client; //foward declaration
typedef struct { unsigned char id;} p_generic;

int packets_thread(Client *client);
bool write_packet(TCPsocket socket, p_generic *p);

typedef char* string8;
typedef char* string16;
typedef struct { /* yep, doesn't do anything yet */ } metadata;
typedef struct { short itemid; char count; short uses; } item;

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
typedef struct { unsigned char id; int EntityID; short vx; short vy; short vz; } p_entity_velocity;
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
typedef struct { unsigned char id; double X; double Y; double Z; float Unknown; int RecordCount; char *Records;} p_explosion;
typedef struct { unsigned char id; int EffectID; int X; char Y; int Z; int SoundData;} p_sound_effect;
typedef struct { unsigned char id; char Reason;} p_new_state;
typedef struct { unsigned char id; int EntityID; bool Unknown; int X; int Y; int Z;} p_thunderbolt;
typedef struct { unsigned char id; char WindowID; char InventoryType; string8 WindowTitle; char NumberOfSlots;} p_open_window;
typedef struct { unsigned char id; char WindowID;} p_close_window;
typedef struct { unsigned char id; char WindowID; short Slot; char RightClick; short ActionNumber; bool Shift; short ItemID; char ItemCount; short ItemUses;} p_window_click;
typedef struct { unsigned char id; char WindowID; short Slot; short ItemID; char ItemCount; short ItemUses;} p_set_slot;
typedef struct { unsigned char id; char WindowID; short Count; item *items;} p_window_items;
typedef struct { unsigned char id; char WindowID; short ProgressBar; short Value;} p_update_progress_bar;
typedef struct { unsigned char id; char WindowID; short ActionNumber; bool Accepted;} p_transaction;
typedef struct { unsigned char id; int X; short Y; int Z; string16 Text1; string16 Text2; string16 Text3; string16 Text4;} p_update_sign;
typedef struct { unsigned char id; short UnknownA; short UnknownB; unsigned char TextLength; char* Text;} p_map_data;
typedef struct { unsigned char id; int StatisticID; char Amount;} p_increment_statistic;
typedef struct { unsigned char id; string16 Message;} p_kick;


#define send_keep_alive(sock) { \
        p_keep_alive p; \
        p.id = 0x00; \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_login_request_cts(sock,_Version,_Username,_MapSeed,_Dimension) { \
        p_login_request_cts p; \
        p.id = 0x01; \
        p.Version = (_Version); \
        p.Username = (_Username); \
        p.MapSeed = (_MapSeed); \
        p.Dimension = (_Dimension); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_login_request_stc(sock,_EntityID,_Unknown,_MapSeed,_Dimension) { \
        p_login_request_stc p; \
        p.id = 0x01; \
        p.EntityID = (_EntityID); \
        p.Unknown = (_Unknown); \
        p.MapSeed = (_MapSeed); \
        p.Dimension = (_Dimension); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_handshake_cts(sock,_Username) { \
        p_handshake_cts p; \
        p.id = 0x02; \
        p.Username = (_Username); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_handshake_stc(sock,_ConnectionHash) { \
        p_handshake_stc p; \
        p.id = 0x02; \
        p.ConnectionHash = (_ConnectionHash); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_chat_message(sock,_Message) { \
        p_chat_message p; \
        p.id = 0x03; \
        p.Message = (_Message); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_time_update(sock,_Time) { \
        p_time_update p; \
        p.id = 0x04; \
        p.Time = (_Time); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_entity_equipment(sock,_EntityID,_Slot,_ItemID,_Damage) { \
        p_entity_equipment p; \
        p.id = 0x05; \
        p.Entity ID = (_EntityID); \
        p.Slot = (_Slot); \
        p.ItemID = (_ItemID); \
        p.Damage = (_Damage); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_spawn_position(sock,_X,_Y,_Z) { \
        p_spawn_position p; \
        p.id = 0x06; \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_use_entity(sock,_UserEID) { \
        p_use_entity p; \
        p.id = 0x07; \
        p.UserEID = (_UserEID); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_update_health(sock,_Health) { \
        p_update_health p; \
        p.id = 0x08; \
        p.Health = (_Health); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_respawn(sock,_World) { \
        p_respawn p; \
        p.id = 0x09; \
        p.World = (_World); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_player(sock,_OnGround) { \
        p_player p; \
        p.id = 0x0A; \
        p.OnGround = (_OnGround); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_player_position(sock,_X,_Y,_Stance,_Z,_OnGround) { \
        p_player_position p; \
        p.id = 0x0B; \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Stance = (_Stance); \
        p.Z = (_Z); \
        p.OnGround = (_OnGround); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_player_look(sock,_Yaw,_Pitch,_OnGround) { \
        p_player_look p; \
        p.id = 0x0C; \
        p.Yaw = (_Yaw); \
        p.Pitch = (_Pitch); \
        p.OnGround = (_OnGround); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_player_position_and_look_cts(sock,_X,_Y,_Stance,_Z,_Yaw,_Pitch,_OnGround) { \
        p_player_position_and_look_cts p; \
        p.id = 0x0D; \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Stance = (_Stance); \
        p.Z = (_Z); \
        p.Yaw = (_Yaw); \
        p.Pitch = (_Pitch); \
        p.OnGround = (_OnGround); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_player_position_and_look_stc(sock,_X,_Stance,_Y,_Z,_Yaw,_Pitch,_OnGround) { \
        p_player_position_and_look_stc p; \
        p.id = 0x0D; \
        p.X = (_X); \
        p.Stance = (_Stance); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.Yaw = (_Yaw); \
        p.Pitch = (_Pitch); \
        p.OnGround = (_OnGround); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_player_digging(sock,_Status,_X,_Y,_Z,_Face) { \
        p_player_digging p; \
        p.id = 0x0E; \
        p.Status = (_Status); \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.Face = (_Face); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_player_block_placement(sock,_X,_Y,_Z,_Direction,_BlockID,_Amount,_Damage) { \
        p_player_block_placement p; \
        p.id = 0x0F; \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.Direction = (_Direction); \
        p.BlockID = (_BlockID); \
        p.Amount = (_Amount); \
        p.Damage = (_Damage); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_holding_change(sock,_SlotID) { \
        p_holding_change p; \
        p.id = 0x10; \
        p.SlotID = (_SlotID); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_use_bed(sock,_EntityID,_InBed,_X,_Y,_Z) { \
        p_use_bed p; \
        p.id = 0x11; \
        p.EntityID = (_EntityID); \
        p.InBed = (_InBed); \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_animate(sock,_EntityID,_Animate) { \
        p_animate p; \
        p.id = 0x12; \
        p.EntityID = (_EntityID); \
        p.Animate = (_Animate); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_act(sock,_EntityID,_Action) { \
        p_act p; \
        p.id = 0x13; \
        p.EntityID = (_EntityID); \
        p.Action = (_Action); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_spawn_player(sock,_EID,_Name,_X,_Y,_Z,_Yaw,_Pitch,_CurrentItem) { \
        p_spawn_player p; \
        p.id = 0x14; \
        p.EID = (_EID); \
        p.Name = (_Name); \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.Yaw = (_Yaw); \
        p.Pitch = (_Pitch); \
        p.CurrentItem = (_CurrentItem); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_pickup_spawn(sock,_EntityID,_Item,_Count,_Damage,_X,_Y,_Z,_Yaw,_Pitch,_Roll) { \
        p_pickup_spawn p; \
        p.id = 0x15; \
        p.EntityID = (_EntityID); \
        p.Item = (_Item); \
        p.Count = (_Count); \
        p.Damage = (_Damage); \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.Yaw = (_Yaw); \
        p.Pitch = (_Pitch); \
        p.Roll = (_Roll); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_collect_item(sock,_CollectedEID,_CollectorEID) { \
        p_collect_item p; \
        p.id = 0x16; \
        p.CollectedEID = (_CollectedEID); \
        p.CollectorEID = (_CollectorEID); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_addobject(sock,_EntityID,_Type,_X,_Y,_Z,_Flag,_Xmap,_Ymap,_Zmap) { \
        p_addobject p; \
        p.id = 0x17; \
        p.EntityID = (_EntityID); \
        p.Type = (_Type); \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.Flag = (_Flag); \
        p.Xmap = (_Xmap); \
        p.Ymap = (_Ymap); \
        p.Zmap = (_Zmap); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_spawn_mob(sock,_EntityID,_Type,_X,_Y,_Z,_Yaw,_Pitch,_Metadata) { \
        p_spawn_mob p; \
        p.id = 0x18; \
        p.EntityID = (_EntityID); \
        p.Type = (_Type); \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.Yaw = (_Yaw); \
        p.Pitch = (_Pitch); \
        p.Metadata = (_Metadata); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_painting(sock,_EntityID,_Title,_X,_Y,_Z,_Direction) { \
        p_painting p; \
        p.id = 0x19; \
        p.Entity ID = (_Entity ID); \
        p.Title = (_Title); \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.Direction = (_Direction); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_stance_update(sock,_A,_B,_C,_D,_E,_F) { \
        p_stance_update p; \
        p.id = 0x1B; \
        p.A = (_A); \
        p.B = (_B); \
        p.C = (_C); \
        p.D = (_D); \
        p.E = (_E); \
        p.F = (_F); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_entity_velocity(sock,_EntityID,_vx,_vy,_vz) { \
        p_entity_velocity p; \
        p.id = 0x1C; \
        p.EntityID = (_EntityID); \
        p.vx = (_vx); \
        p.vy = (_vy); \
        p.vz = (_vz); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_destroy_entity(sock,_EntityID) { \
        p_destroy_entity p; \
        p.id = 0x1D; \
        p.EntityID = (_EntityID); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_entity(sock,_EntityID) { \
        p_entity p; \
        p.id = 0x1E; \
        p.EntityID = (_EntityID); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_entity_relative_move(sock,_EntityID,_dX,_dY,_dZ) { \
        p_entity_relative_move p; \
        p.id = 0x1F; \
        p.EntityID = (_EntityID); \
        p.dX = (_dX); \
        p.dY = (_dY); \
        p.dZ = (_dZ); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_entity_look(sock,_EntityID,_Yaw,_Pitch) { \
        p_entity_look p; \
        p.id = 0x20; \
        p.EntityID = (_EntityID); \
        p.Yaw = (_Yaw); \
        p.Pitch = (_Pitch); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_entity_look_and_relative_move(sock,_EntityID,_dX,_dY,_dZ,_Yaw,_Pitch) { \
        p_entity_look_and_relative_move p; \
        p.id = 0x21; \
        p.EntityID = (_EntityID); \
        p.dX = (_dX); \
        p.dY = (_dY); \
        p.dZ = (_dZ); \
        p.Yaw = (_Yaw); \
        p.Pitch = (_Pitch); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_entity_teleport(sock,_EntityID,_X,_Y,_Z,_Yaw,_Pitch) { \
        p_entity_teleport p; \
        p.id = 0x22; \
        p.EntityID = (_EntityID); \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.Yaw = (_Yaw); \
        p.Pitch = (_Pitch); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_entity_status(sock,_EntityID,_Status) { \
        p_entity_status p; \
        p.id = 0x26; \
        p.EntityID = (_EntityID); \
        p.Status = (_Status); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_attach_entity(sock,_EntityID) { \
        p_attach_entity p; \
        p.id = 0x27; \
        p.EntityID = (_EntityID); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_entity_metadata(sock,_EntityID,_Metadata) { \
        p_entity_metadata p; \
        p.id = 0x28; \
        p.EntityID = (_EntityID); \
        p.Metadata = (_Metadata); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_prechunk(sock,_X,_Z,_Mode) { \
        p_prechunk p; \
        p.id = 0x32; \
        p.X = (_X); \
        p.Z = (_Z); \
        p.Mode = (_Mode); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_map_chunk(sock,_X,_Y,_Z,_SizeX,_SizeY,_SizeZ,_CompressedSize,_CompressedData) { \
        p_map_chunk p; \
        p.id = 0x33; \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.SizeX = (_SizeX); \
        p.SizeY = (_SizeY); \
        p.SizeZ = (_SizeZ); \
        p.CompressedSize = (_CompressedSize); \
        p.CompressedData = (_CompressedData); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_multi_block_change(sock,_ChunkX,_ChunkZ,_ArraySize,_CoordinateArray,_TypeArray,_MetadataArray) { \
        p_multi_block_change p; \
        p.id = 0x34; \
        p.ChunkX = (_ChunkX); \
        p.ChunkZ = (_ChunkZ); \
        p.ArraySize = (_ArraySize); \
        p.CoordinateArray = (_CoordinateArray); \
        p.TypeArray = (_TypeArray); \
        p.MetadataArray = (_MetadataArray); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_block_change(sock,_X,_Y,_Z,_Type,_Metadata) { \
        p_block_change p; \
        p.id = 0x35; \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.Type = (_Type); \
        p.Metadata = (_Metadata); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_block_action(sock,_X,_Y,_Z,_DataA ,_DataB) { \
        p_block_action p; \
        p.id = 0x36; \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.DataA  = (_DataA ); \
        p.DataB = (_DataB); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_explosion(sock,_X,_Y,_Z,_Unknown,_RecordCount,_Records) { \
        p_explosion p; \
        p.id = 0x3C; \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.Unknown = (_Unknown); \
        p.RecordCount = (_RecordCount); \
        p.Records = (_Records); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_sound_effect(sock,_EffectID,_X,_Y,_Z,_SoundData) { \
        p_sound_effect p; \
        p.id = 0x3D; \
        p.EffectID = (_EffectID); \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.SoundData = (_SoundData); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_new_state(sock,_Reason) { \
        p_new_state p; \
        p.id = 0x46; \
        p.Reason = (_Reason); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_thunderbolt(sock,_EntityID,_Unknown,_X,_Y,_Z) { \
        p_thunderbolt p; \
        p.id = 0x47; \
        p.EntityID = (_EntityID); \
        p.Unknown = (_Unknown); \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_open_window(sock,_WindowId,_InventoryType,_WindowTitle,_NumberOfSlots) { \
        p_open_window p; \
        p.id = 0x64; \
        p.WindowId = (_WindowId); \
        p.InventoryType = (_InventoryType); \
        p.WindowTitle = (_WindowTitle); \
        p.NumberOfSlots = (_NumberOfSlots); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_close_window(sock,_WindowID) { \
        p_close_window p; \
        p.id = 0x65; \
        p.WindowID = (_WindowID); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_window_click(sock,_WindowID,_Slot,_RightClick,_ActionNumber,_Shift,_ItemID,_ItemCount,_ItemUses) { \
        p_window_click p; \
        p.id = 0x66; \
        p.WindowID = (_WindowID); \
        p.Slot = (_Slot); \
        p.RightClick = (_RightClick); \
        p.ActionNumber = (_ActionNumber); \
        p.Shift = (_Shift); \
        p.ItemID = (_ItemID); \
        p.ItemCount = (_ItemCount); \
        p.ItemUses = (_ItemUses); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_set_slot(sock,_WindowID,_Slot,_ItemID,_ItemCount,_ItemUses) { \
        p_set_slot p; \
        p.id = 0x67; \
        p.WindowID = (_WindowID); \
        p.Slot = (_Slot); \
        p.ItemID = (_ItemID); \
        p.ItemCount = (_ItemCount); \
        p.ItemUses = (_ItemUses); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_window_items(sock,_WindowID,_Count,_Items) { \
        p_window_items p; \
        p.id = 0x68; \
        p.WindowID = (_WindowID); \
        p.Count = (_Count); \
        p.items = (_Items); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_update_progress_bar(sock,_WindowID,_ProgressBar,_Value) { \
        p_update_progress_bar p; \
        p.id = 0x69; \
        p.WindowID = (_WindowID); \
        p.ProgressBar = (_ProgressBar); \
        p.Value = (_Value); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_transaction(sock,_WindowID,_ActionNumber,_Accepted) { \
        p_transaction p; \
        p.id = 0x6A; \
        p.WindowID = (_WindowID); \
        p.ActionNumber = (_ActionNumber); \
        p.Accepted = (_Accepted); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_update_sign(sock,_X,_Y,_Z,_Text1,_Text2,_Text3,_Text4) { \
        p_update_sign p; \
        p.id = 0x82; \
        p.X = (_X); \
        p.Y = (_Y); \
        p.Z = (_Z); \
        p.Text1 = (_Text1); \
        p.Text2 = (_Text2); \
        p.Text3 = (_Text3); \
        p.Text4 = (_Text4); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_map_data(sock,_UnknownA,_UnknownB,_TextLength,_Text) { \
        p_map_data p; \
        p.id = 0x83; \
        p.UnknownA = (_UnknownA); \
        p.UnknownB = (_UnknownB); \
        p.TextLength = (_TextLength); \
        p.Text = (_Text); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_increment_statistic(sock,_StatisticID,_Amount) { \
        p_increment_statistic p; \
        p.id = 0xC8; \
        p.StatisticID = (_StatisticID); \
        p.Amount = (_Amount); \
        write_packet(sock,(p_generic*)&p); \
    }
#define send_kick(sock,_Message) { \
        p_kick p; \
        p.id = 0xFF; \
        p.Message = (_Message); \
        write_packet(sock,(p_generic*)&p); \
    }


#endif

