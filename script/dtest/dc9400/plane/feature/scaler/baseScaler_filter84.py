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

def __get_name(id, scale_mode, input_modeinfo, output_modeinfo, scale_w, scale_h):
    return 'plane{}.feature.scaler{}_{}x{}.input_{}x{}_output_{}x{}'.format(id, scale_mode, scale_w, scale_h,\
                    input_modeinfo['width'], input_modeinfo['height'], output_modeinfo['width'], output_modeinfo['height'])

def __get_property(input_modeinfo, output_modeinfo, scale_mode):
    p = OrderedDict()
    p["src_w"] = input_modeinfo['width']
    p["src_h"] = input_modeinfo['height']
    p["dst_w"] = output_modeinfo['width']
    p["dst_h"] = output_modeinfo['height']
    p["filter"] = 'H8V4'
    p["mode"] = scale_mode
    return OrderedDict([('SCALER', p)])

def __get_golden():
    return OrderedDict()

def __get_case(plane_id, input_modeinfo, output_modeinfo, scale_w, scale_h, scale_mode):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'AR24', input_modeinfo['width'], input_modeinfo['height'], input_modeinfo['frame'], __get_property(input_modeinfo, output_modeinfo, scale_mode), \
                    crtcX = 0, crtcY = 0, crtcWidth = output_modeinfo['width'], crtcHeight = output_modeinfo['height'])
    conn = Connector(0, crtc, 'AR30', output_modeinfo['width'], output_modeinfo['height'], output_modeinfo['frame'], golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, scale_mode, input_modeinfo, output_modeinfo, scale_w, scale_h))

def __get_mode_info(mode):
    modeinfo = dict()
    m = re.search("(\d+)x(\d+)x(\d+)", mode, re.I)
    if m:
        modeinfo['width'] = int(m.group(1))
        modeinfo['height'] = int(m.group(2))
        modeinfo['frame'] = int(m.group(3))
    return modeinfo

@dtest_unit_group('dc9400')
def __gen(chip_info):
    display_mode_list = ["640x480x60", "3840x2160x60"]
    scale_modes_list = ['LINEAR', 'CUBIC', 'LANCZOS']
    case_list = []
    for input_mode in display_mode_list:
        input_modeinfo = __get_mode_info(input_mode)
        if input_modeinfo['width'] >4096 and input_modeinfo['height'] >2160:##size can not exceed 4k(4096x2160)
            continue
        for output_mode in display_mode_list:
            for scale_mode in scale_modes_list:
                output_modeinfo = __get_mode_info(output_mode)
                if output_modeinfo['width'] > 4096 and output_modeinfo['height'] > 2160:##size can not exceed 4k(4096x2160)
                    continue
                scale_w = round(output_modeinfo['width']/input_modeinfo['width'], 1)
                scale_h = round(output_modeinfo['height']/input_modeinfo['height'], 1)
                if scale_w >= 0.25 and scale_w <= 8 and scale_h >= 0.25 and scale_h <= 8:
                    if scale_w == 1 and scale_h == 1:
                        continue
                    case_list.append(__get_case(0, input_modeinfo, output_modeinfo, scale_w, scale_h, scale_mode))
    return case_list