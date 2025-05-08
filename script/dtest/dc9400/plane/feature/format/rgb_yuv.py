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
import re
from dtest.dc9400.chip_info import cursor_plane_ids

def __get_name(id, subname):
    return 'plane%d.feature.format.'%id + subname

def __get_property():
    return OrderedDict()

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

display_mode_list = ["640x480x60", "1080x2340x60", "1280x720x60", "1920x1080x60", "1440x3520x60",
                   "2700x2600x60", "2500x2820x60", "2340x3404x60", "3200x1920x60", "3840x2160x60"]

def __get_mode_info(mode):
    modeinfo = dict()
    m = re.search("(\d+)x(\d+)x(\d+)", mode, re.I)
    if m:
        modeinfo['width'] = int(m.group(1))
        modeinfo['height'] = int(m.group(2))
        modeinfo['frame'] = int(m.group(3))
    return modeinfo

def __get_case(plane_id: int, format : str, refresh : int, resource="defaultPattern", decType = "NONE", width=640, height=480, crtc_id=0, connector_id=0, connector_format='XR24', use_custom_format=0):
    conn_width = width
    conn_height = height
    in_refresh = refresh
    for output_mode in display_mode_list:
        output_modeinfo = __get_mode_info(output_mode)
        if output_modeinfo['width'] >= width and output_modeinfo['height'] >= height:
            conn_width = output_modeinfo['width']
            conn_height = output_modeinfo['height']
            in_refresh = output_modeinfo['frame']
            break

    crtc = Crtc(crtc_id)
    if plane_id in cursor_plane_ids:
        plane = Plane(plane_id, crtc, format, 256, 256, in_refresh, __get_property(), resource)
    else:
        if (use_custom_format):
            custom_format = OrderedDict()
            custom_format['customFormat'] = 1
            plane = Plane(plane_id, crtc, format, width, height, in_refresh, __get_property(), resource, decType, ori_properties=custom_format)
        else:
            plane = Plane(plane_id, crtc, format, width, height, in_refresh, __get_property(), resource, decType)
    conn = Connector(connector_id, crtc, connector_format, conn_width, conn_height, in_refresh, golden=__get_golden())
    if format == "RG24" and use_custom_format == 1:
        return DTestUnit(plane, conn, __get_name(plane_id, "RG24_planar"))
    elif format == "BG24" and use_custom_format == 1:
        return DTestUnit(plane, conn, __get_name(plane_id, "BG24_planar"))
    else:
        return DTestUnit(plane, conn, __get_name(plane_id, format))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return  [__get_case(0, 'RG16', 60, "00025_16bpp_formatRGB565_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'BG16', 60, "00026_16bpp_formatBGR565_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "00021_32bpp_formatARGB8888_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'AB24', 60, "00022_32bpp_formatABGR8888_640x480_LINEAR.bmp", 'NONE',  640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'RA24', 60, "00023_32bpp_formatRGBA8888_640x480_LINEAR.bmp", 'NONE',  640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'BA24', 60, "00024_32bpp_formatBGRA8888_640x480_LINEAR.bmp", 'NONE',  640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'XB24', 60, "00027_32bpp_formatXBGR8888_640x480_LINEAR.bmp", 'NONE',  640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'XR24', 60, "00028_32bpp_formatXRGB8888_640x480_LINEAR.bmp", 'NONE',  640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'BX24', 60, "00029_32bpp_formatBGRX8888_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'RX24', 60, "00030_32bpp_formatRGBX8888_640x480_LINEAR.bmp", 'NONE',  640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'AR15', 60, "00009_16bpp_formatARGB1555_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'AB15', 60, "00010_16bpp_formatABGR1555_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'RA15', 60, "00011_16bpp_formatRGBA5551_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'BA15', 60, "00012_16bpp_formatBGRA5551_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'AR12', 60, "defaultPattern", 'NONE',  640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'AB12', 60, "defaultPattern", 'NONE',  640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'RA12', 60, "defaultPattern", 'NONE',  640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'BA12', 60, "defaultPattern", 'NONE',  640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'AR30', 60, "00013_32bpp_formatARGB2101010_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'AB30', 60, "00014_32bpp_formatABGR2101010_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'RA30', 60, "00015_32bpp_formatRGBA1010102_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'BA30', 60, "00016_32bpp_formatBGRA1010102_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'YV12', 60, "Boston_YV12_640X480_Linear.vimg", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'YU12', 60, "Boston_YU12_640X480_Linear.vimg", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'NV12', 60, "NV12.640X480.LINEAR.raw", 'NONE', 640, 480, 0, 0,  connector_format='AR30')] \
            +[__get_case(0, 'NV21', 60, "NV21_LINEAR_640X480.raw", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'P010_UV', 60, "P010_VU_LINEAR_640X480.raw", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'NV16', 60, "NV16_LINEAR_640X480.raw", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'NV61', 60, "Crew_NV61_1280X720_Linear.vimg", 'NONE', 1280, 720, 0, 0, connector_format='AR30')] \
            +[__get_case(i, 'AR24', 60, "00021_32bpp_formatARGB8888_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30') for i in range(1, chip_info.plane_num)] \
            +[__get_case(i, 'YV12', 60, "Boston_YV12_640X480_Linear.vimg", 'NONE', 640, 480, 0, 0, connector_format='AR30') for i in (1,6,7) ] \
            +[__get_case(0, 'XR15', 60, "00001_16bpp_formatXRGB1555_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'XB15', 60, "00002_16bpp_formatXBGR1555_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0,  connector_format='AR30')] \
            +[__get_case(0, 'RX15', 60, "00003_16bpp_formatRGBX5551_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'BX15', 60, "00004_16bpp_formatBGRX5551_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'XR30', 60, "00013_32bpp_formatARGB2101010_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0,connector_format='AR30')] \
            +[__get_case(0, 'XB30', 60, "00014_32bpp_formatABGR2101010_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'RX30', 60, "00015_32bpp_formatRGBA1010102_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'BX30', 60, "00016_32bpp_formatBGRA1010102_640x480_LINEAR.bmp", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(1, 'AR30', 60, "32bpp_formatARGB2101010_1080x2340_LINEAR.bmp", 'NONE', 1080, 2340, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'P030_UV', 60, "P030_UV_1920x1080_Linear.raw", 'NONE', 1920, 1080, 0, 0, connector_format='AR30', use_custom_format=1)] \
            +[__get_case(0, 'P210_UV', 60, "P210_UV_1920x1080_LINEAR.raw", 'NONE', 1920, 1080, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'YUYV', 60, "zero2_YUY2_640X480_Linear.vimg", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'RG24', 60, "RGB888_linear_640x480.raw", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'BG24', 60, "BGR888_linear_640x480.raw", 'NONE', 640, 480, 0, 0, connector_format='AR30')] \
            +[__get_case(0, 'P010_VU', 60, "lotus_P010_VU_full_BT709_1920x1080.raw", 'NONE', 1920, 1080, 0, 0, connector_format='AR30', use_custom_format=1)] \
            +[__get_case(0, 'P210_VU', 60, "lake_P210_VU_full_BT709_1920x1080.raw", 'NONE', 1920, 1080, 0, 0, connector_format='AR30', use_custom_format=1)] \
            +[__get_case(0, 'RG24', 60, "zero0_RGB888_Planar_640x480.raw", 'NONE', 640, 480, 0, 0, connector_format='AR30', use_custom_format=1)] \
            +[__get_case(0, 'BG24', 60, "flower_BGR888_Planar_640x480.raw", 'NONE', 640, 480, 0, 0, connector_format='AR30', use_custom_format=1)] \
            +[__get_case(0, 'I010_UV', 60, "exige_I010_UV_1280x720.raw", 'NONE', 1280, 720, 0, 0, connector_format='AR30', use_custom_format=1)] \
            +[__get_case(0, 'I010_VU', 60, "exige_I010_VU_1280x720.raw", 'NONE', 1280, 720, 0, 0, connector_format='AR30', use_custom_format=1)] \
            +[__get_case(0, 'LUMA_8', 60, "YUV_LUMA_8_LINEAR_640X480.raw", 'NONE', 640, 480, 0, 0, connector_format='NV16')] \
            +[__get_case(0, 'LUMA_10', 60, "YUV_LUMA_10_LINEAR_640X480.raw", 'NONE', 640, 480, 0, 0, connector_format='P010_UV', use_custom_format=1)] \
            +[__get_case(0, 'AR4H', 60, "exige_FP16_640x480_LINEAR.raw", 'NONE', 640, 480, 0, 0, connector_format='AR30')]