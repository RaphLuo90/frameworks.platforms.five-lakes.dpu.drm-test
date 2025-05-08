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

def __get_name(subname):
    return 'crtc01.feature.display_compression.' + subname

def __get_property_conn():
    p = OrderedDict()
    p['wbpoint'] = 'ofifoIn'
    return OrderedDict([('WB_POINT', p)])

def __get_property_crtc_splice_dsc(crtc_id, width, height):
    p = OrderedDict()
    p['splice0Enable'] = 1
    p['splice0CrtcMask'] = 'crtc0, crtc1'
    p['splice0OutputIntf'] = 0
    p['srcPanelWidth0'] = width
    p['srcPanelWidth1'] = width

    p1 = OrderedDict()
    p1["picture_width"] = width
    p1["picture_height"] = height
    p1['slices_per_line'] = 4
    p1['slice_height'] = height
    p1['ss_num'] = 2
    p1['split_panel_enable'] = True
    p1['multiplex_mode_enable'] = True
    p1['multiplex_out_sel'] = 0
    p1['de_raster_enable'] = True
    p1['multiplex_eoc_enable'] = True
    p1['video_mode'] = True
    if crtc_id == 0:
        return OrderedDict([('SPLICE_MODE', p), ('DSC', p1)])
    elif crtc_id == 1:
        return OrderedDict([('DSC', p1)])

def __get_golden():
    return OrderedDict()

def __get_case(width, height, fresh, conn_width, conn_height):
    crtc0 = Crtc(0, __get_property_crtc_splice_dsc(0, width, height))
    crtc1 = Crtc(1, __get_property_crtc_splice_dsc(1, width, height))
    plane0 = Plane(0, crtc0, 'AR24', width, height, fresh, crtcX=0, crtcY=0, crtcWidth=width, crtcHeight=height)
    plane1 = Plane(1, crtc1, 'AR24', width, height, fresh, crtcX=0, crtcY=0, crtcWidth=width, crtcHeight=height)
    conn0 = Connector(0, crtc0, 'AR30', conn_width, conn_height, fresh, __get_property_conn(), \
                golden=__get_golden(), other_prop=OrderedDict([('wb_width', width), ('wb_height', height)]))
    conn1 = Connector(1, crtc1, 'AR30', conn_width, conn_height, fresh, __get_property_conn(), \
                golden=__get_golden(), other_prop=OrderedDict([('wb_width', width), ('wb_height', height)]))
    planes = [plane0, plane1]
    conns = [conn0, conn1]
    return DTestUnit(planes, conns, __get_name('dsc_splice.{}x{}'.format(conn_width, conn_height)))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [__get_case(3072, 3456, 60, 6144, 3456), __get_case(3840, 4320, 30, 7680, 4320)]
