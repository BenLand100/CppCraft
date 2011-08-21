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
 
#include "tools.h"
#include "items.h"
 
double incPerTick(int tool, int item, bool underwater, bool onground) {
    int hardness = item_hardness(item);
    if (hardness < 0) return 0;
    if (!canHarvest(tool,item)) return 1.0/hardness/100.0;
    double factor = 1.0;
    if (isEffective(tool,item)) factor *= effectiveness(tool);
    if (underwater) factor /= 5.0;
    if (!onground) factor /= 5.0;
    return factor / hardness / 30.0;
}

bool isEffective(int tool, int item) {
    switch (tool_type(tool)) {
        case T_AXE:
            switch (item) {
                case 5: case 17: case 47: return true;
                default: return false;
            }
        case T_SHOVEL:
            switch (item) {
                case 2: case 3: case  12: case 13: case 78: case 80: case 82: return true;
                default: return false;
            }
        case T_PICK:
            switch (item) {
                case 1: case 4: case 14: case 15: case 16: case 21: case 22: case 41: case 42: case 43: case 44: case 48: case 56: case 57: return true;
                default: return false;
            }
        default: return false;
    }
}

bool canHarvest(int tool, int item) {
    int material = item_material(item);
    switch (material) {
        case M_STONE: 
        case M_IRON:
        case M_SNOW:
        case M_SNOWBLOCK:
            switch (tool_type(tool)) {
                case T_SHOVEL:
                    return (material == M_SNOW || material == M_SNOWBLOCK);
                case T_PICK:
                    switch (item_material(tool)) { //fall through is important
                        case M_DIAMOND:
                            switch (material) {
                                case 49: return true;
                            }
                        case M_IRON:
                            switch (material) {
                                case 56: case 57: case 41: case 14: case 73: case 74: return true;
                            }
                        case M_STONE:
                            switch (material) {
                                case 21: case 22: case 42: case 15: return true;
                            }
                        default: 
                            return (material == M_STONE || material == M_IRON); //endpoint for the fall through
                    }
                case T_AXE:
                    return false; //axe can't harvest anything special
                default:
                    return false;
            }
        default:
            return true;
    }

}

double effectiveness(int tool) {
    switch (item_material(tool)) {
        case M_WOOD: return 2.0;
        case M_STONE: return 4.0;
        case M_IRON: return 6.0;
        case M_DIAMOND: return 8.0;
        case M_GOLD: return 12.0;
        default: return 0.0;
    }
}

int tool_type(int tool) {
    switch (tool) {
        case 256: case 269: case 273: case 277: case 284: return T_SHOVEL;
        case 257: case 270: case 274: case 278: case 285: return T_PICK;
        case 258: case 271: case 275: case 279: case 286: return T_AXE;
        case 267: case 268: case 272: case 276: case 283: return T_SWORD;
        default: return T_NONE;
    }
}
