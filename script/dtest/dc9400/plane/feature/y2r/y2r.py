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
    return 'plane%d.feature.y2r.'%id + subname

def __get_property(hp, vp):
    p = OrderedDict()
    p["hPhase"] = hp
    p["vPhase"] = vp
    return OrderedDict([('UP_SAMPLE', p)])

def __get_ori_property(colorEncoding, colorRange):
    p1 = OrderedDict()
    p1["yuvColorEncoding"] = colorEncoding
    p1["yuvColorRange"] = colorRange
    return OrderedDict(p1)

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

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

def _get_case(plane_id, format, width, height, resource):
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

    case_list = []
    colorEncoding_list = ["BT601", "BT709", "BT2020"]
    colorRange_list = ["LIMITED", "FULL"]
    upSample_h_range = (4,8,12,16)
    upSample_v_range = (4,8,12,16)
    for clr_enc in colorEncoding_list:
        for clr_ran in colorRange_list:
            for hp in upSample_h_range:
                for vp in upSample_v_range:
                    # ###can chose part of fixed cases to reduce cases number.
                    # if plane_id != 0:
                    #     if (hp ==vp) or (abs(hp-vp) <5) or (plane_id in [1,6] and clr_enc in [ "BT709"]) \
                    #             or (plane_id in [7,9] and clr_enc in ["BT601"] and clr_ran in ["FULL"]):
                    #         continue

                    crtc = Crtc(0)
                    plane = Plane(plane_id, crtc, format, width, height, in_refresh, __get_property(hp, vp), resource=resource, ori_properties=__get_ori_property(clr_enc, clr_ran))
                    conn = Connector(0, crtc, 'AR30',conn_width, conn_height, in_refresh, golden=__get_golden())
                    name_string = "{}_{}_H{}V{}_{}".format(clr_enc, clr_ran.lower().capitalize(), hp, vp, format)
                    case_list.append(DTestUnit(plane, conn, __get_name(plane_id, name_string)))
    return case_list

@dtest_unit_group('dc9400')
def __gen(chip_info):
    other_plane_cases = []
    for p in [0, 1, 2, 3, 6, 7, 8, 9]:
        case_group = _get_case(p,'YV12',640, 480,'Boston_YV12_640X480_Linear.vimg')
        other_plane_cases.extend(case_group)

    return  other_plane_cases