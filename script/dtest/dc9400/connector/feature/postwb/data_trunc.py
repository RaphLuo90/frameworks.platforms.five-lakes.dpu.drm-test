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
    return 'connector%d.feature.postwb.data_trunc.'%id + subname

def __get_property(mode):
    p = OrderedDict()
    p['wbDataTruncMode'] = mode
    return OrderedDict([('DATA_TRUNC', p)])

def __get_golden():
    return OrderedDict()

def __get_case(plane_id, crtc_id, conn_id, mode):
    crtc = Crtc(crtc_id)
    plane = Plane(plane_id, crtc, 'AR24', 1280, 720, 60, resource='ARGB8888_linear_1280x720_bear.bmp')
    conn = Connector(conn_id, crtc, 'AR30', 1280, 720, 60, __get_property(mode), golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(conn_id, mode))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    case_list = list()
    case_list.append(__get_case(0, 0, 0, 'ROUNDING'))
    case_list.append(__get_case(0, 0, 0, 'TRUNCATE'))
    return case_list
