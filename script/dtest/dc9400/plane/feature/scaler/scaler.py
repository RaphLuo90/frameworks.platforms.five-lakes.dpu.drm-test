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

def __get_name(id, width, height, dst_width, dst_height):
    return 'plane{}.feature.scaler_{:.1f}x{:.1f}.input_{}x{}_output_{}x{}'.format(id, dst_width/width,
    dst_height/height, width, height, dst_width, dst_height)

def __get_property(src_w, src_h, dst_w, dst_h):
    p = OrderedDict()
    p["src_w"] = src_w
    p["src_h"] = src_h
    p["dst_w"] = dst_w
    p["dst_h"] = dst_h
    p["filter"] = 'H9V5'
    return OrderedDict([('SCALER', p)])

def __get_golden():
    return OrderedDict()

display_mode_list = ["640x480x60", "1280x720x60", "1080x2340x60", "1920x1080x60", "1440x3520x60",
                   "2700x2600x60", "2500x2820x60", "2340x3404x60", "3200x1920x60", "3840x2160x60"]

def __get_mode_info(mode):
    modeinfo = dict()
    m = re.search("(\d+)x(\d+)x(\d+)", mode, re.I)
    if m:
        modeinfo['width'] = int(m.group(1))
        modeinfo['height'] = int(m.group(2))
        modeinfo['frame'] = int(m.group(3))
    return modeinfo

def __get_case(plane_id: int, format : str, refresh : int, resource="defaultPattern", width=640, height=480, crtc_id=0, connector_id=0, 
               dst_width=640, dst_height=480, connector_format='XR24'):
    conn_width = dst_width
    conn_height = dst_height
    in_refresh = refresh
    for output_mode in display_mode_list:
        output_modeinfo = __get_mode_info(output_mode)
        if output_modeinfo['width'] >= dst_width and output_modeinfo['height'] >= dst_height:
            conn_width = output_modeinfo['width']
            conn_height = output_modeinfo['height']
            in_refresh = output_modeinfo['frame']
            break

    crtc = Crtc(crtc_id)
    plane = Plane(plane_id, crtc, format, width, height, in_refresh, __get_property(width, height, dst_width, dst_height), resource,
                  crtcX = 0, crtcY = 0, crtcWidth = dst_width, crtcHeight = dst_height)
    conn = Connector(connector_id, crtc, connector_format, conn_width, conn_height, in_refresh, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, width, height, dst_width, dst_height))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return   [__get_case(0, 'AR24', 60, "00021_32bpp_formatARGB8888_640x480_LINEAR.bmp", 640, 480, 0, 0, 960, 240, connector_format='AR30')] \
            +[__get_case(1, 'AR24', 60, "defaultPattern", 2032, 1356, 0, 0, 1344, 2992, connector_format='AR30')]