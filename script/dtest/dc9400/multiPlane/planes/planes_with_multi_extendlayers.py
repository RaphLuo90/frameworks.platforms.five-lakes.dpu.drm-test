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

def __get_plane0_property(format):
    p = OrderedDict()
    p["format"] = format
    p["decType"] = 'NONE'
    p["tileMode"] = 'LINEAR'
    p["resourceEx0"] = 'defaultPattern'
    p["width_Ex0"] = 320
    p["height_Ex0"] = 240
    p["layerOutEx0X"] = 0
    p["layerOutEx0Y"] = 740
    p["layerOutEx0Width"] = 320
    p["layerOutEx0Height"] = 240
    p["resourceEx1"] = 'defaultPattern'
    p["width_Ex1"] = 320
    p["height_Ex1"] = 240
    p["layerOutEx1X"] = 0
    p["layerOutEx1Y"] = 1000
    p["layerOutEx1Width"] = 320
    p["layerOutEx1Height"] = 240
    p["resourceEx2"] = 'defaultPattern'
    p["width_Ex2"] = 320
    p["height_Ex2"] = 240
    p["layerOutEx2X"] = 0
    p["layerOutEx2Y"] = 1260
    p["layerOutEx2Width"] = 320
    p["layerOutEx2Height"] = 240
    return OrderedDict([('MULTI_EXT_LAYER', p)])

def __get_plane1_property(format):
    p = OrderedDict()
    p["format"] = format
    p["decType"] = 'NONE'
    p["tileMode"] = 'LINEAR'
    p["resourceEx0"] = 'defaultPattern'
    p["width_Ex0"] = 320
    p["height_Ex0"] = 240
    p["layerOutEx0X"] = 1200
    p["layerOutEx0Y"] = 920
    p["layerOutEx0Width"] = 320
    p["layerOutEx0Height"] = 240
    p["resourceEx1"] = 'defaultPattern'
    p["width_Ex1"] = 320
    p["height_Ex1"] = 240
    p["layerOutEx1X"] = 1200
    p["layerOutEx1Y"] = 1180
    p["layerOutEx1Width"] = 320
    p["layerOutEx1Height"] = 240
    p["resourceEx2"] = 'defaultPattern'
    p["width_Ex2"] = 320
    p["height_Ex2"] = 240
    p["layerOutEx2X"] = 1200
    p["layerOutEx2Y"] = 1440
    p["layerOutEx2Width"] = 320
    p["layerOutEx2Height"] = 240

    p1 = OrderedDict()
    p1['colorMode'] = 'SRC'
    return OrderedDict([('MULTI_EXT_LAYER', p), ('BLEND_MODE', p1)])

def __get_plane2_property(format):
    p = OrderedDict()
    p["format"] = format
    p["decType"] = 'NONE'
    p["tileMode"] = 'LINEAR'
    p["resourceEx0"] = 'defaultPattern'
    p["width_Ex0"] = 320
    p["height_Ex0"] = 240
    p["layerOutEx0X"] = 824
    p["layerOutEx0Y"] = 920
    p["layerOutEx0Width"] = 320
    p["layerOutEx0Height"] = 240
    p["resourceEx1"] = 'defaultPattern'
    p["width_Ex1"] = 320
    p["height_Ex1"] = 240
    p["layerOutEx1X"] = 824
    p["layerOutEx1Y"] = 1180
    p["layerOutEx1Width"] = 320
    p["layerOutEx1Height"] = 240
    p["resourceEx2"] = 'defaultPattern'
    p["width_Ex2"] = 320
    p["height_Ex2"] = 240
    p["layerOutEx2X"] = 824
    p["layerOutEx2Y"] = 1440
    p["layerOutEx2Width"] = 320
    p["layerOutEx2Height"] = 240

    p1 = OrderedDict()
    p1['colorMode'] = 'SRC'
    return OrderedDict([('MULTI_EXT_LAYER', p), ('BLEND_MODE', p1)])

def __get_plane3_property(format):
    p = OrderedDict()
    p["format"] = format
    p["decType"] = 'NONE'
    p["tileMode"] = 'LINEAR'
    p["resourceEx0"] = 'defaultPattern'
    p["width_Ex0"] = 320
    p["height_Ex0"] = 240
    p["layerOutEx0X"] = 1920
    p["layerOutEx0Y"] = 740
    p["layerOutEx0Width"] = 320
    p["layerOutEx0Height"] = 240
    p["resourceEx1"] = 'defaultPattern'
    p["width_Ex1"] = 320
    p["height_Ex1"] = 240
    p["layerOutEx1X"] = 1920
    p["layerOutEx1Y"] = 1000
    p["layerOutEx1Width"] = 320
    p["layerOutEx1Height"] = 240
    p["resourceEx2"] = 'defaultPattern'
    p["width_Ex2"] = 320
    p["height_Ex2"] = 240
    p["layerOutEx2X"] = 1920
    p["layerOutEx2Y"] = 1260
    p["layerOutEx2Width"] = 320
    p["layerOutEx2Height"] = 240

    p1 = OrderedDict()
    p1['colorMode'] = 'SRC'
    return OrderedDict([('MULTI_EXT_LAYER', p), ('BLEND_MODE', p1)])

def __get_plane4_property(format):
    p = OrderedDict()
    p["format"] = format
    p["decType"] = 'NONE'
    p["tileMode"] = 'LINEAR'
    p["resourceEx0"] = 'defaultPattern'
    p["width_Ex0"] = 320
    p["height_Ex0"] = 240
    p["layerOutEx0X"] = 3120
    p["layerOutEx0Y"] = 920
    p["layerOutEx0Width"] = 320
    p["layerOutEx0Height"] = 240
    p["resourceEx1"] = 'defaultPattern'
    p["width_Ex1"] = 320
    p["height_Ex1"] = 240
    p["layerOutEx1X"] = 3120
    p["layerOutEx1Y"] = 1180
    p["layerOutEx1Width"] = 320
    p["layerOutEx1Height"] = 240
    p["resourceEx2"] = 'defaultPattern'
    p["width_Ex2"] = 320
    p["height_Ex2"] = 240
    p["layerOutEx2X"] = 3120
    p["layerOutEx2Y"] = 1440
    p["layerOutEx2Width"] = 320
    p["layerOutEx2Height"] = 240

    p1 = OrderedDict()
    p1['colorMode'] = 'SRC'
    return OrderedDict([('MULTI_EXT_LAYER', p), ('BLEND_MODE', p1)])

def __get_plane5_property(format):
    p = OrderedDict()
    p["format"] = format
    p["decType"] = 'NONE'
    p["tileMode"] = 'LINEAR'
    p["resourceEx0"] = 'defaultPattern'
    p["width_Ex0"] = 320
    p["height_Ex0"] = 240
    p["layerOutEx0X"] = 2744
    p["layerOutEx0Y"] = 920
    p["layerOutEx0Width"] = 320
    p["layerOutEx0Height"] = 240
    p["resourceEx1"] = 'defaultPattern'
    p["width_Ex1"] = 320
    p["height_Ex1"] = 240
    p["layerOutEx1X"] = 2744
    p["layerOutEx1Y"] = 1180
    p["layerOutEx1Width"] = 320
    p["layerOutEx1Height"] = 240
    p["resourceEx2"] = 'defaultPattern'
    p["width_Ex2"] = 320
    p["height_Ex2"] = 240
    p["layerOutEx2X"] = 2744
    p["layerOutEx2Y"] = 1440
    p["layerOutEx2Width"] = 320
    p["layerOutEx2Height"] = 240

    p1 = OrderedDict()
    p1['colorMode'] = 'SRC'
    return OrderedDict([('MULTI_EXT_LAYER', p), ('BLEND_MODE', p1)])

def __get_golden():
    return OrderedDict()

def __get_case(plane_number):
    crtc = Crtc(0)
    plane0 = Plane(0, crtc, 'AR24', 1280, 720, 60, __get_plane0_property('AR24'), resource='ARGB8888_linear_1280x720_bear.bmp', crtcX=0, crtcY=0, crtcWidth=1280, crtcHeight=720, ori_properties=OrderedDict([('zpos', 0)]))
    plane1 = Plane(1, crtc, 'AR24', 1280, 720, 60, __get_plane3_property('AR24'), resource='ARGB8888_linear_1280x720_bear.bmp', crtcX=1920, crtcY=0, crtcWidth=1280, crtcHeight=720, ori_properties=OrderedDict([('zpos', 3)]))
    plane2 = Plane(2, crtc, 'AR24', 640, 480, 60, __get_plane1_property('AR24'), resource='zero2_ARGB8.bmp', crtcX=980, crtcY=420, crtcWidth=640, crtcHeight=480, ori_properties=OrderedDict([('zpos', 1)]))
    plane3 = Plane(3, crtc, 'AR24', 640, 480, 60, __get_plane4_property('AR24'), resource='zero2_ARGB8.bmp', crtcX=2900, crtcY=420, crtcWidth=640, crtcHeight=480, ori_properties=OrderedDict([('zpos', 4)]))
    plane4 = Plane(4, crtc, 'AR24', 256, 256, 60, __get_plane2_property('AR24'), crtcX=824, crtcY=620, crtcWidth=256, crtcHeight=256, ori_properties=OrderedDict([('zpos', 2)]))
    plane5 = Plane(5, crtc, 'AR24', 256, 256, 60, __get_plane5_property('AR24'), crtcX=2744, crtcY=620, crtcWidth=256, crtcHeight=256, ori_properties=OrderedDict([('zpos', 5)]))
    plane_list = [plane0, plane2, plane4, plane1, plane3, plane5]
    planes = plane_list[0:plane_number]
    conn = Connector(0, crtc, 'AR30', 3840, 2160, 60, golden=__get_golden())
    name = __get_name(plane_number, 'with_18_multi_extendlayer')
    return DTestUnit(planes, conn, name)

@dtest_unit_group('dc9400')
def __gen(chip_info):
    case_list = list()
    case_list.append(__get_case(6))
    return case_list