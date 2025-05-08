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
    return 'multiFrame.' + subname

def __get_property(src_w, src_h, dst_w, dst_h):
    p = OrderedDict()
    p["src_w"] = src_w
    p["src_h"] = src_h
    p["dst_w"] = dst_w
    p["dst_h"] = dst_h
    p["filter"] = 'H9V5'
    return OrderedDict([('SCALER', p)])

def __get_case(crtc_id, subname):

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc0 = Crtc(crtc_id)
    plane0 = Plane(0, crtc0, 'AR24', 640, 480, 60,  __get_property(640, 480, 3840, 2160), crtcX = 0, crtcY = 0, crtcWidth = 3840, crtcHeight = 2160)
    plane1 = Plane(1, crtc0, 'AR24', 1280, 720, 60, __get_property(1280, 720, 3840, 2160), crtcX = 0, crtcY = 0, crtcWidth = 3840, crtcHeight = 2160)
    plane2 = Plane(2, crtc0, 'AR24', 1920, 1080, 60, __get_property(1920, 1080, 3840, 2160), crtcX = 0, crtcY = 0, crtcWidth = 3840, crtcHeight = 2160)
    conn0 = Connector(0, crtc0, 'AR30', 3840, 2160, 60, golden=__get_golden())
    conn1 = Connector(0, crtc0, 'AR30', 3840, 2160, 60, golden=__get_golden())
    conn2 = Connector(0, crtc0, 'AR30', 3840, 2160, 60, golden=__get_golden())
    frame0 = DTestUnit([plane0], [conn0], __get_name(subname), frameID = 0)
    frame1 = DTestUnit([plane1], [conn1], __get_name(subname), frameID = 1)
    frame2 = DTestUnit([plane2], [conn2], __get_name(subname), frameID = 2)
    frames = [frame0, frame1, frame2]
    return DTestFrameUnit(frames, __get_name(subname))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(0, 'p0p1p2.scaler')]