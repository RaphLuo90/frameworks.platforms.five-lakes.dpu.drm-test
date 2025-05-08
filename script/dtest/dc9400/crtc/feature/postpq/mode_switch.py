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
    return 'crtc%d.feature.postpq.'%id + subname

def __get_case(crtc_id):
    def __get_property():
        return OrderedDict()

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc = Crtc(crtc_id, __get_property())
    plane0 = Plane(0, crtc, 'AR24', 640, 480, 60, crtcX = 0, crtcY = 0, crtcWidth = 640, crtcHeight = 480)
    conn0 = Connector(0, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    frame0 = DTestUnit(plane0, conn0, __get_name(crtc_id, 'mode_switch'), frameID = 0)

    plane1 = Plane(0, crtc, 'AR24', 1920, 1080, 60, crtcX = 0, crtcY = 0, crtcWidth = 1920, crtcHeight = 1080)
    conn1 = Connector(0, crtc, 'AR30', 1920, 1080, 60, golden=__get_golden())
    frame1 = DTestUnit(plane1, conn1, __get_name(crtc_id, 'mode_switch'), frameID = 1)

    plane2 = Plane(0, crtc, 'AR24', 640, 480, 60, crtcX = 0, crtcY = 0, crtcWidth = 640, crtcHeight = 480)
    conn2 = Connector(0, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    frame2 = DTestUnit(plane2, conn2, __get_name(crtc_id, 'mode_switch'), frameID = 2)
    frames = [frame0, frame1, frame2]
    return DTestFrameUnit(frames, __get_name(crtc_id, 'mode_switch'))
@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(0), __get_case(1), __get_case(2), __get_case(3) ]
