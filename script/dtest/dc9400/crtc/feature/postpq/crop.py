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

def __get_name(id):
    return 'crtc%d.feature.postpq.crop'%id

def __get_name_displayout(id):
    return 'crtc%d.feature.postpq.crop.displayout'%id

def __get_name_displayout1(crtc_id, con_id):
    return 'crtc{}.feature.postpq.crop.displayout.connector{}'.format(crtc_id, con_id)

def __get_property():
    p = OrderedDict()
    p['cropX'] = 0
    p['cropY'] = 0
    p['cropWidth'] = 640
    p['cropHeight'] = 480
    p['srcPanelWidth'] = 1920
    p['srcPanelHeight'] = 1080
    return OrderedDict([('PANEL_CROP', p)])

def __get_wb_size_decType(wb_width, wb_height):
    p1 = OrderedDict()
    p1["wb_width"] = wb_width
    p1["wb_height"] = wb_height
    return OrderedDict(p1)

def __get_conn_property():
    p = OrderedDict()
    p['wbpoint'] = 'ofifoIn'
    return OrderedDict([('WB_POINT', p)])

def __get_golden():
    return OrderedDict()

def __get_case_displayout(crtc_id):
    crtc = Crtc(crtc_id, __get_property())
    plane = Plane(0, crtc, 'AR24', 1920, 1080, 60, resource='VV_Background_1920x1080.ARGB8.bmp', crtcX=0, crtcY=0, crtcWidth=1920, crtcHeight=1080)
    conn = Connector(0, crtc, 'AR30', 1920, 1080, 60, other_prop=__get_wb_size_decType(640, 480), golden=__get_golden())
    return DTestUnit(plane, conn, __get_name_displayout(crtc_id))

def __get_case_displayout1(crtc_id):
    crtc = Crtc(crtc_id, __get_property())
    plane = Plane(0, crtc, 'AR24', 1920, 1080, 60, resource='VV_Background_1920x1080.ARGB8.bmp', crtcX=0, crtcY=0, crtcWidth=1920, crtcHeight=1080)
    conn = Connector(1, crtc, 'AR30', 1920, 1080, 60, other_prop=__get_wb_size_decType(640, 480), golden=__get_golden())
    return DTestUnit(plane, conn, __get_name_displayout1(crtc_id, 1))

def __get_case(crtc_id):
    crtc = Crtc(crtc_id, __get_property())
    plane = Plane(0, crtc, 'AR24', 1920, 1080, 60, resource='VV_Background_1920x1080.ARGB8.bmp', crtcX=0, crtcY=0, crtcWidth=1920, crtcHeight=1080)
    conn = Connector(0, crtc, 'AR30', 640, 480, 60, __get_conn_property(), golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(crtc_id))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    case_list = list()
    crtc_list0 = [0, 1, 2, 3]
    crtc_list1 = [0, 1]
    for crtc_id in crtc_list0:
        case_list.append(__get_case(crtc_id))
    for crtc_id in crtc_list1:
        case_list.append(__get_case_displayout(crtc_id))
    case_list.append(__get_case_displayout1(1))
    return case_list