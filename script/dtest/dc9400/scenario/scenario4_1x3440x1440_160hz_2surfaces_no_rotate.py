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
    return 'scenario_case4_1x3440x1440_160hz_2surfaces_no_rotate'

def __get_ori_property():
    p = OrderedDict()
    p['zpos'] = 1
    p['yuvColorEncoding'] = 'BT2020'
    p['yuvColorRange'] = 'FULL'
    return p

def __get_property():
    p = OrderedDict()
    p['hPhase'] = 16
    p['vPhase'] = 4
    return OrderedDict([('UP_SAMPLE', p)])

def __get_crtc_property():
    p = OrderedDict()
    p['mode'] = 'USR'
    p['gamut'] = 'BT2020'
    p['busFormat'] = 'YUV444_10BIT'
    p['coef'] = [1076,2777,243,-572,-1476,2048,2048,-1883,-165,0,0,0,0,2048,2048]
    return OrderedDict([('R2Y', p)])

def __get_golden():
    return OrderedDict()

def __get_case():
    crtc = Crtc(0, __get_crtc_property())
    plane0 = Plane(0, crtc, 'RA30', 3440, 1440, 160, resource='RA30_3440x1440_TILE_8x8_UNIT2x2_compress.raw', decType='DEC', tileMode='TILE_8X8_UNIT2X2',\
                    crtcX=0, crtcY=0, crtcWidth=3440, crtcHeight=1440, ori_properties=OrderedDict([('zpos', 0)]), tileStatus='RA30_3440x1440_TILE_8x8_UNIT2x2.ts')

    plane1 = Plane(2, crtc, 'P210_UV', 2560, 1440, 160, __get_property(), resource='r2y_2560x1440_P210_UV.raw', \
                    crtcX=100, crtcY=0, crtcWidth=2560, crtcHeight=1440, ori_properties=__get_ori_property())

    planes = [plane0, plane1]
    cursor = Cursor(0, crtc, 'AR24', cursorPosX=1088, cursorPosY=721, cursorWidth=32, cursorHeight=32, cursorHotX=0, cursorHotY=0)
    conn = Connector(0, crtc, 'YUV444_10BIT', 3440, 1440, 160, golden=__get_golden())
    return DTestUnit(planes, conn, __get_name(), cursors=cursor)

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [__get_case()]