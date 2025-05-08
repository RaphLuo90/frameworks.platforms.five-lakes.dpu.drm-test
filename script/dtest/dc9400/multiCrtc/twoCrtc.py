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
    return 'multiCrtc.' + subname

def __get_case(crtc_0, crtc_1, subname):
    def __get_property():
        return OrderedDict()

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc0 = Crtc(crtc_0, __get_property())
    crtc1 = Crtc(crtc_1, __get_property())
    plane0 = Plane(0, crtc0, 'AR24', 640, 480, 60, crtcX = 0, crtcY = 0, crtcWidth = 640, crtcHeight = 480)
    plane1 = Plane(1, crtc1, 'AR24', 640, 480, 60, crtcX = 0, crtcY = 0, crtcWidth = 640, crtcHeight = 480)
    plane0_oth = Plane(0, crtc0, 'AR24', 640, 480, 60, crtcX = 0, crtcY = 0, crtcWidth = 640, crtcHeight = 480, resource = "00021_32bpp_formatARGB8888_640x480_LINEAR.bmp")
    plane1_oth = Plane(1, crtc1, 'AR24', 640, 480, 60, crtcX = 0, crtcY = 0, crtcWidth = 640, crtcHeight = 480, resource = "00021_32bpp_formatARGB8888_640x480_LINEAR.bmp")
    conn0 = Connector(0, crtc0, 'AR30', 640, 480, 60, golden=__get_golden())
    conn1 = Connector(1, crtc1, 'AR30', 640, 480, 60, golden=__get_golden())
    conn0_disable = Connector(0, crtc0, 'AR30', 640, 480, 60, golden=__get_golden(), other_prop=OrderedDict([('globalDisable', 1)]))
    conn1_disable = Connector(1, crtc1, 'AR30', 640, 480, 60, golden=__get_golden(), other_prop=OrderedDict([('globalDisable', 1)]))
    frame0 = DTestUnit([plane0], [conn0], __get_name(subname), frameID = 0)
    frame1 = DTestUnit([plane0, plane1_oth], [conn0, conn1], __get_name(subname), frameID = 1)
    frame2 = DTestUnit([plane0_oth, plane1], [conn0, conn1], __get_name(subname), frameID = 2)
    frame3 = DTestUnit([plane0, plane1], [conn0_disable, conn1_disable], __get_name(subname), frameID = 3)
    frames = [frame0, frame1, frame2, frame3]
    return DTestFrameUnit(frames, __get_name(subname))
@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(0, 1,'crtc01.on'), __get_case(2, 3, 'crtc23.on')]
