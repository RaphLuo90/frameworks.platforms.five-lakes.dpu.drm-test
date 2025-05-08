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

    def __get_property_global_disable(prop_name):
        p = OrderedDict()
        p["globalDisable"] = 1
        return OrderedDict([(prop_name, p)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc0 = Crtc(crtc_id, __get_property())
    plane_full = Plane(0, crtc0, 'AR24', 1080, 2340, 60, crtcX = 0, crtcY = 0, crtcWidth = 1080, crtcHeight = 2340)
    plane_bt = Plane(1, crtc0, 'AR24', 1080, 132, 60, crtcX = 0, crtcY = 2208, crtcWidth = 1080, crtcHeight = 132, resource = "VV_Background_1920x1080.ARGB8.bmp")
    plane_tp = Plane(2, crtc0, 'AR24', 1080, 132, 60, crtcX = 0, crtcY = 0, crtcWidth = 1080, crtcHeight = 132, resource = "VV_Background_1920x1080.ARGB8.bmp")
    ###Frame0 plane0 enable###
    conn0 = Connector(0, crtc0, 'AR30', 1080, 2340, 60, golden=__get_golden())
    frame0 = DTestUnit(plane_full, conn0, __get_name('multiPlaneOffOnSwitch'), frameID = 0)
    ###Frame1 plane0/1/2 enable###
    conn1 = Connector(0, crtc0, 'AR30', 1080, 2340, 60, golden=__get_golden())
    frame1 = DTestUnit([plane_full, plane_tp, plane_bt], conn1, __get_name('multiPlaneOffOnSwitch'), frameID = 1)
    ###Frame2 plane0/2 enable plane1 disable###
    plane_bt_disable = Plane(1, crtc0, 'AR24', 1080, 132, 60, crtcX = 0, crtcY = 2208, crtcWidth = 1080, crtcHeight = 132, resource = "VV_Background_1920x1080.ARGB8.bmp", ori_properties=OrderedDict([('globalDisable', 1)]))
    conn2 = Connector(0, crtc0, 'AR30', 1080, 2340, 60, golden=__get_golden())
    frame2 = DTestUnit([plane_full, plane_tp, plane_bt_disable], conn2, __get_name('multiPlaneOffOnSwitch'), frameID = 2)
    ###Frame3 plane0/1 enable plane2 disable###
    plane_tp_disable = Plane(2, crtc0, 'AR24', 1080, 132, 60, crtcX = 0, crtcY = 0, crtcWidth = 1080, crtcHeight = 132, resource = "VV_Background_1920x1080.ARGB8.bmp", ori_properties=OrderedDict([('globalDisable', 1)]))
    conn3 = Connector(0, crtc0, 'AR30', 1080, 2340, 60, golden=__get_golden())
    frame3 = DTestUnit([plane_full, plane_tp_disable, plane_bt], conn2, __get_name('multiPlaneOffOnSwitch'), frameID = 3)
    frames = [frame0, frame1, frame2, frame3]
    return DTestFrameUnit(frames, __get_name('multiPlaneOffOnSwitch'))
@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(0)]
