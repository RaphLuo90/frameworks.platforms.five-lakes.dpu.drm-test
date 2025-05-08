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
    return 'multiCrtc.spliter.' + subname

def __get_property_crtc0():
    p = OrderedDict()
    p['leftX'] = 0
    p['leftWidth'] = 600
    p['rightX'] = 480
    p['rightWidth'] = 600
    p['srcWidth'] = 1080
    return OrderedDict([('SPLITER', p)])

def __get_other_property():
        p = OrderedDict()
        p["wb_width"] = 600
        p["wb_height"] = 2400
        return OrderedDict(p)

def __get_ori_properties():
        p = OrderedDict()
        p["globalDisable"] = 1
        return OrderedDict(p)

def __get_conn_property():
    p = OrderedDict()
    p['wbpoint'] = 'ofifoIn'
    return OrderedDict([('WB_POINT', p)])

def __get_golden():
    return OrderedDict()

def __get_case(crtc_0, crtc_1, subname):
    crtc0 = Crtc(crtc_0, __get_property_crtc0())
    crtc1 = Crtc(crtc_1)

    plane0 = Plane(0, crtc0, 'AR24', 1080, 2400, 60, resource='defaultPattern', crtcX=0, crtcY=0, crtcWidth=1080, crtcHeight=2400)
    plane1 = Plane(1, crtc1, 'AR24', 1080, 2400, 60, resource='defaultPattern', crtcX=0, crtcY=0, crtcWidth=1080, crtcHeight=2400, ori_properties = __get_ori_properties())
    planes = [plane0, plane1]

    conn0 = Connector(0, crtc0, 'AR30', 1080, 2400, 60, __get_conn_property(), golden=__get_golden(), other_prop = __get_other_property())
    conn1 = Connector(1, crtc1, 'AR30', 1080, 2400, 60, __get_conn_property(), golden=__get_golden(), other_prop = __get_other_property())
    conns = [conn0, conn1]

    return DTestUnit(planes, conns, __get_name(subname))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(0, 1,'crtc01')]