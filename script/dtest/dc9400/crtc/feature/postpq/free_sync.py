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

def __get_name(crtc_id):
    return 'crtc{}.feature.postpq.free_sync'.format(crtc_id)

def __get_property_crtc_config():
    p = OrderedDict()
    p['freeSyncType'] = 'config_finish'
    p['freeSyncMaxDelay'] = 1000
    p['freeSyncFinish'] = 1
    return OrderedDict([('FREE_SYNC', p)])

def __get_property_crtc_disable():
    p = OrderedDict()
    p['globalDisable'] = 1
    return OrderedDict([('FREE_SYNC', p)])

def __get_golden():
    return OrderedDict()

def __get_case(crtc_id):
    f0_crtc = Crtc(crtc_id, __get_property_crtc_config())
    f0_plane = Plane(0, f0_crtc, 'AR24', 640, 480, 60, crtcX=0, crtcY=0, crtcWidth=640, crtcHeight=480)
    f0_conn = Connector(0, f0_crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    frame0 = DTestUnit(f0_plane, f0_conn, __get_name(crtc_id), frameID=0)

    f1_crtc = Crtc(crtc_id, __get_property_crtc_disable())
    f1_plane = Plane(0, f1_crtc, 'AR24', 640, 480, 60, resource='00021_32bpp_formatARGB8888_640x480_LINEAR.bmp', crtcX=0, crtcY=0, \
                    crtcWidth=640, crtcHeight=480)
    f1_conn = Connector(0, f1_crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    frame1 = DTestUnit(f1_plane, f1_conn, __get_name(crtc_id), frameID=1)

    frames = [frame0, frame1]
    return DTestFrameUnit(frames, __get_name(crtc_id))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [__get_case(0), __get_case(1),  __get_case(2), __get_case(3)]
