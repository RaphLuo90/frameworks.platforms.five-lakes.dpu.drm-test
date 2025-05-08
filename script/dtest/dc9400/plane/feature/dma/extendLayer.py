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

def __get_property(format):
    p = OrderedDict()
    p["dmaMode"] ='extendLayer'
    p["layerOutX"] = 0
    p["layerOutY"] = 0
    p["layerOutWidth"] = 320
    p["layerOutHeight"] = 240
    p["layerOutExX"] = 0
    p["layerOutExY"] = 300
    p["layerOutExWidth"] = 400
    p["layerOutExHeight"] = 400
    p1 = OrderedDict()
    p1["format"] = format
    p1["width"] = 400
    p1["height"] = 400
    return OrderedDict([('DMA_CONFIG', p), ('EXT_LAYER_FB', p1)])

def __get_property_Tile16X4(format):
    p = OrderedDict()
    p["dmaMode"] = 'extendLayer'
    p["layerOutX"] = 0
    p["layerOutY"] = 0
    p["layerOutWidth"] = 640
    p["layerOutHeight"] = 480
    p["layerOutExX"] = 0
    p["layerOutExY"] = 560
    p["layerOutExWidth"] = 640
    p["layerOutExHeight"] = 480
    p1 = OrderedDict()
    p1["format"] = format
    p1["resource"] = "ARGB2101010_Tile16x4_640x480_lossless.raw"
    p1["decType"] = "PVRIC"
    p1["tileMode"] = "TILE16X4"
    p1["width"] = 640
    p1["height"] = 480
    return OrderedDict([('DMA_CONFIG', p), ('EXT_LAYER_FB', p1)])


def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

def __get_case(plane_id):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'AR24', 320, 240, 60, __get_property('AR24'))
    conn = Connector(0, crtc, 'AR30', 1280, 720, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'extendLayer_LINEAR'))

def __get_case_Tile16X4(plane_id):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'AR30', 640, 480, 60, __get_property_Tile16X4('AR30'), resource='ARGB2101010_Tile16x4_640x480_lossless.raw', decType="PVRIC", tileMode= "TILE16X4")
    conn = Connector(0, crtc, 'AR30', 1920, 1080, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'extendLayer_TILE16X4'))


@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(i) for i in range(0, chip_info.plane_num) ] \
           + [__get_case_Tile16X4(i) for i in (0, 1, 6, 7) ]
