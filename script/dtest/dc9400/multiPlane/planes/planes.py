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
    return 'multiPlane.%dplane.'%id + subname

def __get_plane_property():
    p = OrderedDict()
    p['colorMode'] = 'SRC'
    return OrderedDict([('BLEND_MODE', p)])

def __get_crtc_property():
    p = OrderedDict()
    p['a'] = 0
    p['r'] = 0
    p['g'] = 0
    p['b'] = 16777215
    return OrderedDict([('BG_COLOR', p)])

def __get_golden():
    return OrderedDict()

def __get_case(plane_number, add_bg_color):
    if add_bg_color:
        crtc = Crtc(0, __get_crtc_property())
        name = __get_name(plane_number, 'addbg')
    else:
        crtc = Crtc(0)
        name = __get_name(plane_number, 'nobg')

    plane0 = Plane(0, crtc, 'AR24', 1280, 720, 60, resource='ARGB8888_linear_1280x720_bear.bmp', crtcX=0, crtcY=0, crtcWidth=1280, crtcHeight=720, ori_properties=OrderedDict([('zpos', 0)]))
    plane1 = Plane(1, crtc, 'AR24', 1280, 720, 60, __get_plane_property(), resource='ARGB8888_linear_1280x720_bear.bmp', crtcX=1920, crtcY=0, crtcWidth=1280, crtcHeight=720, ori_properties=OrderedDict([('zpos', 3)]))
    plane2 = Plane(2, crtc, 'AR24', 640, 480, 60, __get_plane_property(), resource='zero2_ARGB8.bmp', crtcX=980, crtcY=420, crtcWidth=640, crtcHeight=480, ori_properties=OrderedDict([('zpos', 1)]))
    plane3 = Plane(3, crtc, 'AR24', 640, 480, 60, __get_plane_property(), resource='zero2_ARGB8.bmp', crtcX=2900, crtcY=420, crtcWidth=640, crtcHeight=480, ori_properties=OrderedDict([('zpos', 4)]))
    plane4 = Plane(4, crtc, 'AR24', 256, 256, 60, __get_plane_property(), crtcX=824, crtcY=620, crtcWidth=256, crtcHeight=256, ori_properties=OrderedDict([('zpos', 2)]))
    plane5 = Plane(5, crtc, 'AR24', 256, 256, 60, __get_plane_property(), crtcX=2744, crtcY=620, crtcWidth=256, crtcHeight=256, ori_properties=OrderedDict([('zpos', 5)]))
    plane6 = Plane(6, crtc, 'AR24', 1280, 720, 60, __get_plane_property(), resource='ARGB8888_linear_1280x720_bear.bmp', crtcX=0, crtcY=1080, crtcWidth=1280, crtcHeight=720, ori_properties=OrderedDict([('zpos', 6)]))
    plane7 = Plane(7, crtc, 'AR24', 1280, 720, 60, __get_plane_property(), resource='ARGB8888_linear_1280x720_bear.bmp', crtcX=1920, crtcY=1080, crtcWidth=1280, crtcHeight=720, ori_properties=OrderedDict([('zpos', 9)]))
    plane8 = Plane(8, crtc, 'AR24', 640, 480, 60, __get_plane_property(), resource='zero2_ARGB8.bmp', crtcX=980, crtcY=1500, crtcWidth=640, crtcHeight=480, ori_properties=OrderedDict([('zpos', 7)]))
    plane9 = Plane(9, crtc, 'AR24', 640, 480, 60, __get_plane_property(), resource='zero2_ARGB8.bmp', crtcX=2900, crtcY=1500, crtcWidth=640, crtcHeight=480, ori_properties=OrderedDict([('zpos', 10)]))
    plane10 = Plane(10, crtc, 'AR24', 256, 256, 60, __get_plane_property(), crtcX=824, crtcY=1700, crtcWidth=256, crtcHeight=256, ori_properties=OrderedDict([('zpos', 8)]))
    plane11 = Plane(11, crtc, 'AR24', 256, 256, 60, __get_plane_property(), crtcX=2744, crtcY=1700, crtcWidth=256, crtcHeight=256, ori_properties=OrderedDict([('zpos', 11)]))
    plane_list = [plane0, plane2, plane4, plane1, plane3, plane5, plane6, plane8, plane10, plane7, plane9, plane11]
    planes = plane_list[0:plane_number]
    if plane_number == 3:
        conn = Connector(0, crtc, 'AR30', 1920, 1080, 60, golden=__get_golden())
    else:
        conn = Connector(0, crtc, 'AR30', 3840, 2160, 60, golden=__get_golden())
    return DTestUnit(planes, conn, name)

@dtest_unit_group('dc9400')
def __gen(chip_info):
    case_list = list()
    plane_numbers = [3, 4, 5, 6, 7, 8, 9, 10, 11, 12]
    for plane_number in plane_numbers:
        case_list.append(__get_case(plane_number, False))
    case_list.append(__get_case(12, True))
    return case_list