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

def __get_name(id, subname, InkMode:str, mstGain = 511, ca0Enable = 1, ca1Enable = 1, ca2Enable = 1):
    if InkMode == 'DEFAULT' or InkMode == 'ADAPT':
        return 'crtc%d.feature.postpq.'%id + subname + InkMode +'.mstGain'+ str(mstGain) + '.ca0Enable' + str(ca0Enable) + '.ca1Enable' + str(ca1Enable) + '.ca2Enable' + str(ca2Enable)
    return 'crtc%d.feature.postpq.'%id + subname + InkMode

def __get_case(id, InkMode:str, mstGain = 511, ca0Enable = 1, ca1Enable = 1, ca2Enable = 1):
    def __get_property():
        p_sharp = OrderedDict()
        p_sharp["sharpEnable"] = 1
        p_sharp["inkMode"] = InkMode

        p_sharpCSC = OrderedDict()
        p_sharpCSC["r2y_coef"] = [ 1742, 5859, 591, -818, -2753, 3572, 5038, -4576, -462]
        p_sharpCSC["y2r_coef"] = [ 8192, 0, 10488, 8192, -1760, -3118, 8192, 17432, 0]
        p_sharpCSC["r2y_offset"] = [0, 2048, 2048]
        p_sharpCSC["y2r_offset"] = [0, 2048, 2048]

        p_sharpLumaGain = OrderedDict()
        p_sharpLumaGain["Lut"] = [ 256, 128, 256, 511, 256, 128, 256, 511, 256 ]

        p_sharpLPF = OrderedDict()
        p_sharpLPF["lpf0_coef"] = [ 0, 0, 1, 2 ]
        p_sharpLPF["lpf1_coef"] = [ 0, 1, 2, 3 ]
        p_sharpLPF["lpf2_coef"] = [ 1, 2, 3, 4 ]

        p_sharpLPF_NOISE = OrderedDict()
        p_sharpLPF_NOISE["lut0"] = [ 256, 240, 223, 207, 192, 178, 165, 151, 139]
        p_sharpLPF_NOISE["lut1"] = [ 256, 230, 206, 184, 163, 145, 134, 130, 130]
        p_sharpLPF_NOISE["lut2"] = [ 256, 230, 203, 181, 159, 141, 130, 126, 130]
        p_sharpLPF_NOISE["luma_strength0"] = 45
        p_sharpLPF_NOISE["luma_strength1"] = 58
        p_sharpLPF_NOISE["luma_strength2"] = 64

        p_sharpLPF_CURVE = OrderedDict()
        p_sharpLPF_CURVE["lut0"] = [ 42, 82, 126, 168, 210, 256, 64, 116, 92, 82, 77, 64, 64 ]
        p_sharpLPF_CURVE["lut1"] = [ 42, 82, 126, 168, 210, 256, 64, 116, 92, 82, 77, 64, 64 ]
        p_sharpLPF_CURVE["lut2"] = [ 42, 82, 126, 168, 210, 256, 64, 116, 92, 82, 77, 64, 64 ]
        p_sharpLPF_CURVE["master_gain"] = mstGain

        p_sharpCOLOR_ADAPTIVE = OrderedDict()
        p_sharpCOLOR_ADAPTIVE["ca0_enable"] = ca0Enable
        p_sharpCOLOR_ADAPTIVE["ca0_param"] = [ 256, 1800, 300, 512, 300, 200, 512 ]
        p_sharpCOLOR_ADAPTIVE["ca1_enable"] = ca1Enable
        p_sharpCOLOR_ADAPTIVE["ca1_param"] = [ 256, 1080, 200, 512, 300, 100, 512 ]
        p_sharpCOLOR_ADAPTIVE["ca2_enable"] = ca2Enable
        p_sharpCOLOR_ADAPTIVE["ca2_param"] = [ 256, 2520, 200, 512, 300, 100, 512 ]

        p_sharpCOLOR_BOOST = OrderedDict()
        p_sharpCOLOR_BOOST["pos_gain"] = 4
        p_sharpCOLOR_BOOST["neg_gain"] = 4
        p_sharpCOLOR_BOOST["y_offset"] = 4

        p_sharpSOFT_CLIP = OrderedDict()
        p_sharpSOFT_CLIP["pos_offset"] = 12
        p_sharpSOFT_CLIP["neg_offset"] = 12
        p_sharpSOFT_CLIP["pos_wet"] = 4
        p_sharpSOFT_CLIP["neg_wet"] = 12

        p_sharp_DITHER = OrderedDict()
        p_sharp_DITHER["dth_r"] = [ 0, 2, 1, 3 ]
        p_sharp_DITHER["dth_g"] = [ 1, 0, 3, 2 ]
        p_sharp_DITHER["dth_b"] = [ 3, 2, 1, 0 ]


        return OrderedDict([('SHARPNESS', p_sharp), ('SHARPNESS_CSC', p_sharpCSC), ('SHARPNESS_LUMA_GAIN',p_sharpLumaGain), \
                            ('SHARPNESS_LPF', p_sharpLPF),('SHARPNESS_LPF_NOISE',p_sharpLPF_NOISE), ('SHARPNESS_LPF_CURVE',p_sharpLPF_CURVE),\
                            ('SHARPNESS_COLOR_ADAPTIVE',p_sharpCOLOR_ADAPTIVE), ('SHARPNESS_COLOR_BOOST',p_sharpCOLOR_BOOST), \
                            ('SHARPNESS_SOFT_CLIP',p_sharpSOFT_CLIP), ('SHARPNESS_DITHER',p_sharp_DITHER)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc = Crtc(id, __get_property())
    plane = Plane(0, crtc, 'AR24', 1280, 720, 60,resource = 'ARGB8888_linear_1280x720_bear.bmp')
    conn = Connector(0, crtc, 'AR30', 1280, 720, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(id, 'sharpness', InkMode, mstGain, ca0Enable, ca1Enable, ca2Enable))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    case_list = [__get_case(0, 'DEFAULT', 128, 1, 1, 1), \
                 __get_case(0, 'DEFAULT', 256, 1, 1, 1), \
                 __get_case(0, 'DEFAULT', 511, 1, 1, 1), \
                 __get_case(0, 'ADAPT', 511, 1, 1, 0), \
                 __get_case(0, 'ADAPT', 511, 1, 0, 1), \
                 __get_case(0, 'ADAPT', 511, 1, 0, 0), \
                 __get_case(0, 'ADAPT', 511, 0, 1, 1), \
                 __get_case(0, 'ADAPT', 511, 0, 1, 0), \
                 __get_case(0, 'ADAPT', 511, 0, 0, 1) \
                 ]
    inkmodes = ['G0', 'G1', 'G2', 'L0', 'L1', 'L2', 'V0', 'V1', 'V2', 'ADAPT', 'LUMA']
    for inkmode in inkmodes:
        case_list.append(__get_case(0, inkmode))
    #return [ __get_case(0, )]
    return case_list
