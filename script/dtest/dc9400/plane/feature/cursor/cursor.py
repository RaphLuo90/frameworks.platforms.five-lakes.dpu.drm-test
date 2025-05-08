#############################################################################
#    Copyright 2012 - 2023 Vivante Corporation, Santa Clara, California.    #
#    All Rights Reserved.                                                   #
#                                                                           #
#    Permission is hereby granted, free of charge, to any person obtaining  #
#    a copy of this software and associated documentation files (the        #
#    'Software'), to deal in the Software without restriction, including    #
#    without limitation the rights to use, copy, modify, merge, publish,    #
#    distribute, sub license, and/or sell copies of the Software, and to    #
#    permit persons to whom the Software is furnished to do so, subject     #
#    to the following conditions:                                           #
#                                                                           #
#    The above copyright notice and this permission notice (including the   #
#    next paragraph) shall be included in all copies or substantial         #
#    portions of the Software.                                              #
#                                                                           #
#    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,        #
#    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     #
#    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.#
#    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY       #
#    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   #
#    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      #
#    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 #
#                                                                           #
#############################################################################

from collections import OrderedDict
from dtest.helper import *

def __get_name(id, subname):
    return 'plane%d.feature.'%id + subname

def __get_property():
    return OrderedDict()

def __get_golden():
    return OrderedDict()

def __get_case(plane_id, cursor, cursorPosX, cursorPosY):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'AR24', width=cursor, height=cursor, refresh=60, properties=__get_property(), \
            crtcX=cursorPosX, crtcY=cursorPosY, crtcWidth=cursor, crtcHeight=cursor)
    conn = Connector(0, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, '{}x{}_PX{}_PY{}_p{}'.format(cursor, cursor, cursorPosX, cursorPosY, plane_id)))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    case_list = list()
    cursor_list = [8, 64, 128, 256]
    cursor_pos_list =  [(0,0), (61, 53), (384, 224)]
    plane_list = [4, 5, 10, 11] #only plane 4,5,10,11 support cursor
    plane_id = 0
    count = 0

    for cursor in cursor_list:
        for pi in cursor_pos_list:
            cursorPosX = pi[0]
            cursorPosY = pi[1]

            if plane_id in plane_list:
                case_list.append(__get_case(plane_id, cursor, cursorPosX, cursorPosY))

            count += 1
            plane_id = plane_list[count % 4] ### change pland id for each cases
    return case_list