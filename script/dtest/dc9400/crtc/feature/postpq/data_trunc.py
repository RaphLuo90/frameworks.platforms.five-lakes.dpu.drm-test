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
    return 'crtc%d.feature.postpq.data_trunc.'%id + subname

def __get_property_blend_plane():
    p = OrderedDict()
    p['dataExtendMode'] = 'MSB'
    p['alphaDataExtendValue'] = 4294967295
    return OrderedDict([('DATA_EXTEND', p)])

def __get_property_blend_crtc():
    p = OrderedDict()
    p['gammaDataTruncMode'] = 'TRUNCATE'
    p['panelDataTruncMode'] = 'TRUNCATE'
    p['blendDataTruncMode'] = 'ROUNDING'
    return OrderedDict([('DATA_TRUNC', p)])

def __get_property_gamma_crtc():
    p1 = OrderedDict()
    p2 = OrderedDict()
    p1['gammaDataTruncMode'] = 'ROUNDING'
    p1['panelDataTruncMode'] = 'TRUNCATE'
    p1['blendDataTruncMode'] = 'TRUNCATE'
    p2['seg_point'] = [32, 48, 816, 832, 936, 944, 960, 968]
    p2['seg_step'] = [32, 16, 8, 4, 8, 4, 8, 4, 8]
    p2['entry'] = [
            0,
            40,
            60,
            72,
            84,
            99,
            114,
            131,
            149,
            168,
            189,
            211,
            235,
            260,
            287,
            315,
            345,
            377,
            410,
            444,
            481,
            519,
            558,
            600,
            643,
            688,
            735,
            783,
            834,
            886,
            940,
            996,
            1053,
            1113,
            1175,
            1238,
            1304,
            1371,
            1441,
            1512,
            1586,
            1661,
            1739,
            1819,
            1901,
            1985,
            2071,
            2159,
            2249,
            2342,
            2436,
            2533,
            2632,
            2734,
            2837,
            2943,
            3051,
            3162,
            3274,
            3389,
            3507,
            3626,
            3748,
            3873,
            3999,
            4128,
            4260,
            4394,
            4530,
            4669,
            4810,
            4954,
            5100,
            5249,
            5400,
            5553,
            5710,
            5868,
            6030,
            6193,
            6360,
            6529,
            6700,
            6874,
            7051,
            7231,
            7413,
            7597,
            7784,
            7974,
            8167,
            8362,
            8560,
            8761,
            8965,
            9171,
            9380,
            9592,
            9806,
            9914,
            10023,
            10133,
            10243,
            10466,
            10692,
            10920,
            11151,
            11385,
            11622,
            11862,
            12104,
            12350,
            12598,
            12850,
            13104,
            13361,
            13490,
            13621,
            13884,
            14149,
            14284,
            14418,
            14690,
            14965,
            15242,
            15523,
            15807,
            16093,
            16383
          ]
    return OrderedDict([('DATA_TRUNC', p1), ('DEGAMMA', p2)])

def __get_golden():
    return OrderedDict()

def __get_case(plane_id, crtc_id, conn_id, mode):
    if mode == 'blend':
        crtc = Crtc(crtc_id, __get_property_blend_crtc())
        plane = Plane(plane_id, crtc, 'AR24', 640, 480, 60, __get_property_blend_plane(), resource='zero2_ARGB8.bmp')
        conn = Connector(conn_id, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    elif mode == 'gamma':
        crtc = Crtc(crtc_id, __get_property_gamma_crtc())
        plane = Plane(plane_id, crtc, 'AR24', 1280, 720, 60, resource='ARGB8888_linear_1280x720_bear.bmp')
        conn = Connector(conn_id, crtc, 'AR30', 1280, 720, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(crtc_id, mode))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    case_list = list()
    trunc_mode_list = ['blend', 'gamma']
    crtc_list = [0, 1, 2, 3, 4]
    for crtc_id in crtc_list:
        for mode in trunc_mode_list:
            if crtc_id in [2, 3, 4] and mode == 'gamma':
                continue
            if crtc_id == 4:
                conn_id = 2
            else:
                conn_id = 0
            case_list.append(__get_case(0, crtc_id, conn_id, mode))
    return case_list
