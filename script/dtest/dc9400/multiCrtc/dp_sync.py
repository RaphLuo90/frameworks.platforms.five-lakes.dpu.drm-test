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

def __get_name(subname):
    return 'multiCrtc.dp_sync.' + subname

def __get_property_crtc(crtc_id0, crtc_id1):
    p = OrderedDict()
    p['dpSyncCrtcMask'] = 'crtc{}, crtc{}'.format(crtc_id0, crtc_id1)
    return OrderedDict([('DP_SYNC', p)])

def __get_property_crtc_disable():
    p = OrderedDict()
    p['globalDisable'] = 1
    return OrderedDict([('DP_SYNC', p)])

def __get_golden():
    return OrderedDict()

def __get_case(crtc_id0, crtc_id1):
    f0_crtc0 = Crtc(crtc_id0)
    f0_plane0 = Plane(0, f0_crtc0, 'AR24', 640, 480, 60, crtcX=0, crtcY=0, crtcWidth=640, crtcHeight=480)
    f0_conn0 = Connector(0, f0_crtc0, 'AR30', 640, 480, 60, golden=__get_golden())
    frame0 = DTestUnit(f0_plane0, f0_conn0, __get_name('crtc{}{}'.format(crtc_id0, crtc_id1)), frameID=0)

    f1_crtc0 = Crtc(crtc_id0, __get_property_crtc(crtc_id0, crtc_id1))
    f1_crtc1 = Crtc(crtc_id1)
    f1_plane0 = Plane(0, f1_crtc0, 'AR24', 640, 480, 60, resource='00021_32bpp_formatARGB8888_640x480_LINEAR.bmp', crtcX=0, crtcY=0, \
                    crtcWidth=640, crtcHeight=480)
    f1_plane1 = Plane(1, f1_crtc1, 'AR24', 640, 480, 60,  crtcX=0, crtcY=0, crtcWidth=640, crtcHeight=480)
    f1_conn0 = Connector(0, f1_crtc0, 'AR30', 640, 480, 60, golden=__get_golden())
    f1_conn1 = Connector(1, f1_crtc1, 'AR30', 640, 480, 60, golden=__get_golden())
    f1_planes = [f1_plane0, f1_plane1]
    f1_conns = [f1_conn0, f1_conn1]
    frame1 = DTestUnit(f1_planes, f1_conns, __get_name('crtc{}{}'.format(crtc_id0, crtc_id1)), frameID=1)

    f2_crtc0 = Crtc(crtc_id0, __get_property_crtc_disable())
    f2_crtc1 = Crtc(crtc_id1)
    f2_plane0 = Plane(0, f2_crtc0, 'AR24', 640, 480, 60, resource='00021_32bpp_formatARGB8888_640x480_LINEAR.bmp', crtcX=0, crtcY=0, \
                    crtcWidth=640, crtcHeight=480)
    f2_plane1 = Plane(1, f2_crtc1, 'AR24', 640, 480, 60,  crtcX=0, crtcY=0, crtcWidth=640, crtcHeight=480)
    f2_conn0 = Connector(0, f2_crtc0, 'AR30', 640, 480, 60, golden=__get_golden())
    f2_conn1 = Connector(1, f2_crtc1, 'AR30', 640, 480, 60, golden=__get_golden())
    f2_planes = [f2_plane0, f2_plane1]
    f2_conns = [f2_conn0, f2_conn1]
    frame2 = DTestUnit(f2_planes, f2_conns, __get_name('crtc{}{}'.format(crtc_id0, crtc_id1)), frameID=2)

    frames = [frame0, frame1, frame2]
    return DTestFrameUnit(frames, __get_name('crtc{}{}'.format(crtc_id0, crtc_id1)))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [__get_case(0, 1), __get_case(2, 3)]
