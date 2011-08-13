#!/usr/bin/python

#DON'T OVERRITE THE ACTUAL FILE WITH THIS FILE'S OUTPUT
#kept because it might be useful if minecraft's protocol
#ever changes radically (unlikely)

from re import *
from string import *

f = open('packetdump', 'r')

packets = []

lastname = ''
while True:
    line = f.readline()
    if line == '': 
        break
    m = search('===([^=\(]*)',line)
    if m: lastname = strip(m.group(1))
    if search('wikitable',line) != None:
        f.readline()
        if search('Packet ID',f.readline()): #foundone
            while not search('row1',f.readline()): pass
            id = ''
            fields = []
            while True:
                line = f.readline()
                if not search('class=',line): break
                if search('col0',line):
                    m = search('0x(..)',line)
                    if m: 
                        id = m.group(1)
                        line = f.readline()  
                    else:
                        m1 = search('" \\| (.*)',line)
                        m2 = search('" \\| (.*)',f.readline())
                        if m1 and m2 and not search("Total Size",m1.group(1)): fields.append((m1.group(1),m2.group(1))) 
                if search('col1',line):
                    m1 = search('" \\| (.*)',line)
                    m2 = search('" \\| (.*)',f.readline())
                    if m1 and m2: fields.append((m1.group(1),m2.group(1))) 
            packets.append((replace(lower(lastname),' ','_'), id, fields))
               
header = ''
 
for lastname,id,fields in packets:
    header += 'typedef struct { unsigned char id; ' + '; '.join([type + ' ' + name for name,type in fields]) + '} p_' + lastname + ';\n'

parsers = ''
indent = '            '

for lastname,id,fields in packets:
    parsers += indent + 'case 0x'+id+':{\n'
    parsers += indent + '    p_' + lastname + ' *p = new p_' + lastname + ';\n'
    for name,type in fields:
        parsers += indent + '    p->' + name + ' = io.r_' + type + '();\n'
    parsers += indent + '    if (!io.working) break;\n'
    parsers += indent + '    client->packet((p_generic*)p);\n'
    parsers += indent+'} break;\n'

writers = ''
indent = '        '

for lastname,id,fields in packets:
    writers += indent + 'case 0x'+id+':{\n'
    writers += indent + '    p_' + lastname + ' *p = (p_' + lastname + '*)packet;\n'
    for name,type in fields:
        writers += indent + '    io.w_' + type + '(p->' + name + ');\n'
    writers += indent+'} break;\n'
    
defines = ''
for lastname,id,fields in packets:
    defines += '#define send_'+lastname+'('+','.join(['_'+name for name,type in fields])+') { \\\n        p_'+lastname+' p; \\\n        p.id = 0x' + id + '; \\\n'
    for name,type in fields:
        defines += '        p.' + name + ' = (_' + name + '); \\\n'
    defines += '        write_packet((p_generic*)&p); \\\n    }\n'
    
    
    
print defines
f.close()
