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
    return 'scenario_case7_1x5120x2880_120hz_2surfaces_flipy'

def __get_ori_property_p0():
    p = OrderedDict()
    p['rotation'] = 'FLIPY'
    p['zpos'] = 0
    return p

def __get_ori_property_p1():
    p = OrderedDict()
    p['rotation'] = 'FLIPY'
    p['zpos'] = 1
    p['yuvColorEncoding'] = 'BT2020'
    p['yuvColorRange'] = 'LIMITED'
    return p

def __get_property():
    p = OrderedDict()
    p['hPhase'] = 16
    p['vPhase'] = 12
    return OrderedDict([('UP_SAMPLE', p)])

def __get_golden():
    return OrderedDict()

def __get_case():
    def __get_dsc_property():
        p = OrderedDict()
        p["picture_width"] = 5120
        p["picture_height"] = 2880
        p["slices_per_line"] = 4
        p["slice_height"] = 2880
        p["ss_num"] = 2
        p["split_panel_enable"] = True
        p["multiplex_mode_enable"] = True
        p["multiplex_out_sel"] = 0
        p["de_raster_enable"] = True
        p["multiplex_eoc_enable"] = True
        p["video_mode"] = True
        return OrderedDict([('DSC', p)])

    def __get_conn_property():
        p1 = OrderedDict()
        p1["wbPoint"] = 'ofifoOut'
        return OrderedDict([('WB_POINT', p1)])

    crtc = Crtc(0, __get_dsc_property())
    plane0 = Plane(0, crtc, 'AR30', 5120, 2880, 120, resource='ARGB2101010_5120x2880_TILE_8x8_UNIT2x2_dec400_compress.raw', decType='DEC', tileMode='TILE_8X8_UNIT2X2',\
                    crtcX=0, crtcY=0, crtcWidth=5120, crtcHeight=2880, ori_properties=__get_ori_property_p0(), tileStatus='ARGB2101010_5120x2880_TILE_8x8_UNIT2x2_dec400_compress.ts')

    plane1 = Plane(2, crtc, 'P210_UV', 1920, 1080, 120, __get_property(), resource='P210_UV_1920x1080_LINEAR.raw', \
                    crtcX=0, crtcY=0, crtcWidth=1920, crtcHeight=1080, ori_properties=__get_ori_property_p1())

    planes = [plane0, plane1]
    cursor = Cursor(0, crtc, 'AR24', cursorPosX=383, cursorPosY=221, cursorWidth=64, cursorHeight=64, cursorHotX=0, cursorHotY=0)
    conn = Connector(0, crtc, 'AR30', 5120, 2880, 120, __get_conn_property(), golden=__get_golden())
    return DTestUnit(planes, conn, __get_name(), cursors=cursor)

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [__get_case()]