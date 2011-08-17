#!/usr/bin/python

'''
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
'''

from re import *
from string import *

f = open('items', 'r')

items = []

while True:
    line = f.readline()
    if line == '': break
    line = strip(line)
    if len(line) > 0 and line[0] != '#':
        #id,name,maxstack,placeable,useable,solid,diggable,activatable,safe,hardness,material
        parts = split(line)
        parts[0] = str(int(parts[0],16)) if parts[0] != '-1' else '-1'
        parts[1] = '(const char*)"'+parts[1]+'"'
        for i in range(2,9):
            parts[i] = 'true' if parts[i] == '1' else 'false'
        parts[5] = 'true' if parts[5] == 'false' else 'false'
        parts[10] = 'M_'+(parts[10].upper() if parts[10] != 'NoMaterial' else 'NONE')
        items.append(parts)

methods = ['',
'inline const char* item_name(int id) {\n    switch(id) {\n',
'inline int item_maxstack(int id) {\n    switch(id) {\n',
'inline bool item_placeable(int id) {\n    switch(id) {\n',
'inline bool item_usable(int id) {\n    switch(id) {\n',
'inline bool item_passable(int id) {\n    switch(id) {\n',
'inline bool item_diggable(int id) {\n    switch(id) {\n',
'inline bool item_activatable(int id) {\n    switch(id) {\n',
'inline bool item_safe(int id) {\n    switch(id) {\n',
'inline float item_hardness(int id) {\n    switch(id) {\n',
'inline int item_material(int id) {\n    switch(id) {\n']

for parts in items:
    for i in range(1,len(parts)):
        methods[i] += '        case ' + parts[0] + ': return ' + parts[i] + ';\n'
        
print '''/**
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

#ifndef _items
#define _items

#define M_NONE          0
#define M_STONE         1
#define M_DIRT          2
#define M_WOOD          3
#define M_CROPS         4
#define M_WATER         5
#define M_LAVA          6
#define M_SAND          7
#define M_LEAVES        8
#define M_SPONGE        9
#define M_GLASS         10
#define M_WOOL          11
#define M_IRON          12
#define M_TNT           13
#define M_REDSTONE      14
#define M_FIRE          15
#define M_SNOW          16
#define M_ICE           17
#define M_SNOWBLOCK     18
#define M_CACTUS        19
#define M_CLAY          20
#define M_PUMPKIN       21
#define M_PORTAL        22
#define M_CAKE          23
#define M_DIAMOND       24
#define M_GOLD          25
'''

    
for i in range(1,len(parts)):
    methods[i] += '    }\n}\n'
    print methods[i]
    
print '#endif\n\n'
