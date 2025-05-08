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

def __get_name(id, roi_info):
    return 'crtc{}.feature.lbox.roi_{}_{}_{}x{}'.format(id, roi_info['x'], \
    roi_info['y'], roi_info['w'], roi_info['h'])

def __get_property(roi_info):
    p = OrderedDict()
    p["color_red"] = roi_info['r']
    p["color_green"] = roi_info['g']
    p["color_blue"] = roi_info['b']
    p["roi_origin_x"] = roi_info['x']
    p["roi_origin_y"] = roi_info['y']
    p["roi_size_width"] = roi_info['w']
    p["roi_size_height"] = roi_info['h']
    return OrderedDict([('LBOX', p)])

def __get_golden():
    return OrderedDict()

def __get_case(crtc_id, roi_info):
    crtc = Crtc(crtc_id, __get_property(roi_info))
    plane = Plane(0, crtc, 'AR24', 1280, 720, 60, resource='VIV_Background_1280_720.bmp')
    conn = Connector(0, crtc, 'AR30', 1280, 720, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(crtc_id, roi_info))

def __set_lbox_info(r, g, b, x, y, w, h):
    roi_info = dict()

    roi_info['r'] = r
    roi_info['g'] = g
    roi_info['b'] = b
    roi_info['x'] = x
    roi_info['y'] = y
    roi_info['w'] = w
    roi_info['h'] = h
    return roi_info

@dtest_unit_group('dc9400')
def __gen(chip_info):
    lbox_list = []
    lbox_list.append(__set_lbox_info(1023, 0, 0, 0, 0, 420, 720))
    lbox_list.append(__set_lbox_info(0, 1023, 0, 420, 0, 420, 720))
    lbox_list.append(__set_lbox_info(0, 0, 1023, 860, 0, 420, 720))
    case_list = []
    for roi_info in lbox_list:
        if roi_info['w'] >4096 and roi_info['h'] >2160:##size can not exceed 4k(4096x2160)
            continue

        for i in range(3):
            case_list.append(__get_case(i, roi_info))

    return case_list

