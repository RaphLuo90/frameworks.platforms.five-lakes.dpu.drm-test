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
import  re

def __get_name(id, subname):
    return 'plane%d.feature.format.'%id + subname

def __get_property():
    return OrderedDict()

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

display_mode_list = ["640x480x60", "1280x720x60", "1920x1080x60", "1440x3520x60",
                   "2700x2600x60", "2500x2820x60", "2340x3404x60", "3200x1920x60", "3840x2160x60"]

def __get_mode_info(mode):
    modeinfo = dict()
    m = re.search("(\d+)x(\d+)x(\d+)", mode, re.I)
    if m:
        modeinfo['width'] = int(m.group(1))
        modeinfo['height'] = int(m.group(2))
        modeinfo['frame'] = int(m.group(3))
    return modeinfo

def __get_case(plane_id: int, format : str, refresh : int, resource="defaultPattern", width=640, height=480, crtc_id=0, connID=0, connFormat='AR30', decType="NONE",tileMode = "LINEAR", tileStatus=None, use_custom_format=0):
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
    if (use_custom_format):
        custom_format = OrderedDict()
        custom_format['customFormat'] = 1
        plane = Plane(plane_id, crtc, format, width, height, in_refresh, __get_property(), resource,  decType= decType, tileMode=tileMode, tileStatus=tileStatus, ori_properties=custom_format)
    else:
        plane = Plane(plane_id, crtc, format, width, height, in_refresh, __get_property(), resource,  decType= decType, tileMode=tileMode, tileStatus=tileStatus)
    conn = Connector(connID, crtc, connFormat, conn_width, conn_height, in_refresh, golden=__get_golden())
    if decType == "PVRIC":
        return DTestUnit(plane, conn, __get_name(plane_id, "{}_{}".format(format, tileMode)))
    else:
        return DTestUnit(plane, conn, __get_name(plane_id, "{}_{}_{}".format(format, tileMode, decType)))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return  [__get_case(i, 'BA24', 60, "BGRA8888_Tile16x4_640x480_lossless.raw", 640, 480, 0, 0, connFormat='AR30', decType="PVRIC", tileMode= "TILE16X4", tileStatus=None) for i in range(0, chip_info.plane_num)] \
            +[__get_case(0, 'BX24', 60, "BGRX8888_Tile16x4_640x480_lossless.raw", 640, 480, 0, 0, connFormat='AR30', decType="PVRIC", tileMode= "TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'RG16', 60, "RGB565_Tile16x4_640x480_lossless.raw", 640, 480, 0, 0, connFormat='AR30', decType="PVRIC", tileMode= "TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'BG16', 60, "BGR565_Tile16x4_640x480_lossless.raw", 640, 480, 0, 0, connFormat='AR30', decType="PVRIC", tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'AR30', 60, "ARGB2101010_Tile16x4_640x480_lossless.raw", 640, 480, 0, 0, connFormat='AR30', decType="PVRIC",tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'AB30', 60, "ABGR2101010_Tile16x4_640x480_lossless.raw", 640, 480, 0, 0, connFormat='AR30', decType="PVRIC",tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'RA30', 60, "RGBA1010102_Tile16x4_640x480_lossless.raw", 640, 480, 0, 0, connFormat='AR30', decType="PVRIC", tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'BA30', 60, "BGRA1010102_Tile16x4_640x480_lossless.raw", 640, 480, 0, 0, connFormat='AR30', decType="PVRIC", tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'AR4H', 60, "ARGB16161616_Tile32x2_640x480_lossless.raw", 640, 480, 0, 0, connFormat='AR30', decType="PVRIC", tileMode="TILE32X2", tileStatus=None)] \
            +[__get_case(i, 'NV12', 60, "NV12_Tile8x8_640x480_lossless.raw", 640, 480, 0, 0, connFormat='AR30', decType="PVRIC", tileMode="TILE8X8", tileStatus=None) for i in (0,1,6,7)] \
            +[__get_case(i, 'P010_UV', 60, "P010_Tile8x8_640x480_lossless.raw", 640, 480, 0, 0, connFormat='AR30', decType="PVRIC", tileMode = "TILE8X8", tileStatus=None) for i in (0,1,6,7) ] \
            +[__get_case(0, 'AR30', 60, "ARGB2101010_640x480_TILE_8x8_UNIT2x2.raw", 640, 480, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_UNIT2X2", tileStatus=None)] \
            +[__get_case(0, 'AR24', 60, "Lena_ARGB8888_512x512_TILE_8x8_UNIT2x2.raw", 512, 512, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_UNIT2X2", tileStatus=None)] \
            +[__get_case(0, 'AR4H', 60, "exige_FP16_640x480_TILE_8x4_UNIT2x2.raw", 640, 480, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X4_UNIT2X2", tileStatus=None)] \
            +[__get_case(0, 'AR30', 60, "ARGB2101010_640x480_TILE_8x8_UNIT2x2_dec400_compress.raw", 640, 480, 0, 0, connFormat='AR30', decType="DEC", tileMode="TILE_8X8_UNIT2X2", tileStatus="ARGB2101010_640x480_TILE_8x8_UNIT2x2_dec400.ts")] \
            +[__get_case(0, 'AR24', 60, "Lena_ARGB8888_512x512_TILE_8x8_UNIT2x2_dec400_compress.raw", 512, 512, 0, 0, connFormat='AR30', decType="DEC", tileMode="TILE_8X8_UNIT2X2", tileStatus="Lena_ARGB8888_512x512_TILE_8x8_UNIT2x2_dec400.ts")] \
            +[__get_case(0, 'AR4H', 60, "exige_FP16_640x480_TILE_8x4_UNIT2x2_dec400_compress.raw", 640, 480, 0, 0, connFormat='AR30', decType="DEC", tileMode="TILE_8X4_UNIT2X2", tileStatus="exige_FP16_640x480_TILE_8x4_UNIT2x2_dec400.ts")] \
            +[__get_case(0, 'NV12', 60, "NV12_tile16x16_1088x1920.raw", 1088, 1920, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE16X16", tileStatus=None)] \
            +[__get_case(0, 'P010_UV', 60, "formatP010_2560x1440_Tile16x16.raw", 2560, 1440, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE16X16", tileStatus=None)] \
            +[__get_case(0, 'NV12', 60, "NV12_1280x720_TILE32x8_A.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE32X8_A", tileStatus=None)]\
            +[__get_case(0, 'NV21', 60, "NV21_1280x720_TILE32x8_A.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE32X8_A", tileStatus=None)] \
            +[__get_case(0, 'AR24', 60, "ARGB8888_1280x720_TILE16x4.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'AB24', 60, "ABGR8888_1280x720_TILE16x4.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'RA24', 60, "RGBA8888_1280x720_TILE16x4.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'BA24', 60, "BGRA8888_1280x720_TILE16x4.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'AR30', 60, "ARGB2101010_1280x720_TILE16x4.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'AB30', 60, "ABGR2101010_1280x720_TILE16x4.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'RA30', 60, "RGBA2101010_1280x720_TILE16x4.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'BA30', 60, "BGRA2101010_1280x720_TILE16x4.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE16X4", tileStatus=None)] \
            +[__get_case(0, 'XR24', 60, "nature_XRGB8888_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'XB24', 60, "nature_XBGR8888_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'RX24', 60, "nature_RGBX8888_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'BX24', 60, "nature_BGRX8888_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'AR30', 60, "nature_ARGB2101010_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'AB30', 60, "nature_ABGR2101010_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'RA30', 60, "nature_RGBA1010102_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'BA30', 60, "nature_BGRA1010102_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'NV12', 60, "NV12_1280x720_TILE32x8_YUVSP8x8.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_32X8_YUVSP8X8", tileStatus=None)] \
            +[__get_case(0, 'NV21', 60, "NV21_1280x720_TILE32x8_YUVSP8x8.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_32X8_YUVSP8X8", tileStatus=None)] \
            +[__get_case(0, 'P010_UV', 60, "P010_1280x720_TILE16x8_YUVSP8x8.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_16X8_YUVSP8X8", tileStatus=None)] \
            +[__get_case(0, 'P010_VU', 60, "P010_VU_1280x720_TILE16x8_YUVSP8x8.raw", 1280, 720, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_16X8_YUVSP8X8", tileStatus=None, use_custom_format=1)] \
            +[__get_case(0, 'AR24', 60, "nature_ARGB8888_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'AB24', 60, "nature_ABGR8888_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'RA24', 60, "nature_RGBA8888_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'BA24', 60, "nature_BGRA8888_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'XR30', 60, "nature_XRGB2101010_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'XB30', 60, "nature_XBGR2101010_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'RX30', 60, "nature_RGBX1010102_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)] \
            +[__get_case(0, 'BX30', 60, "nature_BGRX1010102_2560x1600_TILE8x8_SUPERTILE_X.raw", 2560, 1600, 0, 0, connFormat='AR30', decType="NONE", tileMode="TILE_8X8_SUPERTILE_X", tileStatus=None)]

