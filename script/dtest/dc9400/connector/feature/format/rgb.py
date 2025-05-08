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
    return 'connector%d.feature.format.'%id + subname

def __get_name_displayin(id, subname):
    return 'connector{}.feature.format.{}.displayin'.format(id, subname)

def __get_property():
    return OrderedDict()

def __get_conn_property():
    p = OrderedDict()
    p['wbPoint'] = 'displayIn'
    return OrderedDict([('WB_POINT', p)])

def __get_golden():
    return OrderedDict()

def __get_case(plane_id: int, format : str, refresh : int, resource="defaultPattern", width=640, height=480, crtc_id=0, connector_id=0, connector_format='XR24', plane_use_cus_format=0, con_use_cus_format=0):
    crtc = Crtc(crtc_id)
    if (plane_use_cus_format):
        plane_custom_format = OrderedDict()
        plane_custom_format['customFormat'] = 1
        plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property(), resource, ori_properties=plane_custom_format)
    else:
        plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property(), resource)
    if (con_use_cus_format):
        con_custom_format = OrderedDict()
        con_custom_format['customFormat'] = 1
        conn = Connector(connector_id, crtc, connector_format, width, height, refresh, golden=__get_golden(), other_prop=con_custom_format)
    else:
        conn = Connector(connector_id, crtc, connector_format, width, height, refresh, golden=__get_golden())
    if format == "RG24" and con_use_cus_format == 1:
        return DTestUnit(plane, conn, __get_name(connector_id, "RG24_planar"))
    elif format == "BG24" and con_use_cus_format == 1:
        return DTestUnit(plane, conn, __get_name(connector_id, "BG24_planar"))
    else:
        return DTestUnit(plane, conn, __get_name(connector_id, format))

def __get_case_displayin(plane_id: int, format : str, refresh : int, resource="defaultPattern", width=640, height=480, crtc_id=0, connector_id=0, connector_format='XR24', plane_use_cus_format=0, con_use_cus_format=0):
    crtc = Crtc(crtc_id)
    if (plane_use_cus_format):
        plane_custom_format = OrderedDict()
        plane_custom_format['customFormat'] = 1
        plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property(), resource, ori_properties=plane_custom_format)
    else:
        plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property(), resource)
    if (con_use_cus_format):
        con_custom_format = OrderedDict()
        con_custom_format['customFormat'] = 1
        conn = Connector(connector_id, crtc, connector_format, width, height, refresh, __get_conn_property(), golden=__get_golden(), other_prop=con_custom_format)
    else:
        conn = Connector(connector_id, crtc, connector_format, width, height, refresh, __get_conn_property(), golden=__get_golden())
    if format == "RG24" and con_use_cus_format == 1:
        return DTestUnit(plane, conn, __get_name_displayin(connector_id, "RG24_planar"))
    else:
        return DTestUnit(plane, conn, __get_name_displayin(connector_id, format))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return  [__get_case(0, 'AR24', 60, "00021_32bpp_formatARGB8888_640x480_LINEAR.bmp", 640, 480, 0, 0, connector_format='AR24')] \
            +[__get_case(0, 'AB24', 60, "00022_32bpp_formatABGR8888_640x480_LINEAR.bmp", 640, 480, 0, 0, connector_format='AB30')] \
            +[__get_case(0, 'RA24', 60, "00023_32bpp_formatRGBA8888_640x480_LINEAR.bmp", 640, 480, 0, 0, connector_format='RA24')] \
            +[__get_case(0, 'BA24', 60, "00024_32bpp_formatBGRA8888_640x480_LINEAR.bmp", 640, 480, 0, 0, connector_format='BA24')] \
            +[__get_case(0, 'AR30', 60, "32bpp_formatARGB2101010_1080x2340_LINEAR.bmp", 1080, 2340, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'AB30', 60, "00014_32bpp_formatABGR2101010_640x480_LINEAR.bmp", 640, 480, 0, 0, connector_format='AB30')] \
            +[__get_case(0, 'RA30', 60, "00015_32bpp_formatRGBA1010102_640x480_LINEAR.bmp", 640, 480, 0, 0, connector_format='RA30')] \
            +[__get_case(0, 'BA30', 60, "00016_32bpp_formatBGRA1010102_640x480_LINEAR.bmp", 640, 480, 0, 0, connector_format='BA30')] \
            +[__get_case(0, 'RG24', 60, "RGB888_linear_640x480.raw", 640, 480, 0, 0, connector_format='RG24')] \
            +[__get_case(0, 'BG24', 60, "BGR888_linear_640x480.raw", 640, 480, 0, 0, connector_format='BG24')] \
            +[__get_case(0, 'RG24', 60, "zero0_RGB888_Planar_640x480.raw", 640, 480, 0, 0, connector_format='RG24', plane_use_cus_format=1, con_use_cus_format=1)] \
            +[__get_case(0, 'BG24', 60, "flower_BGR888_Planar_640x480.raw", 640, 480, 0, 0, connector_format='BG24', plane_use_cus_format=1, con_use_cus_format=1)] \
            +[__get_case_displayin(0, 'AR24', 60, "00021_32bpp_formatARGB8888_640x480_LINEAR.bmp", 640, 480, 0, 0, connector_format='AR24')] \
            +[__get_case_displayin(0, 'AR30', 60, "32bpp_formatARGB2101010_1080x2340_LINEAR.bmp", 1080, 2340, 0, 0, connector_format='AR30')] \
            +[__get_case_displayin(0, 'RG24', 60, "RGB888_linear_640x480.raw", 640, 480, 0, 0, connector_format='RG24')] \
            +[__get_case_displayin(0, 'RG24', 60, "zero0_RGB888_Planar_640x480.raw", 640, 480, 0, 0, connector_format='RG24', plane_use_cus_format=1, con_use_cus_format=1)]