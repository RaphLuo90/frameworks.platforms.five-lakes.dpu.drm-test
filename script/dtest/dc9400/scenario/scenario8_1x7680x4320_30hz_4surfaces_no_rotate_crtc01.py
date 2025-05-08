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
    return 'scenario_case8_1x7680x4320_30hz_4surfaces_no_rotate_crtc01'

def __get_property_plane(plane_id):
    p = OrderedDict()
    p['dmaMode'] = 'oneROI'
    if plane_id in [0,2]:
        p['roiX'] = 0
    elif plane_id in [1,3]:
        p['roiX'] = 3840
    p['roiY'] = 0
    p['roiWidth'] = 3840
    p['roiHeight'] = 4320
    p['layerOutX'] = 0
    p['layerOutY'] = 0
    p['layerOutWidth'] = 3840
    p['layerOutHeight'] = 4320
    if plane_id in [0, 1, 2]:
        return OrderedDict([('DMA_CONFIG', p)])
    if plane_id in [3]:
        p1 = OrderedDict()
        p1['hPhase'] = 16
        p1['vPhase'] = 16
        return OrderedDict([('DMA_CONFIG', p), ('UP_SAMPLE', p1)])

def __get_property_crtc():
    p = OrderedDict()
    p['splice0Enable'] = 1
    p['splice0CrtcMask'] = 'crtc0, crtc1'
    p['splice0OutputIntf'] = 0
    p['srcPanelWidth0'] = 3840
    p['srcPanelWidth1'] = 3840
    return OrderedDict([('SPLICE_MODE', p)])

def __get_golden():
    return OrderedDict()

def __get_case():
    f0_crtc0 = Crtc(0)
    f0_plane0 = Plane(0, f0_crtc0, 'AR24', 640, 480, 60, crtcX=0, crtcY=0, crtcWidth=640, crtcHeight=480)
    f0_conn0 = Connector(0, f0_crtc0, 'AR30', 640, 480, 60, golden=__get_golden())
    frame0 = DTestUnit(f0_plane0, f0_conn0, __get_name(), frameID=0)

    f1_crtc0 = Crtc(0, __get_property_crtc())
    f1_crtc1 = Crtc(1)
    f1_plane0 = Plane(0, f1_crtc0, 'AR30', 7680, 4320, 30, __get_property_plane(0), resource='ARGB2101010_7680x4320_TILE8x8_UNIT2x2.raw', crtcX=0, crtcY=0, \
                    tileMode="TILE_8X8_UNIT2X2", crtcWidth=7680, crtcHeight=4320, ori_properties=OrderedDict([('zpos', 0)]))

    f1_plane1 = Plane(1, f1_crtc1, 'AR30', 7680, 4320, 30, __get_property_plane(1), resource='ARGB2101010_7680x4320_TILE_8x8_UNIT2x2_dec400_compress.raw', \
                    decType="DEC", tileMode="TILE_8X8_UNIT2X2", crtcX=0, crtcY=0, crtcWidth=7680, crtcHeight=4320, \
                    ori_properties=OrderedDict([('zpos', 0)]), tileStatus='ARGB2101010_7680x4320_TILE_8x8_UNIT2x2_dec400_compress.ts')

    f1_plane2 = Plane(2, f1_crtc0, 'P010_UV', 7680, 4320, 30, __get_property_plane(2), resource='P010_UV_LINEAR_7680X4320.raw', crtcX=0, crtcY=0, \
                    crtcWidth=7680, crtcHeight=4320, ori_properties=OrderedDict([('zpos', 1)]))

    f1_plane3 = Plane(3, f1_crtc1, 'P010_UV', 7680, 4320, 30, __get_property_plane(3), resource='P010_UV_LINEAR_7680X4320.raw', crtcX=0, crtcY=0, \
                    crtcWidth=7680, crtcHeight=4320, ori_properties=OrderedDict([('zpos', 1), ('yuvColorEncoding', 'BT709'), ('yuvColorRange', 'FULL')]))

    f1_plane4 = Plane(4, f1_crtc0, 'AR24', 256, 256, 30, crtcX=0, crtcY=0, \
                    crtcWidth=256, crtcHeight=256, ori_properties=OrderedDict([('zpos', 2)]))

    f1_plane5 = Plane(5, f1_crtc1, 'AR24', 256, 256, 30, crtcX=0, crtcY=0, \
                    crtcWidth=256, crtcHeight=256, ori_properties=OrderedDict([('zpos', 2)]))

    f1_conn0 = Connector(0, f1_crtc0, 'AR30', 7680, 4320, 30, golden=__get_golden(), other_prop=OrderedDict([('wb_width', 3840), ('wb_height', 4320)]))
    f1_conn1 = Connector(1, f1_crtc1, 'AR30', 7680, 4320, 30, golden=__get_golden(), other_prop=OrderedDict([('wb_width', 3840), ('wb_height', 4320)]))

    f1_planes = [f1_plane0, f1_plane1, f1_plane2, f1_plane3, f1_plane4, f1_plane5]
    f1_conns = [f1_conn0, f1_conn1]
    frame1 = DTestUnit(f1_planes, f1_conns, __get_name(), frameID=1)
    frames = [frame0, frame1]
    return DTestFrameUnit(frames, __get_name())

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [__get_case()]