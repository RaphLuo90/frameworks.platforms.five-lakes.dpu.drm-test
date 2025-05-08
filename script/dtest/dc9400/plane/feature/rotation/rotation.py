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

def __get_ori_property(rotation):
    p = OrderedDict()
    p["rotation"] = rotation
    return OrderedDict(p)

def __get_golden():
    return OrderedDict()

def __get_case(plane_id, format, width, height, refresh, resource, decType, tileMode, rotation, crtcWidth, crtcHeight):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property(), resource = resource, decType=  decType, tileMode = tileMode, \
                  crtcWidth = crtcWidth,  crtcHeight = crtcHeight, ori_properties = __get_ori_property(rotation))
    connWidth = 1920
    connHeight = 1200
    if width>connWidth or height >connHeight:
        connWidth = 2700
        connHeight = 2600
    conn = Connector(0, crtc, 'AR30', connWidth, connHeight, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'rotation{}.input{}x{}_{}_{}'.format(rotation, width, height, format, tileMode)))

def __get_info(res_name):
    res_name = res_name.lower()
    m = re.search("(\d+)x(\d+)_(tile\d+x\d+).*", res_name, re.I)
    m2 = re.search("(tile\d+x\d+)_(\d+)x(\d+).*", res_name, re.I)
    width = 0; height = 0; res_type = None
    if m:
        width = int(m.group(1))
        height = int(m.group(2))
        res_type = str(m.group(3)).upper()
    elif m2:
        width = int(m2.group(2))
        height = int(m2.group(3))
        res_type = str(m2.group(1)).upper()
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

    return width, height, res_type

@dtest_unit_group('dc9400')
def __gen(chip_info):
    resource_map = {
        'Boston_YV12_640X480_Linear.vimg': 'YV12',
        'NV12.640X480.LINEAR.raw': 'NV12',
        'P010_VU_LINEAR_640X480.raw': 'P010_UV',
        'NV16_LINEAR_640X480.raw': 'NV16',
        'NV12_Tile8x8_640x480_lossless.raw': 'NV12',
        'P010_Tile8x8_640x480_lossless.raw': 'P010_UV',
    }
    case_list = []
    rotation_list = ['90', '180', '270', 'FLIPX', 'FLIPY']
    plane_id_list = [0,1,6,7]
    for plane_id in plane_id_list:
        for key in resource_map.keys():
            width, height, res_type = __get_info(key)
            if res_type not in ['LINEAR', 'TILE8X8']:
                continue
            for rotation in rotation_list:
                if rotation in ['90', '270']:
                    if res_type in ['TILE8X8']:
                        case_list.append(__get_case(plane_id, resource_map.get(key), width, height, 60, key, "PVRIC", res_type, rotation, height, width))
                    else:
                        case_list.append(__get_case(plane_id, resource_map.get(key), width, height, 60, key, "NONE", res_type, rotation, height, width))
                else:
                    if res_type in ['TILE8X8']:
                        case_list.append(__get_case(plane_id, resource_map.get(key), width, height, 60, key, "PVRIC", res_type, rotation, width, height))
                    else:
                        case_list.append(__get_case(plane_id, resource_map.get(key), width, height, 60, key, "NONE", res_type, rotation, width, height))

    return case_list
