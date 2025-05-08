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
    return 'plane{}.feature.{}'.format(id, subname)

def __get_property():
    return OrderedDict()

def __get_ori_property(yuvTransform):
    p1 = OrderedDict()
    p1["yuv_transform"] = yuvTransform
    return OrderedDict(p1)

def __get_golden():
    return OrderedDict()

display_mode_list = ["640x480x60", "1280x720x60", "1920x1080x60", "3200x1920x60",
                    "3840x2160x60", "4096x2160x60"]
def __get_case_dec400a(plane_id, crtc_id, format, width, height, refresh, resource, decType, tileMode, yuvTransform):
    crtc = Crtc(crtc_id)
    plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property(), resource =resource, decType=decType, tileMode=tileMode, ori_properties=__get_ori_property(yuvTransform))
    connWidth = 1280
    connHeight = 720
    if width > 1280 and height > 720:
        connWidth = width
        connHeight = height
    conn = Connector(crtc_id, crtc, 'AR24', connWidth, connHeight, refresh, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'dec400a_{}_{}_{}x{}_yuvTransform{}'.format(format, tileMode, width, height, yuvTransform)))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    resource_list = [ ##resource, format, width, height, decType, tileMode, yuvTransform
        ['512x512_lena_a8r8g8b8_tile32x8_yuvtransform0.raw', 'AR24', 512, 512, 'DEC400A', 'TILE_32X8_A', 0],
        ['512x512_lena_a8r8g8b8_tile32x8_yuvtransform1.raw', 'AR24', 512, 512, 'DEC400A', 'TILE_32X8_A', 1],

        ['512x512_lena_a2r10g10b10_tile32x8_yuvtransform0.raw', 'AR30', 512, 512, 'DEC400A', 'TILE_32X8_A', 0],
        ['512x512_lena_a2r10g10b10_tile32x8_yuvtransform1.raw', 'AR30', 512, 512, 'DEC400A', 'TILE_32X8_A', 1],

        ['512x512_lena_nv12_tile32x8_yuvtransform0.raw', 'NV12_DEC', 512, 512, 'DEC400A', 'TILE_32X8_A', 0],
        ['512x512_lena_p010_tile32x8_yuvtransform0.raw', 'P010_DEC', 512, 512, 'DEC400A', 'TILE_32X8_A', 0],

        ['3840x2160_a8r8g8b8_tile32x8_yuvtransform0.raw', 'AR24', 3840, 2160, 'DEC400A', 'TILE_32X8_A', 0],
        ['3840x2160_a8r8g8b8_tile32x8_yuvtransform1.raw', 'AR24', 3840, 2160, 'DEC400A', 'TILE_32X8_A', 1],

        ['3840x2160_a2r10g10b10_tile32x8_yuvtransform0.raw', 'AR30', 3840, 2160, 'DEC400A', 'TILE_32X8_A', 0],
        ['3840x2160_a2r10g10b10_tile32x8_yuvtransform1.raw', 'AR30', 3840, 2160, 'DEC400A', 'TILE_32X8_A', 1],

        ['3840x2160_nv12_tile32x8_yuvtransform0.raw', 'NV12_DEC', 3840, 2160, 'DEC400A', 'TILE_32X8_A', 0],
        ['3840x2160_p010_tile32x8_yuvtransform0.raw', 'P010_DEC', 3840, 2160, 'DEC400A', 'TILE_32X8_A', 0],

        ['4096x2160_a8r8g8b8_tile32x8_yuvtransform0.raw', 'AR24', 4096, 2160, 'DEC400A', 'TILE_32X8_A', 0],
        ['4096x2160_a8r8g8b8_tile32x8_yuvtransform1.raw', 'AR24', 4096, 2160, 'DEC400A', 'TILE_32X8_A', 1],

        ['4096x2160_a2r10g10b10_tile32x8_yuvtransform0.raw', 'AR30', 4096, 2160, 'DEC400A', 'TILE_32X8_A', 0],
        ['4096x2160_a2r10g10b10_tile32x8_yuvtransform1.raw', 'AR30', 4096, 2160, 'DEC400A', 'TILE_32X8_A', 1],

        ['4096x2160_nv12_tile32x8_yuvtransform0.raw', 'NV12_DEC', 4096, 2160, 'DEC400A', 'TILE_32X8_A', 0],
        ['4096x2160_p010_tile32x8_yuvtransform0.raw', 'P010_DEC', 4096, 2160, 'DEC400A', 'TILE_32X8_A', 0]
    ]
    case_list = []
    plane_id = 0
    crtc_id = 0
    refresh = 60
    for info in resource_list:
        case_list.append(__get_case_dec400a(plane_id, crtc_id, info[1], info[2], info[3], refresh, info[0], info[4], info[5], info[6]))

    return case_list
