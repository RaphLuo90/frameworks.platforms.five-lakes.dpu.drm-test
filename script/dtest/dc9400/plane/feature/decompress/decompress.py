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
import re

def __get_name(id, subname):
    return 'plane{}.feature.{}'.format(id, subname)

def __get_property():
    return OrderedDict()

def __get_property_conn():
    p = OrderedDict()
    p["wbPoint"] = 'displayIn'
    return OrderedDict([('WB_POINT', p)])

def __get_golden():
    return OrderedDict()

def __get_case(plane_id, format, width, height, refresh, resource, decType, tileMode, lossy):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property(), resource =resource, decType=decType, tileMode=tileMode, ori_properties={'lossy': lossy})
    connWidth = 640
    connHeight = 480
    if width>connWidth or height >connHeight:
        connWidth = 2700
        connHeight = 2600
    conn = Connector(0, crtc, 'AR30', connWidth, connHeight, refresh, properties=__get_property_conn(), golden=__get_golden())
    if lossy:
        lossyString = 'lossy'
    else:
        lossyString = 'lossless'

    return DTestUnit(plane, conn, __get_name(plane_id, 'decompress_{}_{}'.format(lossyString, format)))


def __get_case_clear(plane_id, format, width, height, refresh, resource, decType, tileMode, lossy):
    def __get_property_clear():
        p = OrderedDict()
        p["decClearLow0"] = 1073676288
        p["decClearHigh0"] = 1073676288
        return OrderedDict([('PVRIC_CLEAR', p)])

    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property_clear(), resource=resource, decType=decType, tileMode=tileMode, ori_properties={'lossy': lossy})
    connWidth = 640
    connHeight = 480
    if width>connWidth or height >connHeight:
        connWidth = 2700
        connHeight = 2600
    conn = Connector(0, crtc, 'AR30', connWidth, connHeight, refresh, properties=__get_property_conn(), golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'decompress_clear'))


def __get_case_const(plane_id, format, width, height, refresh, resource, decType, tileMode, lossy):
    def __get_property_const():
        p = OrderedDict()
        p['is_yuv'] = 0
        p["decConstColor0"] = 108922494
        p["decConstColor1"] = 1073676288
        return OrderedDict([('PVRIC_CONST', p)])

    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property_const(), resource=resource, decType=decType, tileMode=tileMode, ori_properties={'lossy': lossy})
    connWidth = 640
    connHeight = 480
    if width>connWidth or height >connHeight:
        connWidth = 2700
        connHeight = 2600
    conn = Connector(0, crtc, 'AR30', connWidth, connHeight, refresh, properties=__get_property_conn(), golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'decompress_const'))


def __get_info(res_name):
    res_name = res_name.lower()
    m = re.search("(\d+)x(\d+)_(tile\d+x\d+)_(lossless|lossy).*", res_name, re.I)
    m2 = re.search("(tile\d+x\d+)_(\d+)x(\d+)_(lossless|lossy).*", res_name, re.I)
    width = 0; height = 0; res_type = None; lossy = 0
    if m:
        width = int(m.group(1))
        height = int(m.group(2))
        res_type = str(m.group(3)).upper()
        if (m.group(4)).lower() =='lossy':
            lossy = 1
        elif (m.group(4)).lower() =='lossless':
            lossy = 0

    elif m2:
        width = int(m2.group(2))
        height = int(m2.group(3))
        res_type = str(m2.group(1)).upper()
        if (m2.group(4)).lower() =='lossy':
            lossy = 1
        elif (m2.group(4)).lower() =='lossless':
            lossy = 0

    elif 'tile' not in res_name.lower():
        res_type = 'LINEAR'
        m3 = re.search("(\d+)x(\d+).*", res_name, re.I)
        m4 = re.search("(\d+)x(\d+)_*", res_name, re.I)
        if m3:
            width = int(m3.group(1))
            height = int(m3.group(2))
        elif m4:
            width = int(m4.group(1))
            height = int(m4.group(2))
    else:
        print("\nerror: not parse info for {}, please check manually!".format(res_name))

    return width, height, res_type, lossy

@dtest_unit_group('dc9400')
def __gen(chip_info):
    resource_map = {
        'ARGB8888_Tile16x4_640x480_lossless.raw': 'AR24',
        'ARGB16161616_Tile32x2_640x480_lossless.raw': 'AR4H',
        'ARGB2101010_Tile16x4_640x480_lossless.raw':'AR30',
        'RGB565_Tile16x4_640x480_lossless.raw': 'RG16',
        'NV12_Tile8x8_640x480_lossless.raw': 'NV12',
        'P010_Tile8x8_640x480_lossless.raw': 'P010_UV',
        'NV12_Tile8x8_640x480_lossy.raw': 'NV12',
        'P010_Tile8x8_640x480_lossy.raw': 'P010_UV'
    }
    case_list = []
    plane_id = 0
    refresh = 60
    decType = "PVRIC"
    for key in resource_map.keys():
        width, height, res_type, lossy = __get_info(key)
        case_list.append(__get_case(plane_id, resource_map.get(key), width, height, refresh, key, decType, res_type, lossy))

    return case_list \
            + [__get_case_clear(0, 'AR24', 640, 480, 60, 'ARGB8888_decClear0xFFFF0000_640x480.raw', 'PVRIC', 'TILE16X4', lossy=0) ] \
            + [__get_case_const(0, 'RG16', 640, 480, 60, 'RGB565_decConst0xFF00FF00_640x480.raw', 'PVRIC', 'TILE16X4', lossy=0)]
