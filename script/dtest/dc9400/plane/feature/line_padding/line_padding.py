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
import  re

def __get_name(id, subname):
    return 'plane%d.feature.line_padding.'%id + subname

def __get_property(mode, a, r, g, b):
    p = OrderedDict()
    p["mode"] = mode
    p["a"] = a
    p["r"] = r
    p["g"] = g
    p["b"] = b
    return OrderedDict([('LINE_PADDING', p)])

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

display_mode_list = ["640x480", "720x480", "800x600", "1280x720", "1024x768", "1280x1024", "720x1612", "1080x2400"]

def __get_mode_info(mode):
    modeinfo = dict()
    m = re.search("(\d+)x(\d+)", mode, re.I)
    if m:
        modeinfo['width'] = int(m.group(1))
        modeinfo['height'] = int(m.group(2))
    return modeinfo

def __get_case(plane_id: int, format : str, resource : str, decType : str, tileMode : str, mode : str, crtcHeight : int, a : int, r : int, g : int, b : int):

    conn_width = 640
    conn_height = crtcHeight
    for output_mode in display_mode_list:
        output_modeinfo = __get_mode_info(output_mode)
        if output_modeinfo['width'] >= 640 and output_modeinfo['height'] >= crtcHeight:
            conn_width = output_modeinfo['width']
            conn_height = output_modeinfo['height']
            break

    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, format, 640, 480, 60, __get_property(mode, a, r, g, b), resource, decType= decType, tileMode=tileMode, crtcWidth = 640, crtcHeight = crtcHeight)
    conn = Connector(0, crtc, 'AR30', conn_width, conn_height, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, "{}_{}.{}".format(format, tileMode, mode)))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return  [__get_case(0, "RG16", "00025_16bpp_formatRGB565_640x480_LINEAR.bmp",  "NONE", "LINEAR",   "2TO1", 960, 0, 16777215, 0, 0)] \
            +[__get_case(0, "YV12", "Boston_YV12_640X480_Linear.vimg",             "NONE", "LINEAR",   "4TO3", 640, 0, 0, 16777215, 0)] \
            +[__get_case(0, "AR24", "zero2_ARGB8888_640x480_tile32x8.raw",         "NONE", "TILE32X8", "1TO1", 500, 0, 0, 0, 16777215)] \
            +[__get_case(0, "NV12", "NV12_640x480_TILE32x8.raw",                   "NONE", "TILE32X8", "8TO5", 800, 0, 16777215, 16777215, 0)] \
            +[__get_case(1, "RG16", "00025_16bpp_formatRGB565_640x480_LINEAR.bmp", "NONE", "LINEAR",   "3TO1", 1440, 0, 16777215, 0, 0)] \
            +[__get_case(1, "YV12", "Boston_YV12_640X480_Linear.vimg",             "NONE", "LINEAR",   "5TO2", 1200, 0, 0, 16777215, 0)] \
            +[__get_case(1, "AR24", "zero2_ARGB8888_640x480_tile32x8.raw",         "NONE", "TILE32X8", "3TO2", 800, 0, 0, 0, 16777215)] \
            +[__get_case(1, "NV12", "NV12_640x480_TILE32x8.raw",                   "NONE", "TILE32X8", "5TO3", 900, 0, 0, 16777215, 16777215)] \
            +[__get_case(6, "RG16", "00025_16bpp_formatRGB565_640x480_LINEAR.bmp", "NONE", "LINEAR",   "3TO2", 720, 0, 16777215, 0, 0)] \
            +[__get_case(6, "YV12", "Boston_YV12_640X480_Linear.vimg",             "NONE", "LINEAR",   "5TO3", 800, 0, 0, 16777215, 0)] \
            +[__get_case(6, "AR24", "zero2_ARGB8888_640x480_tile32x8.raw",         "NONE", "TILE32X8", "3TO1", 1500, 0, 0, 0, 16777215)] \
            +[__get_case(6, "NV12", "NV12_640x480_TILE32x8.raw",                   "NONE", "TILE32X8", "5TO2", 1300, 0, 16777215, 0, 16777215)] \
            +[__get_case(7, "RG16", "00025_16bpp_formatRGB565_640x480_LINEAR.bmp", "NONE", "LINEAR",   "4TO1", 1920, 0, 16777215, 0, 0)] \
            +[__get_case(7, "YV12", "Boston_YV12_640X480_Linear.vimg",             "NONE", "LINEAR",   "8TO5", 768, 0, 0, 16777215, 0)] \
            +[__get_case(7, "AR24", "zero2_ARGB8888_640x480_tile32x8.raw",         "NONE", "TILE32X8", "2TO1", 1000, 0, 0, 0, 16777215)] \
            +[__get_case(7, "NV12", "NV12_640x480_TILE32x8.raw",                   "NONE", "TILE32X8", "4TO3", 700, 0, 16777215, 16777215, 16777215)]
