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
    return 'connector%d.feature.postwb.r2y.'%id + subname

def __get_property():
    return OrderedDict()

def __get_connector_decType(decType):
    p1 = OrderedDict()
    p1["decType"] = decType
    return OrderedDict(p1)

def __get_property_R2Y_crtc():
    p = OrderedDict()
    p['mode'] = 'USR'
    p['gamut'] = 'BT2020'
    p['coef'] = [1076, 2777, 243, -572, -1476, 2048, 2048, -1883, -165, 0, 0, 0, 0, 2048, 2048]
    return OrderedDict([('R2Y', p)])

def __get_golden():
    return OrderedDict()

def __get_case(plane_id: int, format : str, refresh : int, resource="defaultPattern", width=640, height=480, crtc_id=0, connector_id=0, connector_format='XR24'):
    crtc = Crtc(crtc_id)
    plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property(), resource)
    conn = Connector(connector_id, crtc, connector_format, width, height, refresh, properties=__get_property_R2Y_crtc(), golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(connector_id, connector_format))

def __get_other_case(plane_id: int, format : str, refresh : int, resource="defaultPattern", decType="NONE", width=640, height=480, crtc_id=0, connector_id=0, connector_format='XR24'):
    crtc = Crtc(crtc_id)
    plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property(), resource, 'NONE')
    custom_format = OrderedDict()
    custom_format['decType'] = decType
    custom_format['customFormat'] = 1
    conn = Connector(connector_id, crtc, connector_format, width, height, refresh, properties=__get_property_R2Y_crtc(), other_prop=custom_format, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(connector_id, connector_format))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return  [__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 0, 0, connector_format='YV12')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 0, 0, connector_format='YU12')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 0, 0, connector_format='NV12')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 0, 0, connector_format='NV21')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 0, 0, connector_format='NV16')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 0, 0, connector_format='NV61')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 0, 0, connector_format='P010_UV')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 0, 0, connector_format='P210_UV')] \
            +[__get_other_case(0, 'AR24', 60, "VV_Background_1920x1080.ARGB8.bmp", 'NONE', 1920, 1080, 0, 0, connector_format='P030_UV')] \
            +[__get_other_case(0, 'AR24', 60, "defaultPattern", 'NONE', 640, 480, 0, 0, connector_format='P010_VU')] \
            +[__get_other_case(0, 'AR24', 60, "defaultPattern", 'NONE', 640, 480, 0, 0, connector_format='P210_VU')] \
            +[__get_other_case(0, 'AR24', 60, "defaultPattern", 'NONE', 640, 480, 0, 0, connector_format='I010_UV')] \
            +[__get_other_case(0, 'AR24', 60, "defaultPattern", 'NONE', 640, 480, 0, 0, connector_format='I010_VU')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 1, 1, connector_format='YV12')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 1, 1, connector_format='YU12')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 1, 1, connector_format='NV12')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 1, 1, connector_format='NV21')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 1, 1, connector_format='NV16')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 1, 1, connector_format='NV61')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 1, 1, connector_format='P010_UV')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 640, 480, 1, 1, connector_format='P210_UV')] \
            +[__get_other_case(0, 'AR24', 60, "VV_Background_1920x1080.ARGB8.bmp", 'NONE', 1920, 1080, 1, 1, connector_format='P030_UV')] \
            +[__get_other_case(0, 'AR24', 60, "defaultPattern", 'NONE', 640, 480, 1, 1, connector_format='P010_VU')] \
            +[__get_other_case(0, 'AR24', 60, "defaultPattern", 'NONE', 640, 480, 1, 1, connector_format='P210_VU')] \
            +[__get_other_case(0, 'AR24', 60, "defaultPattern", 'NONE', 640, 480, 1, 1, connector_format='I010_UV')] \
            +[__get_other_case(0, 'AR24', 60, "defaultPattern", 'NONE', 640, 480, 1, 1, connector_format='I010_VU')]
