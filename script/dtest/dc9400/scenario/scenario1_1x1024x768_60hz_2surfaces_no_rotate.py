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

def __get_name():
    return 'scenario_case1_1x1024x768_60hz_2surfaces_no_rotate'

def __get_property():
    p = OrderedDict()
    p['enable'] = 1
    p['src_w'] = 1024
    p['src_h'] = 768
    p['dst_w'] = 1920
    p['dst_h'] = 1080
    p["filter"] = 'H9V5'
    return OrderedDict([('SCALER', p)])

def __get_golden():
    return OrderedDict()

def __get_case():
    crtc = Crtc(0)
    plane = Plane(0, crtc, 'AR24', 1024, 768, 60, __get_property(), resource="LINEAR_ARGB8888_1024x768.bmp", crtcX=0, crtcY=0, crtcWidth=1920, crtcHeight=1080, ori_properties=OrderedDict([('zpos', 0)]))
    cursor = Cursor(0, crtc,'AR24', cursorPosX=0, cursorPosY=0, cursorWidth=8, cursorHeight=8, cursorHotX=0, cursorHotY=0, properties=OrderedDict([('zpos', 2)]))
    conn = Connector(0, crtc, 'AR30', 1920, 1080, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(), cursors=cursor)

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [__get_case()]
