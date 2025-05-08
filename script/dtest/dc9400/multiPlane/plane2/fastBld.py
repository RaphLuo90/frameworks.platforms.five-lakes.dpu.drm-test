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
    return 'multiPlane.%dplane.'%id + subname

def __get_property():
    return OrderedDict()

def __get_plane1_property(mode):
    p = OrderedDict()
    p["colorMode"] = mode
    return OrderedDict([('BLEND_MODE', p)])

def __get_conn_property(crtc_id):
    p1 = OrderedDict()
    if crtc_id == 4:
        p1["wbPoint"] = 'displayIn'
        return OrderedDict([('WB_POINT', p1)])
    else:
        return OrderedDict()

def __get_golden():
    return OrderedDict()

def __get_case(src_id, dst_id, crtc_id, mode):
    crtc = Crtc(crtc_id)
    if src_id ==4 or src_id ==5 or src_id ==10 or src_id ==11:
        plane0 = Plane(src_id, crtc, 'AR24', 256, 256, 60, __get_property(), resource = "ALPHA0xFF_COLOR0xFF0000_256x256.raw", \
                    crtcX=0, crtcY=80, crtcWidth=256, crtcHeight=256, ori_properties=OrderedDict([('zpos', 0)]))
    else:
        plane0 = Plane(src_id, crtc, 'AR24', 640, 480, 60, __get_property(), resource = "ALPHA0xFF_COLOR0xFF0000_640x480.raw", \
                    ori_properties=OrderedDict([('zpos', 0)]))
    if dst_id ==4 or dst_id ==5 or dst_id ==10 or dst_id ==11:
        plane1 = Plane(dst_id, crtc, 'AR24', 256, 256, 60, __get_plane1_property(mode), resource = "ALPHA0xFF_COLOR0x0000FF_256x256.raw",\
                    crtcX=30, crtcY=80, crtcWidth=256, crtcHeight=256, ori_properties=OrderedDict([('zpos', 1)]))
    else:
        plane1 = Plane(dst_id, crtc, 'AR24', 640, 480, 60, __get_plane1_property(mode), resource = "ALPHA0xFF_COLOR0x0000FF_640x480.raw",\
                    ori_properties=OrderedDict([('zpos', 1)]))

    planes = [plane0, plane1]
    conn = Connector(0, crtc, 'AR30', 640, 480, 60, __get_conn_property(crtc_id), golden=__get_golden())
    return DTestUnit(planes, conn, __get_name(2, 'p{}p{}_fastBld_{}'.format(src_id, dst_id, mode)))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    modelist = ['CLEAR', 'SRC', 'DST', 'SRC_OVER', 'DST_OVER', 'SRC_IN', 'DST_IN', 'SRC_OUT', 'DST_OUT', \
                'SRC_ATOP', 'DST_ATOP', 'XOR', 'PLUS', 'BLEND']
    case_list = []
    primary_list = [3, 4, 5, 9]
    for mode in modelist:
        case_list.append(__get_case(0, 1, 0, mode))
    for src_id in range(0, chip_info.plane_num):
        for dst_id in range(src_id+1, chip_info.plane_num):
            if (src_id in primary_list and dst_id in primary_list) or (src_id == 0 and dst_id == 1):
                continue
            if src_id in primary_list:
                crtc_id = primary_list.index(src_id)
            elif dst_id in primary_list:
                crtc_id = primary_list.index(dst_id)
            else:
                crtc_id = 0
            case_list.append(__get_case(src_id, dst_id, crtc_id, modelist[dst_id])) ##just to reduce case number, so chose fixed mode.
    return case_list
