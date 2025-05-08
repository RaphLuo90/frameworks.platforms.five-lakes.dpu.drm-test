from collections import OrderedDict
from dtest.helper import *
import re

def __get_name(id, subname):
    return 'plane{}.feature.{}'.format(id, subname)

def __get_property():
    return OrderedDict()

def __get_ori_property(rotation):
    p = OrderedDict()
    p["rotation"] = rotation
    return OrderedDict(p)

def __get_golden():
    return OrderedDict()

def __get_case(plane_id,crtc_id, format, width, height, refresh, resource, decType, tileMode, rotation, crtcWidth, crtcHeight):
    crtc = Crtc(crtc_id)
    plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property(), resource=resource, decType=decType,
                  tileMode=tileMode, crtcWidth = crtcWidth,  crtcHeight = crtcHeight, ori_properties = __get_ori_property(rotation))
    if crtcWidth <= 1280 and crtcHeight <=720:
        if format == "LUMA_8":
            conn = Connector(crtc.id, crtc, 'NV16', crtcWidth, crtcHeight, 60, golden=__get_golden())
        else:
            conn = Connector(crtc.id, crtc, 'AR30', crtcWidth, crtcHeight, 60, golden=__get_golden())
    else:
        conn = Connector(crtc.id, crtc, 'AR30', 2700, 2600, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'rotation{}.input{}x{}_{}_{}'.format(rotation, width, height, format, tileMode)))

def __get_custom_case(plane_id,crtc_id, format, width, height, refresh, resource, decType, tileMode, rotation, crtcWidth, crtcHeight, use_custom_format=1):
    crtc = Crtc(crtc_id)
    plane_ori = OrderedDict()
    plane_ori["customFormat"] = 1
    plane_ori["rotation"] = rotation
    plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property(), resource=resource, decType=decType,
        tileMode=tileMode, crtcWidth = crtcWidth,  crtcHeight = crtcHeight, ori_properties = plane_ori)
    if width <= 1280 and height <=720:
        if format == "LUMA_10":
            conn = Connector(crtc.id, crtc, 'P010_UV', crtcWidth, crtcHeight, 60, golden=__get_golden())
        else:
            conn = Connector(crtc.id, crtc, 'AR30', crtcWidth, crtcHeight, 60, golden=__get_golden())
    else:
        conn = Connector(crtc.id, crtc, 'AR30', 2700, 2600, 60, golden=__get_golden())
    if format == "RG24":
        return DTestUnit(plane, conn, __get_name(plane_id, 'rotation{}.input{}x{}_{}_{}'.format(rotation, width, height, "RG24_planar", tileMode)))
    else:
        return DTestUnit(plane, conn, __get_name(plane_id, 'rotation{}.input{}x{}_{}_{}'.format(rotation, width, height, format, tileMode)))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    rotation_support_90 = {
        'NV12_tile16x16_1088x1920.raw': ['NV12', 'TILE16X16', 1088, 1920],
        'formatP010_2560x1440_Tile16x16.raw': ['P010_UV', 'TILE16X16', 2560, 1440]
    }

    rotation_support_flip = {
         'NV12_tile16x16_1088x1920.raw': ['NV12', 'TILE16X16', 1088, 1920],
        'formatP010_2560x1440_Tile16x16.raw': ['P010_UV', 'TILE16X16', 2560, 1440]
    }

    rotation_support_180_and_flip = {
         'NV21_LINEAR_640X480.raw': ['NV21', 'LINEAR', 640, 480],
         'Crew_NV61_1280X720_Linear.vimg': ['NV61', 'LINEAR', 1280, 720],
         'Boston_YU12_640X480_Linear.vimg': ['YU12', 'LINEAR', 640, 480],
         'P210_UV_1920x1080_LINEAR.raw': ['P210_UV', 'LINEAR', 1920, 1080],
         '00021_32bpp_formatARGB8888_640x480_LINEAR.bmp': ['AR24', 'LINEAR', 640, 480],
         '00022_32bpp_formatABGR8888_640x480_LINEAR.bmp': ['AB24', 'LINEAR', 640, 480],
         '00013_32bpp_formatARGB2101010_640x480_LINEAR.bmp': ['AR30', 'LINEAR', 640, 480],
         '00014_32bpp_formatABGR2101010_640x480_LINEAR.bmp': ['AB30', 'LINEAR', 640, 480],
         'RGB888_linear_640x480.raw': ['RG24', 'LINEAR', 640, 480],
         'YUV_LUMA_8_LINEAR_640X480.raw': ['LUMA_8', 'LINEAR', 640, 480],
         'NV12_1280x720_TILE32x8_A.raw': ['NV12', 'TILE32X8_A', 1280, 720],
         'ARGB8888_1280x720_TILE16x4.raw': ['AR24', 'TILE16X4', 1280, 720],
         'ABGR8888_1280x720_TILE16x4.raw': ['AB24', 'TILE16X4', 1280, 720]
    }

    rotation_support_180_and_flip_custom = {
          'P010_VU_LINEAR_640X480.raw': ['P010_VU', 'LINEAR', 640, 480],
          'exige_I010_UV_1280x720.raw': ['I010_UV', 'LINEAR', 1280, 720],
          'lake_P210_VU_full_BT709_1920x1080.raw': ['P210_VU', 'LINEAR', 1920, 1080],
          'P030_UV_1920x1080_Linear.raw': ['P030_UV', 'LINEAR', 1920, 1080],
          'zero0_RGB888_Planar_640x480.raw': ['RG24', 'LINEAR', 640, 480],
          'YUV_LUMA_10_LINEAR_640X480.raw': ['LUMA_10', 'LINEAR', 640, 480]
    }

    rotation_support_all = {
        'nature_XRGB8888_2560x1600_TILE8x8_SUPERTILE_X.raw': ['XR24', 'TILE_8X8_SUPERTILE_X', 2560, 1600],
        'nature_XRGB2101010_2560x1600_TILE8x8_SUPERTILE_X.raw': ['XR30', 'TILE_8X8_SUPERTILE_X', 2560, 1600],
        'NV12_1280x720_TILE32x8_YUVSP8x8.raw': ['NV12', 'TILE_32X8_YUVSP8X8', 1280, 720],
        'P010_1280x720_TILE16x8_YUVSP8x8.raw': ['P010_UV', 'TILE_16X8_YUVSP8X8', 1280, 720],
        'nature_ARGB8888_2560x1600_TILE8x8_SUPERTILE_X.raw': ['AR24', 'TILE_8X8_SUPERTILE_X', 2560, 1600],
        'nature_ARGB2101010_2560x1600_TILE8x8_SUPERTILE_X.raw': ['AR30', 'TILE_8X8_SUPERTILE_X', 2560, 1600]
    }

    case_list = []
    for key in rotation_support_90.keys():
        format, res_type, width, height = rotation_support_90.get(key)
        case_list.append(__get_case(0, 0, format, width, height, 60, key, "NONE", res_type, "90", height, width))

    for key in rotation_support_flip.keys():
        format, res_type, width, height = rotation_support_flip.get(key)
        for rotation in ["FLIPX", "FLIPY"]:
            case_list.append(__get_case(0, 0, format, width, height, 60, key, "NONE", res_type, rotation, width, height))

    for key in rotation_support_180_and_flip.keys():
        format, res_type, width, height = rotation_support_180_and_flip.get(key)
        for rotation in ["FLIPX", "FLIPY", "180"]:
            case_list.append(__get_case(0, 0, format, width, height, 60, key, "NONE", res_type, rotation, width, height))

    for key in rotation_support_180_and_flip_custom.keys():
        format, res_type, width, height = rotation_support_180_and_flip_custom.get(key)
        for rotation in ["FLIPX", "FLIPY", "180"]:
            case_list.append(__get_custom_case(0, 0, format, width, height, 60, key, "NONE", res_type, rotation, width, height, 1))

    for key in rotation_support_all.keys():
        format, res_type, width, height = rotation_support_all.get(key)
        for rotation in ["FLIPX", "FLIPY", "180", "90", "270"]:
            if rotation == "90" or rotation == "270":
                case_list.append(__get_case(0, 0, format, width, height, 60, key, "NONE", res_type, rotation, height, width))
            else:
                case_list.append(__get_case(0, 0, format, width, height, 60, key, "NONE", res_type, rotation, width, height))

    case_list.append(__get_case(0, 0, 'NV12', 1088, 1920, 60, 'NV12_tile16x16_1088x1920.raw', "NONE", 'TILE16X16', '180', 1088, 1920))
    case_list.append(__get_case(0, 0, 'P010_UV', 2560, 1440, 60, 'formatP010_2560x1440_Tile16x16.raw', "NONE", 'TILE16X16', '180', 2560, 1440))
    case_list.append(__get_case(0, 0, 'NV12', 1088, 1920, 60, 'NV12_tile16x16_1088x1920.raw', "NONE", 'TILE16X16', '270', 1920, 1088))
    case_list.append(__get_case(0, 0, 'P010_UV', 2560, 1440, 60, 'formatP010_2560x1440_Tile16x16.raw', "NONE", 'TILE16X16', '270', 1440, 2560))

    return case_list
