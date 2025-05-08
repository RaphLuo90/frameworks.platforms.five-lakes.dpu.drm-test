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

def __get_name(id, subname, fc):
    if fc == 1:
        return 'plane%d.feature.decompress.'%id + subname + '.fastclear_vulkan'
    elif fc == 2:
        return 'plane%d.feature.decompress.'%id + subname + '.fastclear_normal'
    else:
        return 'plane%d.feature.decompress.'%id + subname
def __get_property(fc):
    if fc == 1:
        p = OrderedDict()
        p["fcEnable"] = 1
        p["fcSize"] = 128
        return OrderedDict([('DEC_DECODER_FC', p)])
    elif fc == 2:
        p = OrderedDict()
        p["fcEnable"] = 0
        p["fcSize"] = 128
        p["fcRGBY"] = "0xFFFF0000"
        p["fcUV"] = "0xFFFF0000"
        return OrderedDict([('DEC_DECODER_FC', p)])
    else:
        return OrderedDict()

def __get_ori_property(fc, mode, gamut):
    if fc == 1:
        p = OrderedDict()
        p["fcEnable"] = 1
        p["fcSize"] = 128
        p2 = OrderedDict()
        p2["mode"] = mode
        p2["gamut"] = gamut
        return OrderedDict([('DEC_DECODER_FC', p), ('Y2R_CONFIG', p2)])
    elif fc == 2:
        p = OrderedDict()
        p["fcEnable"] = 0
        p["fcSize"] = 128
        p["fcRGBY"] = "0xFFFF0000"
        p["fcUV"] = "0xFFFF0000"
        p2 = OrderedDict()
        p2["mode"] = mode
        p2["gamut"] = gamut
        return OrderedDict([('DEC_DECODER_FC', p), ('Y2R_CONFIG', p2)])
    else:
        p2 = OrderedDict()
        p2["mode"] = mode
        p2["gamut"] = gamut
        return OrderedDict(p2)

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

format_list = ["NV12", "NV21","NV16", "NV61", "P010_UV", "P010_VU", "P210_UV", "P210_VU"]

def __get_case(format : str, refresh : int, resource="defaultPattern", width=640, height=480, connFormat='AR30', decType="NONE",tileMode = "LINEAR", align64=0, tileStatus=None, fc = 0):
    crtc = Crtc(0)

    plane_ori = OrderedDict()
    plane_ori['align64'] = align64
    if format in custom_format:
        plane_ori['customFormat'] = 1
    if format in format_list:
        plane_ori['yuvColorEncoding'] = 'BT601'
        plane_ori['yuvColorRange'] = 'FULL'
        plane = Plane(0, crtc, format, width, height, refresh, __get_property(fc), resource,  decType= decType, tileMode=tileMode, tileStatus=tileStatus, ori_properties=plane_ori)
    else:
        plane = Plane(0, crtc, format, width, height, refresh, __get_property(fc), resource,  decType= decType, tileMode=tileMode, tileStatus=tileStatus, ori_properties=plane_ori)
    if width >= 2560 and height >= 1600:
       con_width = 3840
       con_height = 2160
    else:
       con_width = width
       con_height = height
    conn = Connector(0, crtc, connFormat, con_width, con_height, refresh, golden=__get_golden())
    if align64:
        return DTestUnit(plane, conn, __get_name(0, "{}_{}_{}x{}_ALIGN_64".format(format, tileMode, width, height), fc))
    else:
        return DTestUnit(plane, conn, __get_name(0, "{}_{}_{}x{}_ALIGN_32".format(format, tileMode, width, height), fc))

custom_format = ['P010_VU','P210_VU']

@dtest_unit_group('dc9400')
def __gen(chip_info):
    case_group = []

    return  case_group +[__get_case('NV12', 60, "nature_NV12_1280x720_dec400_32x8_YUVSP8x8_32align_compress.raw", 1280, 720, 'AR30', "DEC", "TILE_32X8_YUVSP8X8", 0, [("tileStatus","nature_NV12_1280x720_dec400_32x8_YUVSP8x8_32align.Y.ts"), ("tileStatus1","nature_NV12_1280x720_dec400_32x8_YUVSP8x8_32align.UV.ts")])] \
            +[__get_case('NV21', 60, "Outdoor.NV21_TILE_32X8_YUVSP8X8_1280x720_compressed.raw", 1280, 720, 'AR30', "DEC", "TILE_32X8_YUVSP8X8", 0, [("tileStatus","Outdoor.NV21_TILE_32X8_YUVSP8X8_1280x720_compressed.raw.y.ts"), ("tileStatus1","Outdoor.NV21_TILE_32X8_YUVSP8X8_1280x720_compressed.raw.uv.ts")])] \
            +[__get_case('P010_VU', 60, "P010_VU_TILE_16X8_YUVSP8X8_1280x720_ALIGN_32_compressed.raw", 1280, 720, 'AR30', "DEC", "TILE_16X8_YUVSP8X8", 0, [("tileStatus","P010_VU_TILE_16X8_YUVSP8X8_1280x720_ALIGN_32_compressed.raw.y.ts"), ("tileStatus1","P010_VU_TILE_16X8_YUVSP8X8_1280x720_ALIGN_32_compressed.raw.uv.ts")])] \
            +[__get_case('P010_UV', 60, "nature_P010_UV_1280x720_dec400_16x8_YUVSP8x8_32align_compress.raw", 1280, 720, 'AR30', "DEC", "TILE_16X8_YUVSP8X8", 0, [("tileStatus","nature_P010_UV_1280x720_dec400_16x8_YUVSP8x8_32align.Y.ts"), ("tileStatus1","nature_P010_UV_1280x720_dec400_16x8_YUVSP8x8_32align.UV.ts")])] \
            +[__get_case('XR24', 60, "nature_XRGB8_2560x1600_dec400_8x8_SUPERTILE_X_32align_compress.raw", 2560, 1600, 'AR30', "DEC", "TILE_8X8_SUPERTILE_X", 0, [("tileStatus","nature_XRGB8_2560x1600_dec400_8x8_SUPERTILE_X_32align.ts")])] \
            +[__get_case('XR30', 60, "nature_XRGB2101010_2560x1600_dec400_8x8_SUPERTILE_X_32align_compress.raw", 2560, 1600, 'AR30', "DEC", "TILE_8X8_SUPERTILE_X", 0, [("tileStatus","nature_XRGB2101010_2560x1600_dec400_8x8_SUPERTILE_X_32align.ts")])] \
            +[__get_case('AR30', 60, "nature_ARGB2101010_2560x1600_dec400_8x8_SUPERTILE_X_32align_compress.raw", 2560, 1600, 'AR30', "DEC", "TILE_8X8_SUPERTILE_X", 0, [("tileStatus","nature_ARGB2101010_2560x1600_dec400_8x8_SUPERTILE_X_32align.ts")])] \
            +[__get_case('AR24', 60, "nature_ARGB8_2560x1600_dec400_8x8_SUPERTILE_X_32align_compress.raw", 2560, 1600, 'AR30', "DEC", "TILE_8X8_SUPERTILE_X", 0, [("tileStatus","nature_ARGB8_2560x1600_dec400_8x8_SUPERTILE_X_32align.ts")], 0)] \
            +[__get_case('AR24', 60, "Outdoor_ARGB8_1920x1080_dec400_64x1r_32align_compress.raw", 1920, 1080, 'AR30', "DEC", "RASTER_64X1", 0, [("tileStatus","Outdoor_ARGB8_1920x1080_dec400_64x1r_32align.ts")])] \
            +[__get_case('AR30', 60, "road_ARGB2101010_1920x1080_dec400_64x1r_32align_compress.raw", 1920, 1080, 'AR30', "DEC", "RASTER_64X1", 0, [("tileStatus","road_ARGB2101010_1920x1080_dec400_64x1r_32align.ts")])] \
            +[__get_case('NV12', 60, "pear_blossom1_NV12_1280x720_dec400_256x1r_32align_compress.raw", 1280, 720, 'AR30', "DEC", "RASTER_256X1", 0, [("tileStatus","pear_blossom1_NV12_1280x720_dec400_256x1r_32align.Y.ts"), ("tileStatus1","pear_blossom1_NV12_1280x720_dec400_256x1r_32align.UV.ts")])] \
            +[__get_case('NV21', 60, "Outdoor.NV21_RASTER_256X1_1280x720_compressed.raw", 1280, 720, 'AR30', "DEC", "RASTER_256X1", 0, [("tileStatus","Outdoor.NV21_RASTER_256X1_1280x720_compressed.raw.y.ts"), ("tileStatus1","Outdoor.NV21_RASTER_256X1_1280x720_compressed.raw.uv.ts")])] \
            +[__get_case('P010_UV', 60, "road_P010_UV_1920x1080_dec400_128x1r_32align_compress.raw", 1920, 1080, 'AR30', "DEC", "RASTER_128X1", 0, [("tileStatus","road_P010_UV_1920x1080_dec400_128x1r_32align.Y.ts"), ("tileStatus1","road_P010_UV_1920x1080_dec400_128x1r_32align.UV.ts")])] \
            +[__get_case('P210_UV', 60, "road_P210_UV_1920x1080_dec400_128x1r_32align_compress.raw", 1920, 1080, 'AR30', "DEC", "RASTER_128X1", 0, [("tileStatus","road_P210_UV_1920x1080_dec400_128x1r_32align.Y.ts"), ("tileStatus1","road_P210_UV_1920x1080_dec400_128x1r_32align.UV.ts")])] \
            +[__get_case('P010_VU', 60, "P010_VU_RASTER_128X1_1280x720_ALIGN_32_compressed.raw", 1280, 720, 'AR30', "DEC", "RASTER_128X1", 0, [("tileStatus","P010_VU_RASTER_128X1_1280x720_ALIGN_32_compressed.raw.y.ts"), ("tileStatus1","P010_VU_RASTER_128X1_1280x720_ALIGN_32_compressed.raw.uv.ts")])] \
            +[__get_case('P210_VU', 60, "P210_VU_RASTER_128X1_1280x720_ALIGN_32_compressed.raw", 1280, 720, 'AR30', "DEC", "RASTER_128X1", 0, [("tileStatus","P210_VU_RASTER_128X1_1280x720_ALIGN_32_compressed.raw.y.ts"), ("tileStatus1","P210_VU_RASTER_128X1_1280x720_ALIGN_32_compressed.raw.uv.ts")])] \
            +[__get_case('NV16', 60, "pear_blossom1_NV16_1280x720_dec400_256x1r_32align_compress.raw", 1280, 720, 'AR30', "DEC", "RASTER_256X1", 0, [("tileStatus","pear_blossom1_NV16_1280x720_dec400_256x1r_32align.Y.ts"), ("tileStatus1","pear_blossom1_NV16_1280x720_dec400_256x1r_32align.UV.ts")])] \
            +[__get_case('NV61', 60, "Outdoor.NV61_RASTER_256X1_1280x720_compressed.raw", 1280, 720, 'AR30', "DEC", "RASTER_256X1", 0, [("tileStatus","Outdoor.NV61_RASTER_256X1_1280x720_compressed.raw.y.ts"), ("tileStatus1","Outdoor.NV61_RASTER_256X1_1280x720_compressed.raw.uv.ts")])] \            +[__get_case('AR24', 60, "connector0.feature.compress.AR24_TILE_8X8_SUPERTILE_X_DEC_fc.raw", 640, 480, 'AR30', "DEC", "TILE_8X8_SUPERTILE_X", 0, [("tileStatus","connector0.feature.compress.AR24_TILE_8X8_SUPERTILE_X_DEC_fc_vulkan.raw.ts")],1)] \
            +[__get_case('AR24', 60, "connector0.feature.compress.AR24_TILE_8X8_SUPERTILE_X_DEC_fc.raw", 640, 480, 'AR30', "DEC", "TILE_8X8_SUPERTILE_X", 0, [("tileStatus","connector0.feature.compress.AR24_TILE_8X8_SUPERTILE_X_DEC_fc_normal.raw.ts")],2)]
