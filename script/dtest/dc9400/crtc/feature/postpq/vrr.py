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

    def __get_ori_property(enable, refresh):
        p_vrr = OrderedDict()
        p_vrr["vrrEnable"] = enable
        p_vrr["vrrRefresh"] = refresh
        return OrderedDict(p_vrr)

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc = Crtc(crtc_id, __get_property())
    plane = Plane(0, crtc, 'AR24', 1920, 1080, 120, crtcX = 0, crtcY = 0, crtcWidth = 1920, crtcHeight = 1080)
    conn0 = Connector(0, crtc, 'AR30', 1920, 1080, 120, golden=__get_golden())
    frame0 = DTestUnit(plane, conn0, __get_name(crtc_id, 'vrr'), frameID = 0)
    conn1 = Connector(0, crtc, 'AR30', 1920, 1080, 120, golden=__get_golden(), other_prop = __get_ori_property(1,60))
    frame1 = DTestUnit(plane, conn1, __get_name(crtc_id, 'vrr'), frameID = 1)
    conn2 = Connector(0, crtc, 'AR30', 1920, 1080, 120, golden=__get_golden(), other_prop = __get_ori_property(1,120))
    frame2 = DTestUnit(plane, conn2, __get_name(crtc_id, 'vrr'), frameID = 2)
    frames = [frame0, frame1, frame2]
    return DTestFrameUnit(frames, __get_name(crtc_id, 'vrr'))
@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(0)]
