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

def __get_name(id, con_id=0,sub=0):
    return 'crtc%d.feature.postpq.'%id + 'dbi%d'%sub+ '.connector%d'%con_id

def __get_property(width, height):
    dbi = OrderedDict()

    dbi["panel_width"] = width
    dbi["panel_height"] = height

    roi0_ctl = OrderedDict()
    roi0_ctl["x"] = 0
    roi0_ctl["y"] = 0
    roi0_ctl["w"] = 1080
    roi0_ctl["h"] = 2992

    udc0_ctl = OrderedDict()
    udc0_ctl["x"] = 750
    udc0_ctl["y"] = 2650

    udc1_ctl = OrderedDict()
    udc1_ctl["x"] = 880
    udc1_ctl["y"] = 2792

    dither = OrderedDict()
    dither["index_type"] = 1
    dither["sw_index"] = 2
    dither["frm_mode"] = 10
    dither["table_low"] = [2143066133, 4125546537, 2014534398]
    dither["table_high"] = [2748376162, 1283171088, 547081572]

    dbi["ROI0_DBI_CONTROL"] = roi0_ctl
    dbi["UDC0_DBI_CONTROL"] = udc0_ctl
    dbi["UDC1_DBI_CONTROL"] = udc1_ctl
    dbi["DITHER"] = dither
    dbi["MAIN_LUT"] = "pattern_in_norm_luts_main.ppm"
    dbi["UDC0_LUT"] = "pattern_in_norm_luts_udc0.ppm"
    dbi["UDC1_LUT"] = "pattern_in_norm_luts_udc1.ppm"

    coefs = OrderedDict()

    main_coef = OrderedDict()
    main_coef["slope_m_red"] = [28672, 24576, 24576, 8192, -12288, -24576, -9451, 4000]
    main_coef["slope_a_red"] = [24576, -8192, -7000, 0, 12000, 16384, 16000, 4500]
    main_coef["intercept_m_red"] = [2048, 1024, -20480, 0, 46080, 107648, 33792, -1024]
    main_coef["intercept_a_red"] = [10240, 5120, 8192, 0, -46080, -51200, -40960, 1024]
    main_coef["threshold_red"] = [512, 1024, 1536, 2048, 2560, 3072, 3584, 4095]
    main_coef["slope_m_green"] = [28672, 24576, 24576, 8192, -12288, -24576, -9451, 4000]
    main_coef["slope_a_green"] = [24576, -8192, -7000, 0, 12000, 16384, 16000, 4500]
    main_coef["intercept_m_green"] = [2048, 1024, -20480, 0, 46080, 107648, 33792, -1024]
    main_coef["intercept_a_green"] = [10240, 5120, 8192, 0, -46080, -51200, -40960, 1024]
    main_coef["threshold_green"] = [512, 1024, 1536, 2048, 2560, 3072, 3584, 4095]
    main_coef["slope_m_blue"] = [28672, 24576, 24576, 8192, -12288, -24576, -9451, 4000]
    main_coef["slope_a_blue"] = [24576, -8192, -7000, 0, 12000, 16384, 16000, 4500]
    main_coef["intercept_m_blue"] = [2048, 1024, -20480, 0, 46080, 107648, 33792, -1024]
    main_coef["intercept_a_blue"] = [10240, 5120, 8192, 0, -46080, -51200, -40960, 1024]
    main_coef["threshold_blue"] = [512, 1024, 1536, 2048, 2560, 3072, 3584, 4095]

    udc0_coef = OrderedDict()
    udc0_coef["slope_m_red"] = [8192, 0, 0, 0, 0, 0, 0, 0]
    udc0_coef["slope_a_red"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc0_coef["intercept_m_red"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc0_coef["intercept_a_red"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc0_coef["threshold_red"] = [4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095]
    udc0_coef["slope_m_green"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc0_coef["slope_a_green"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc0_coef["intercept_m_green"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc0_coef["intercept_a_green"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc0_coef["threshold_green"] = [4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095]
    udc0_coef["slope_m_blue"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc0_coef["slope_a_blue"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc0_coef["intercept_m_blue"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc0_coef["intercept_a_blue"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc0_coef["threshold_blue"] = [4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095]

    udc1_coef = OrderedDict()
    udc1_coef["slope_m_red"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc1_coef["slope_a_red"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc1_coef["intercept_m_red"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc1_coef["intercept_a_red"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc1_coef["threshold_red"] = [4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095]
    udc1_coef["slope_m_green"] = [8192, 0, 0, 0, 0, 0, 0, 0]
    udc1_coef["slope_a_green"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc1_coef["intercept_m_green"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc1_coef["intercept_a_green"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc1_coef["threshold_green"] = [4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095]
    udc1_coef["slope_m_blue"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc1_coef["slope_a_blue"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc1_coef["intercept_m_blue"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc1_coef["intercept_a_blue"] = [0, 0, 0, 0, 0, 0, 0, 0]
    udc1_coef["threshold_blue"] = [4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095]


    coefs["MAIN"] = main_coef
    coefs["UDC0"] = udc0_coef
    coefs["UDC1"] = udc1_coef

    return OrderedDict([("DBI", dbi), ("DBI_CORRECTION_COEF", coefs)])

def __get_property1(width, height):
    dbi = OrderedDict()

    dbi["panel_width"] = width
    dbi["panel_height"] = height

    roi1_ctl = OrderedDict()
    roi1_ctl["x"] = 2
    roi1_ctl["y"] = 300
    roi1_ctl["w"] = 770
    roi1_ctl["h"] = 2200

    roi2_ctl = OrderedDict()
    roi2_ctl["x"] = 0
    roi2_ctl["y"] = 2601
    roi2_ctl["w"] = 1078
    roi2_ctl["h"] = 389

    udc1_ctl = OrderedDict()
    udc1_ctl["x"] = 878
    udc1_ctl["y"] = 1399

    dither = OrderedDict()
    dither["index_type"] = 1
    dither["sw_index"] = 0
    dither["frm_mode"] = 6
    dither["table_low"] = [2143066133, 4125546537, 2014534398]
    dither["table_high"] = [2748376162, 1283171088, 547081572]

    dbi["ROI1_DBI_CONTROL"] = roi1_ctl
    dbi["ROI2_DBI_CONTROL"] = roi2_ctl
    dbi["UDC1_DBI_CONTROL"] = udc1_ctl
    dbi["DITHER"] = dither
    dbi["MAIN_LUT"] = "pattern1_in_norm_luts_main.ppm"
    dbi["UDC1_LUT"] = "pattern1_in_norm_luts_udc1.ppm"

    coefs = OrderedDict()

    main_coef = OrderedDict()
    main_coef["slope_m_red"] = [28672, 24576, 24576, 8192, -12288, -24576, -9451, 4000]
    main_coef["slope_a_red"] = [24576, -8192, -7000, 0, 12000, 16384, 16000, 4500]
    main_coef["intercept_m_red"] = [2048, 1024, -20480, 0, 46080, 107648, 33792, -1024]
    main_coef["intercept_a_red"] = [10240, 5120, 8192, 0, -46080, -51200, -40960, 1024]
    main_coef["threshold_red"] = [512, 1024, 1536, 2048, 2560, 3072, 3584, 4095]
    main_coef["slope_m_green"] = [-12288, -24576, -9451, 4000, 28672, 24576, 24576, 8192]
    main_coef["slope_a_green"] = [12000, 16384, 16000, 4500, 24576, -8192, -7000, 0]
    main_coef["intercept_m_green"] = [46080, 107648, 33792, -1024, 2048, 1024, -20480, 0]
    main_coef["intercept_a_green"] = [-46080, -51200, -40960, 1024, 10240, 5120, 8192, 0]
    main_coef["threshold_green"] = [1536, 2048, 2560, 3072, 4095, 4095, 4095, 4095]
    main_coef["slope_m_blue"] = [28672, 24576, -12288, -24576, -9451, 4000, 24576, 8192]
    main_coef["slope_a_blue"] = [24576, -8192, 12000, 16384, 16000, 4500, -7000, 0]
    main_coef["intercept_m_blue"] = [2048, 1024, 46080, 107648, 33792, -1024, -20480, 0]
    main_coef["intercept_a_blue"] = [10240, 5120, -46080, -51200, -40960, 1024, -8192, 0]
    main_coef["threshold_blue"] = [1024, 1600, 2500, 3120, 3555, 3720, 4000, 4095]

    udc1_coef = OrderedDict()
    udc1_coef["slope_m_red"] = [0, 0, 0, 0, -32199, 32000, 0, 0]
    udc1_coef["slope_a_red"] = [0, 0, 0, 0, 29000, -16000, 0, 0]
    udc1_coef["intercept_m_red"] = [0, 0, 0, 0, 107648, 33792, 0, 0]
    udc1_coef["intercept_a_red"] = [0, 0, 0, 0, -46080, -51200, 0, 0]
    udc1_coef["threshold_red"] = [200, 400, 500, 800, 1800, 2048, 3072, 4095]
    udc1_coef["slope_m_green"] = [-16384, 24576, 0, 0, 0, 0, 0, 0]
    udc1_coef["slope_a_green"] = [14000, -8000, 0, 0, 0, 0, 0, 0]
    udc1_coef["intercept_m_green"] = [1024, -20480, 0, 0, 0, 0, 0, 0]
    udc1_coef["intercept_a_green"] = [10240, 5120, 0, 0, 0, 0, 0, 0]
    udc1_coef["threshold_green"] = [1800, 2048, 2080, 2081, 2441, 2990, 3072, 4095]
    udc1_coef["slope_m_blue"] = [0, 0, 0, 0, 0, 32000, -5990, 0]
    udc1_coef["slope_a_blue"] = [0, 0, 0, 0, 0, -12800, 2039, 0]
    udc1_coef["intercept_m_blue"] = [0, 0, 0, 0, 0, 33792, -160240, 0]
    udc1_coef["intercept_a_blue"] = [0, 0, 0, 0, 0, -40960, 182400, 0]
    udc1_coef["threshold_blue"] = [200, 400, 600, 800, 1024, 1800, 2048, 4095]


    coefs["MAIN"] = main_coef
    coefs["UDC1"] = udc1_coef

    return OrderedDict([("DBI", dbi), ("DBI_CORRECTION_COEF", coefs)])

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

def __get_case(panel_width, panel_height):
    crtc = Crtc(0, __get_property(panel_width, panel_height))
    plane = Plane(0, crtc, 'AR30', panel_width, panel_height, 60, resource='pattern_ar30_linear_1080x2992.raw')
    conn = Connector(0, crtc, 'AR30', panel_width, panel_height, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(0, 0, 0))

def __get_case1(panel_width, panel_height):
    crtc = Crtc(0, __get_property1(panel_width, panel_height))
    plane = Plane(0, crtc, 'AR30', panel_width, panel_height, 60, resource='pattern1_ar30_linear_1078x2990.raw')
    conn = Connector(0, crtc, 'AR30', panel_width, panel_height, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(0, 0, 1))


@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(1080, 2992), __get_case1(1078, 2990)]
