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
 
#ifndef _tools
#define _tools

double incPerTick(int tool, int item, bool underwater, bool onground);

bool isEffective(int tool, int item);
bool canHarvest(int tool, int item);
double effectiveness(int tool);

#define T_NONE      0
#define T_PICK      1
#define T_AXE       2
#define T_SHOVEL    3
#define T_SWORD     4

int tool_type(int tool);

#endif
