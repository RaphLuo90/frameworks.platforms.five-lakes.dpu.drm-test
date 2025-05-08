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

def __get_name(id, subname):
    return 'plane%d.feature.dma.'%id + subname

def __get_property():
    p = OrderedDict()
    p["dmaMode"] ='extendLayerEx'
    p["roiX"] = 0
    p["roiY"] = 0
    p["roiWidth"] = 240
    p["roiHeight"] = 200
    p["layerOutX"] = 10
    p["layerOutY"] = 10
    p["layerOutWidth"] = 240
    p["layerOutHeight"] = 200
    p["roiExX"] = 240
    p["roiExY"] = 240
    p["roiExWidth"] = 240
    p["roiExHeight"] = 240
    p["layerOutExX"] = 320
    p["layerOutExY"] = 240
    p["layerOutExWidth"] = 240
    p["layerOutExHeight"] = 240
    p1 = OrderedDict()
    p1["format"] ='RG16'
    p1["width"] = 640
    p1["height"] = 480
    return OrderedDict([('DMA_CONFIG', p), ('EXT_LAYER_FB', p1)])

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

def __get_case(plane_id):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'RG16', 640, 480, 60, __get_property(), resource="00025_16bpp_formatRGB565_640x480_LINEAR.bmp")
    conn = Connector(0, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'extendLayerEx_LINEAR'))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(i) for i in range(0, chip_info.plane_num) ]

