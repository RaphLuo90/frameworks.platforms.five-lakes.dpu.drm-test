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
    return 'plane%d.feature.data_extend.'%id + subname

def __get_property(mode, alpha_value):
    p = OrderedDict()
    p['dataExtendMode'] = mode
    p['alphaDataExtendEnable'] = 1
    p['alphaDataExtendValue'] = alpha_value
    return OrderedDict([('DATA_EXTEND', p)])

def __get_golden():
    return OrderedDict()

def __get_case(plane_id, mode, alpha_value):
    crtc = Crtc(4)
    plane = Plane(plane_id, crtc, 'AR30', 640, 480, 60, __get_property(mode, alpha_value), resource='00013_32bpp_formatARGB2101010_640x480_LINEAR.bmp')
    conn = Connector(2, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, '{}_alpha{}'.format(mode, alpha_value)))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    case_list = list()
    extend_mode_list = ['STD', 'MSB']
    alpha_value_list = [0, 4294967295]
    for mode in extend_mode_list:
        for alpha_value in alpha_value_list:
            case_list.append(__get_case(0, mode, alpha_value))
            case_list.append(__get_case(3, mode, alpha_value))

    return case_list