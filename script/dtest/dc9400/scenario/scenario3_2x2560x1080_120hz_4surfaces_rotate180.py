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
    return 'scenario_case3_2x2560x1080_120hz_4surfaces_rotate180'

def __get_property_p0():
    p = OrderedDict()
    p['enable'] = 1
    p['src_w'] = 2560
    p['src_h'] = 1080
    p['dst_w'] = 3840
    p['dst_h'] = 2160
    p["filter"] = 'H9V5'
    return OrderedDict([('SCALER', p)])

def __get_property_p2():
    p_scale = OrderedDict()
    p_scale['enable'] = 1
    p_scale['src_w'] = 1920
    p_scale['src_h'] = 1080
    p_scale['dst_w'] = 2560
    p_scale['dst_h'] = 1080
    p_scale["filter"] = 'H9V5'
    p_up = OrderedDict()
    p_up['hPhase'] = 8
    p_up['vPhase'] = 8
    return OrderedDict([('SCALER', p_scale), ('UP_SAMPLE', p_up)])

def __get_property_conn():
    p = OrderedDict()
    p['wbpoint'] = 'ofifoIn'
    return OrderedDict([('WB_POINT', p)])

def __get_golden():
    return OrderedDict()

def __get_case():
    f0_crtc0 = Crtc(0)
    f1_crtc0 = Crtc(0)
    f1_crtc2 = Crtc(2)
    f0_plane0 = Plane(0, f0_crtc0, 'AR24', 640, 480, 60, crtcX=0, crtcY=0, crtcWidth=640, crtcHeight=480)
    f1_plane0 = Plane(0, f1_crtc0, 'AR30', 2560, 1080, 120, __get_property_p0(), crtcX=0, crtcY=0, \
                    crtcWidth=3840, crtcHeight=2160, ori_properties=OrderedDict([('zpos', 0)]))

    f1_plane2 = Plane(2, f1_crtc0, 'NV12', 1920, 1080, 120, __get_property_p2(), resource='NV12_1920x1080_linear.vimg', crtcX=0, crtcY=0, \
                    crtcWidth=2560, crtcHeight=1080, ori_properties=OrderedDict([('yuvColorEncoding', 'BT2020'), ('yuvColorRange', 'FULL'), ('zpos', 1)]))

    f1_plane6 = Plane(6, f1_crtc2, 'AR30', 2560, 1080, 120, __get_property_p0(), crtcX=0, crtcY=0, \
                    crtcWidth=3840, crtcHeight=2160, ori_properties=OrderedDict([('zpos', 0), ('rotation', '180')]))

    f1_plane8 = Plane(8, f1_crtc2, 'NV12', 1920, 1080, 120, __get_property_p2(), resource='NV12_1920x1080_linear.vimg', crtcX=0, crtcY=0, \
                    crtcWidth=2560, crtcHeight=1080, ori_properties=OrderedDict([('yuvColorEncoding', 'BT2020'), ('yuvColorRange', 'FULL'), ('zpos', 1), ('rotation', '180')]))

    cursor0 = Cursor(0, f1_crtc0, 'AR24', cursorPosX=0, cursorPosY=0, cursorWidth=16, cursorHeight=16, cursorHotX=0, cursorHotY=0)
    cursor2 = Cursor(2, f1_crtc2, 'AR24', cursorPosX=0, cursorPosY=0, cursorWidth=16, cursorHeight=16, cursorHotX=0, cursorHotY=0)

    f0_conn0 = Connector(0, f0_crtc0, 'AR30', 640, 480, 60, golden=__get_golden())
    f1_conn0 = Connector(0, f1_crtc0, 'AR30', 3840, 2160, 120, __get_property_conn(), golden=__get_golden())
    f1_conn1 = Connector(1, f1_crtc2, 'AR30', 3840, 2160, 120, __get_property_conn(), golden=__get_golden())

    f1_planes = [f1_plane0, f1_plane2, f1_plane6, f1_plane8]
    f1_cursors = [cursor0, cursor2]
    f1_conns = [f1_conn0, f1_conn1]

    frame0 = DTestUnit(f0_plane0, f0_conn0, __get_name(), frameID=0)
    frame1 = DTestUnit(f1_planes, f1_conns, __get_name(), frameID=1, cursors=f1_cursors)
    frames = [frame0, frame1]
    return DTestFrameUnit(frames, __get_name())

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [__get_case()]