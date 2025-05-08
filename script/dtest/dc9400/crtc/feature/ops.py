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

def __get_name(crtc_id, subname):
    return 'crtc%d.feature.'%crtc_id + subname

def __get_case(crtc_id, subname):
    def __get_property():
        p = OrderedDict()
        p["curve_type"] = 'PQ'
        return OrderedDict([('OPS', p)])

    def __get_property_disable():
        p = OrderedDict()
        p["globalDisable"] = 1
        return OrderedDict([('OPS', p)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc0 = Crtc(crtc_id, __get_property())
    crtc0_frm1 = Crtc(crtc_id, __get_property())
    crtc0_frm2 = Crtc(crtc_id, __get_property_disable())
    plane0 = Plane(0, crtc0, 'AR24', 640, 480, 60, crtcX = 0, crtcY = 0, crtcWidth = 640, crtcHeight = 480)
    plane0_frm1 = Plane(0, crtc0_frm1, 'AR24', 640, 480, 60, crtcX = 0, crtcY = 0, crtcWidth = 640, crtcHeight = 480, resource = "00021_32bpp_formatARGB8888_640x480_LINEAR.bmp")
    plane0_frm2 = Plane(0, crtc0_frm2, 'AR24', 640, 480, 60, crtcX = 0, crtcY = 0, crtcWidth = 640, crtcHeight = 480, resource = "00021_32bpp_formatARGB8888_640x480_LINEAR.bmp")
    conn0 = Connector(0, crtc0, 'AR30', 640, 480, 60, golden=__get_golden())
    conn0_frm1 = Connector(0, crtc0_frm1, 'AR30', 640, 480, 60, golden=__get_golden())
    conn0_frm2 = Connector(0, crtc0_frm2, 'AR30', 640, 480, 60, golden=__get_golden())
    frame0 = DTestUnit([plane0], [conn0], __get_name(crtc_id, subname), frameID = 0)
    frame1 = DTestUnit([plane0_frm1], [conn0_frm1], __get_name(crtc_id, subname), frameID = 1)
    frame2 = DTestUnit([plane0_frm2], [conn0_frm2], __get_name(crtc_id, subname), frameID = 2)
    frames = [frame0, frame1, frame2]
    return DTestFrameUnit(frames, __get_name(crtc_id, subname))
@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(0, 'ops')]
