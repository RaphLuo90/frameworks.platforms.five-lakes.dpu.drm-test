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
    return 'crtc%d.feature.postpq.data_extend.'%id + subname

def __get_property(mode, crtc_id):
    p1 = OrderedDict()
    p1['dataExtendMode'] = mode
    if crtc_id in [2, 3, 4]:
        p2 = OrderedDict()
        p2['mode'] = 'USR'
        p2['gamut'] = 'BT2020'
        p2['busFormat'] = 'YUV422_10BIT'
        p2['coef'] = [1076, 2777, 243, -572, -1476, 2048, 2048, -1883, -165, 0, 0, 0, 0, 2048, 2048]
        return OrderedDict([('DATA_EXTEND', p1), ('R2Y', p2)])
    return OrderedDict([('DATA_EXTEND', p1)])

def __get_golden():
    return OrderedDict()

def __get_case(plane_id, crtc_id, conn_id, mode):
    if crtc_id in [2, 3, 4]:
        conn_format = 'YUV444_10BIT'
    else:
        conn_format = 'AR30'
    crtc = Crtc(crtc_id, __get_property(mode, crtc_id))
    plane = Plane(plane_id, crtc, 'AR30', 640, 480, 60, resource='00013_32bpp_formatARGB2101010_640x480_LINEAR.bmp')
    conn = Connector(conn_id, crtc, conn_format, 640, 480, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(crtc_id, mode))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    case_list = list()
    extend_mode_list = ['STD', 'MSB']
    crtc_list = [0, 1, 2, 3, 4]
    for crtc_id in crtc_list:
        for mode in extend_mode_list:
            if crtc_id == 4:
                conn_id = 2
            else:
                conn_id = 0
            case_list.append(__get_case(0, crtc_id, conn_id, mode))
    return case_list
