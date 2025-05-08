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

def __get_name():
    return 'scenario_case5_4x3840x2160_120hz_8surfaces_rotate90_crtc23'

def __get_property_plane():
    p1 = OrderedDict()
    p1['hPhase'] = 16
    p1['vPhase'] = 4
    return OrderedDict([('UP_SAMPLE', p1)])

def __get_property_conn():
    p = OrderedDict()
    p['wbpoint'] = 'ofifoIn'
    return OrderedDict([('WB_POINT', p)])

def __get_golden():
    return OrderedDict()

def __get_case():
    f0_crtc2 = Crtc(2)
    f0_plane6 = Plane(6, f0_crtc2, 'AR24', 640, 480, 60, crtcX=0, crtcY=0, crtcWidth=640, crtcHeight=480)
    f0_conn0 = Connector(0, f0_crtc2, 'AR30', 640, 480, 60, golden=__get_golden())
    frame0 = DTestUnit(f0_plane6, f0_conn0, __get_name(), frameID=0)

    f1_crtc0 = Crtc(0); f1_crtc1 = Crtc(1); f1_crtc2 = Crtc(2); f1_crtc3 = Crtc(3)
    f1_plane0 = Plane(0, f1_crtc0, 'AR4H', 2160, 3840, 60, resource='FP16_2160x3840_TILE_8x4_UNIT_2x2.raw', crtcX=0, crtcY=0, \
                    tileMode="TILE_8X4_UNIT2X2", crtcWidth=3840, crtcHeight=2160, ori_properties=OrderedDict([('zpos', 0), ('rotation', '90')]))

    f1_plane2 = Plane(2, f1_crtc0, 'AR24', 2160, 3840, 60, resource='ARGB8888_2160x3840_TILE_8x8_UNIT2x2.raw', crtcX=0, crtcY=0, \
                    tileMode="TILE_8X8_UNIT2X2", crtcWidth=3840, crtcHeight=2160, ori_properties=OrderedDict([('zpos', 1), ('rotation', '90')]))

    f1_plane1 = Plane(1, f1_crtc1, 'AR4H', 3840, 2160, 60, resource='FP16_3840x2160_TILE_8x4_UNIT2x2_dec400_compress.raw', crtcX=0, crtcY=0, \
                    decType="DEC", tileMode="TILE_8X4_UNIT2X2", crtcWidth=3840, crtcHeight=2160, ori_properties=OrderedDict([('zpos', 0)]),\
                    tileStatus='FP16_3840x2160_TILE_8x4_UNIT2x2_dec400_compress.ts')

    f1_plane3 = Plane(3, f1_crtc1, 'P210_UV', 3840, 2160, 60, __get_property_plane(), resource='P210_UV_3840x2160_LINEAR.raw', crtcX=0, crtcY=0, \
                    crtcWidth=3840, crtcHeight=2160, ori_properties=OrderedDict([('zpos', 1), ('yuvColorEncoding', 'BT2020'), ('yuvColorRange', 'FULL')]))

    f1_plane6 = Plane(6, f1_crtc2, 'AR4H', 2160, 3840, 60, resource='FP16_2160x3840_TILE_8x4_UNIT_2x2.raw', crtcX=0, crtcY=0, \
                    tileMode="TILE_8X4_UNIT2X2", crtcWidth=3840, crtcHeight=2160, ori_properties=OrderedDict([('zpos', 0), ('rotation', '90')]))

    f1_plane8 = Plane(8, f1_crtc2, 'AR24', 2160, 3840, 60, resource='ARGB8888_2160x3840_TILE_8x8_UNIT2x2.raw', crtcX=0, crtcY=0, \
                    tileMode="TILE_8X8_UNIT2X2", crtcWidth=3840, crtcHeight=2160, ori_properties=OrderedDict([('zpos', 1), ('rotation', '90')]))

    f1_plane7 = Plane(7, f1_crtc3, 'AR4H', 3840, 2160, 60, resource='FP16_3840x2160_TILE_8x4_UNIT2x2_dec400_compress.raw', crtcX=0, crtcY=0, \
                    decType="DEC", tileMode="TILE_8X4_UNIT2X2", crtcWidth=3840, crtcHeight=2160, ori_properties=OrderedDict([('zpos', 0)]),\
                    tileStatus='FP16_3840x2160_TILE_8x4_UNIT2x2_dec400_compress.ts')

    f1_plane9 = Plane(9, f1_crtc3, 'P210_UV', 3840, 2160, 60, __get_property_plane(), resource='P210_UV_3840x2160_LINEAR.raw', crtcX=0, crtcY=0, \
                    crtcWidth=3840, crtcHeight=2160, ori_properties=OrderedDict([('zpos', 1), ('yuvColorEncoding', 'BT2020'), ('yuvColorRange', 'FULL')]))

    f1_planes = [f1_plane0, f1_plane1, f1_plane2, f1_plane3, f1_plane6, f1_plane7, f1_plane8, f1_plane9]

    cursor0 = Cursor(0, f1_crtc0, 'AR24', cursorPosX=0, cursorPosY=0, cursorWidth=64, cursorHeight=64, cursorHotX=0, cursorHotY=0)
    cursor1 = Cursor(1, f1_crtc1, 'AR24', cursorPosX=0, cursorPosY=0, cursorWidth=64, cursorHeight=64, cursorHotX=0, cursorHotY=0)
    cursor2 = Cursor(2, f1_crtc2, 'AR24', cursorPosX=0, cursorPosY=0, cursorWidth=64, cursorHeight=64, cursorHotX=0, cursorHotY=0)
    cursor3 = Cursor(3, f1_crtc3, 'AR24', cursorPosX=0, cursorPosY=0, cursorWidth=64, cursorHeight=64, cursorHotX=0, cursorHotY=0)
    cursors = [cursor0, cursor1, cursor2, cursor3]

    f1_conn0 = Connector(0, f1_crtc2, 'AR30', 3840, 2160, 60, __get_property_conn(), golden=__get_golden())
    f1_conn1 = Connector(1, f1_crtc3, 'AR30', 3840, 2160, 60, __get_property_conn(), golden=__get_golden())
    f1_conns = [f1_conn0, f1_conn1]
    frame1 = DTestUnit(f1_planes, f1_conns, __get_name(), frameID=1, cursors=cursors)
    frames = [frame0, frame1]
    return DTestFrameUnit(frames, __get_name())

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [__get_case()]