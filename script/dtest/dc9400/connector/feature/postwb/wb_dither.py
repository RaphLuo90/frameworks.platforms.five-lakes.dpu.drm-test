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
import re

def __get_name(id, subname):
    return 'connector%d.feature.postwb.'%id + subname

def __get_case(resource, crtc_id, conn_id, format, width, height):
    def __get_property(index_type, sw_index=2):
        p = OrderedDict()
        p["index_type"] = index_type
        if index_type in [1]:  ##only sw (1) need
            p["sw_index"] = sw_index

        p["table_low"] = [1850516096, 1850516096, 1850516096]
        p["table_high"] = [1568641459, 1568641459, 1568641459]
        return OrderedDict([('WB_DITHER', p)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    case_groups = []
    index_types = {'none': 0,
                   'sw': 1,
                   'hw': 2
                   }
    sw_index_list = [0, 1, 2, 3]
    for name, val in index_types.items():
        if name == 'sw':
            for sw_index in sw_index_list:
                prop = __get_property(val, sw_index)
                crtc = Crtc(crtc_id)
                plane = Plane(0, crtc, 'AR24', width, height, 60, resource=resource)
                conn = Connector(conn_id, crtc, format, width, height, 60, prop, golden=__get_golden())
                case_groups.append(
                    DTestUnit(plane, conn, __get_name(conn_id, 'wb_dither_{}_{}'.format(name, sw_index))))
        else:
            prop = __get_property(val)
            crtc = Crtc(crtc_id)
            plane = Plane(0, crtc, 'AR24', width, height, 60, resource=resource)
            conn = Connector(conn_id, crtc, format, width, height, 60, prop, golden=__get_golden())
            case_groups.append(DTestUnit(plane, conn, __get_name(conn_id, 'wb_dither_{}'.format(name))))

    return case_groups

@dtest_unit_group('dc9400')
def __gen(chip_info):
    case_groups = []
    case_groups.extend(__get_case('ARGB8888_linear_1280x720_bear.bmp', 0, 0, "AR24", 1280, 720))
    case_groups.extend(__get_case('ARGB8888_linear_1280x720_bear.bmp', 1, 1, "AR24", 1280, 720))
    return  case_groups
