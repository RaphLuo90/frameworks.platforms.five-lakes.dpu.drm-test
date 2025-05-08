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
    return 'crtc%d.feature.postpq.'%id + subname

display_mode_list = ["640x480x60", "1280x720x60", "1920x1080x60", "1440x3520x60",
                   "2700x2600x60", "2500x2820x60", "2340x3404x60", "3200x1920x60", "3840x2160x60"]

def __get_mode_info(mode):
    modeinfo = dict()
    m = re.search("(\d+)x(\d+)x(\d+)", mode, re.I)
    if m:
        modeinfo['width'] = int(m.group(1))
        modeinfo['height'] = int(m.group(2))
        modeinfo['frame'] = int(m.group(3))
    return modeinfo

def __get_case(resource, crtc_id, conn_id, format, width, height):
    def __get_property(index_type, sw_index=2):
        p = OrderedDict()
        p["index_type"] = index_type
        if index_type in [1]:  ##only sw (1) need
            p["sw_index"] = sw_index

        p["table_low"] = [1850516096, 1850516096, 1850516096]
        p["table_high"] = [1568641459, 1568641459, 1568641459]
        return OrderedDict([('PANEL_DITHER', p)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    conn_width = width
    conn_height = height
    in_refresh = 60
    for output_mode in display_mode_list:
        output_modeinfo = __get_mode_info(output_mode)
        if output_modeinfo['width'] >= width and output_modeinfo['height'] >= height:
            conn_width = output_modeinfo['width']
            conn_height = output_modeinfo['height']
            in_refresh = output_modeinfo['frame']
            break

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
                crtc = Crtc(crtc_id, prop)
                plane = Plane(0, crtc, format, width, height, in_refresh, resource=resource)
                conn = Connector(conn_id, crtc, 'AR30', conn_width, conn_height, in_refresh, golden=__get_golden())
                if conn_id == 1:
                    case_groups.append(
                        DTestUnit(plane, conn, __get_name(crtc_id, 'panel_dither_{}_{}_connector{}'.format(name, sw_index, conn_id))))
                else:
                    case_groups.append(
                        DTestUnit(plane, conn, __get_name(crtc_id, 'panel_dither_{}_{}'.format(name, sw_index))))
        else:
            prop = __get_property(val)
            crtc = Crtc(crtc_id, prop)
            plane = Plane(0, crtc, format, width, height, in_refresh, resource=resource)
            conn = Connector(conn_id, crtc, 'AR30', conn_width, conn_height, in_refresh, golden=__get_golden())
            if conn_id == 1:
                case_groups.append(DTestUnit(plane, conn, __get_name(crtc_id, 'panel_dither_{}_connector{}'.format(name, conn_id))))
            else:
                case_groups.append(DTestUnit(plane, conn, __get_name(crtc_id, 'panel_dither_{}'.format(name))))

    return case_groups

@dtest_unit_group('dc9400')
def __gen(chip_info):
    crtc_list = [0, 1, 2, 3]
    case_groups = []
    for crtc_id in crtc_list:
        conn_id = 0
        case_groups.extend(__get_case('ARGB8888_linear_1280x720_bear.bmp', crtc_id, conn_id, "AR24", 1280, 720))

    ##CRTC4 + WB2
    case_groups.extend(__get_case('ARGB8888_linear_1280x720_bear.bmp', 4, 2, "AR24", 1280, 720))

    ##CRTC1 + WB1
    case_groups.extend(__get_case('ARGB8888_linear_1280x720_bear.bmp', 1, 1, "AR24", 1280, 720))

    return  case_groups
