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

def __get_case(crtc_id):
    def __get_property():
        return OrderedDict()

    def __get_pri_gamma_property(prop_name):
        p = OrderedDict()
        p["in_bit"] = 14
        p["out_bit"] = 14
        p["seg_point"] = [ 2, 4, 16, 128, 256, 512, 2048 ]
        p["seg_step"] = [ 1, 2, 4, 8, 16, 8, 128, 256 ]
        return OrderedDict([(prop_name, p)])

    def __get_property_global_disable(prop_name):
        p = OrderedDict()
        p["globalDisable"] = 1
        return OrderedDict([(prop_name, p)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc0 = Crtc(crtc_id, __get_property())
    crtc1 = Crtc(crtc_id, __get_pri_gamma_property('PRIOR_GAMMA'))
    crtc2 = Crtc(crtc_id, __get_property_global_disable('PRIOR_GAMMA'))
    f0_plane = Plane(0, crtc0, 'AR24', 1920, 1080, 60, crtcX = 0, crtcY = 0, crtcWidth = 1920, crtcHeight = 1080)
    f1_plane = Plane(0, crtc1, 'AR24', 1920, 1080, 60, crtcX = 0, crtcY = 0, crtcWidth = 1920, crtcHeight = 1080)
    f2_plane = Plane(0, crtc2, 'AR24', 1920, 1080, 60, crtcX = 0, crtcY = 0, crtcWidth = 1920, crtcHeight = 1080)
    f3_plane = Plane(0, crtc0, 'AR24', 1920, 1080, 60, crtcX = 0, crtcY = 0, crtcWidth = 1920, crtcHeight = 1080)
    conn0 = Connector(0, crtc0, 'AR30', 1920, 1080, 60, golden=__get_golden())
    frame0 = DTestUnit(f0_plane, conn0, __get_name('pqGammaOffOnSwitch'), frameID = 0)
    conn1 = Connector(0, crtc1, 'AR30', 1920, 1080, 60, golden=__get_golden())
    frame1 = DTestUnit(f1_plane, conn1, __get_name('pqGammaOffOnSwitch'), frameID = 1)
    conn2 = Connector(0, crtc2, 'AR30', 1920, 1080, 60, golden=__get_golden())
    frame2 = DTestUnit(f2_plane, conn2, __get_name('pqGammaOffOnSwitch'), frameID = 2)
    conn3 = Connector(0, crtc0, 'AR30', 1920, 1080, 60, golden=__get_golden(), other_prop=OrderedDict([('globalDisable', 1)]))
    frame3 = DTestUnit(f3_plane, conn3, __get_name('pqGammaOffOnSwitch'), frameID = 3)
    frames = [frame0, frame1, frame2, frame3]
    return DTestFrameUnit(frames, __get_name('pqGammaOffOnSwitch'))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(0)]
