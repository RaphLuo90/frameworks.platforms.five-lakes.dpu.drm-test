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
    return 'connector%d.feature.postwb.data_extend.'%id + subname

def __get_property(mode, crtc_id):
    p1 = OrderedDict()
    p1['dataExtendMode'] = mode
    p2 = OrderedDict()
    p2['mode'] = 'USR'
    p2['gamut'] = 'BT2020'
    p2['coef'] = [1076, 2777, 243, -572, -1476, 2048, 2048, -1883, -165, 0, 0, 0, 0, 2048, 2048]
    return OrderedDict([('DATA_EXTEND', p1), ('R2Y', p2)])

def __get_golden():
    return OrderedDict()

def __get_case(plane_id, crtc_id, conn_id, mode):
    crtc = Crtc(crtc_id)
    plane = Plane(plane_id, crtc, 'AR30', 640, 480, 60, resource='00013_32bpp_formatARGB2101010_640x480_LINEAR.bmp')
    conn = Connector(conn_id, crtc, 'NV12', 640, 480, 60, __get_property(mode, crtc_id), golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(conn_id, mode))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    #case_list = list()
    #case_list.append(__get_case(0, 0, 0, 'STD'))
    #case_list.append(__get_case(0, 1, 0, 'MSB'))
    #return case_list
    return [ __get_case(0, 0, 0, 'STD'), __get_case(0, 0, 0, 'MSB') ]
