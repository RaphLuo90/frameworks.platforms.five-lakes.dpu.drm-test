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
    return 'plane%d.feature.multi_extendlayer.'%id + subname

def __get_property_one_extendlayer(format):
    p = OrderedDict()
    p["format"] = format
    p["decType"] = 'NONE'
    p["tileMode"] = 'LINEAR'
    p["resourceEx0"] = 'defaultPattern'
    p["width_Ex0"] = 320
    p["height_Ex0"] = 240
    p["layerOutEx0X"] = 300
    p["layerOutEx0Y"] = 280
    p["layerOutEx0Width"] = 320
    p["layerOutEx0Height"] = 240
    return OrderedDict([('MULTI_EXT_LAYER', p)])

def __get_property_two_extendlayer(format):
    p = OrderedDict()
    p["format"] = format
    p["decType"] = 'NONE'
    p["tileMode"] = 'LINEAR'
    p["resourceEx0"] = 'defaultPattern'
    p["width_Ex0"] = 320
    p["height_Ex0"] = 240
    p["layerOutEx0X"] = 300
    p["layerOutEx0Y"] = 280
    p["layerOutEx0Width"] = 320
    p["layerOutEx0Height"] = 240
    p["resourceEx1"] = 'defaultPattern'
    p["width_Ex1"] = 320
    p["height_Ex1"] = 240
    p["layerOutEx1X"] = 600
    p["layerOutEx1Y"] = 560
    p["layerOutEx1Width"] = 320
    p["layerOutEx1Height"] = 240
    return OrderedDict([('MULTI_EXT_LAYER', p)])

def __get_property_three_extendlayer(format):
    p = OrderedDict()
    p["format"] = format
    p["decType"] = 'NONE'
    p["tileMode"] = 'LINEAR'
    p["resourceEx0"] = 'defaultPattern'
    p["width_Ex0"] = 320
    p["height_Ex0"] = 240
    p["layerOutEx0X"] = 300
    p["layerOutEx0Y"] = 280
    p["layerOutEx0Width"] = 320
    p["layerOutEx0Height"] = 240
    p["resourceEx1"] = 'defaultPattern'
    p["width_Ex1"] = 320
    p["height_Ex1"] = 240
    p["layerOutEx1X"] = 600
    p["layerOutEx1Y"] = 560
    p["layerOutEx1Width"] = 320
    p["layerOutEx1Height"] = 240
    p["resourceEx2"] = 'defaultPattern'
    p["width_Ex2"] = 320
    p["height_Ex2"] = 240
    p["layerOutEx2X"] = 900
    p["layerOutEx2Y"] = 840
    p["layerOutEx2Width"] = 320
    p["layerOutEx2Height"] = 240
    return OrderedDict([('MULTI_EXT_LAYER', p)])

def __get_property_dec_extendlayer(format):
    p = OrderedDict()
    p["format"] = format
    p["decType"] = 'DEC'
    p["tileMode"] = 'TILE_32X8_YUVSP8X8'
    p["resourceEx0"] = 'compress_NV12_TILE_32X8_YUVSP_8X8_640X480_default.raw'
    p["tileStatusEx0"] = 'compress_NV12_TILE_32X8_YUVSP_8X8_640X480_default.raw.y.ts'
    p["tileStatus1Ex0"] = 'compress_NV12_TILE_32X8_YUVSP_8X8_640X480_default.raw.uv.ts'
    p["width_Ex0"] = 640
    p["height_Ex0"] = 480
    p["layerOutEx0X"] = 640
    p["layerOutEx0Y"] = 520
    p["layerOutEx0Width"] = 640
    p["layerOutEx0Height"] = 480
    p["resourceEx1"] = 'compress_NV12_TILE_32X8_YUVSP_8X8_640X480_default.raw'
    p["tileStatusEx1"] = 'compress_NV12_TILE_32X8_YUVSP_8X8_640X480_default.raw.y.ts'
    p["tileStatus1Ex1"] = 'compress_NV12_TILE_32X8_YUVSP_8X8_640X480_default.raw.uv.ts'
    p["width_Ex1"] = 640
    p["height_Ex1"] = 480
    p["layerOutEx1X"] = 1280
    p["layerOutEx1Y"] = 1040
    p["layerOutEx1Width"] = 640
    p["layerOutEx1Height"] = 480
    p["resourceEx2"] = 'compress_NV12_TILE_32X8_YUVSP_8X8_640X480_default.raw'
    p["tileStatusEx2"] = 'compress_NV12_TILE_32X8_YUVSP_8X8_640X480_default.raw.y.ts'
    p["tileStatus1Ex2"] = 'compress_NV12_TILE_32X8_YUVSP_8X8_640X480_default.raw.uv.ts'
    p["width_Ex2"] = 640
    p["height_Ex2"] = 480
    p["layerOutEx2X"] = 1920
    p["layerOutEx2Y"] = 1560
    p["layerOutEx2Width"] = 640
    p["layerOutEx2Height"] = 480
    return OrderedDict([('MULTI_EXT_LAYER', p)])

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

def __get_case_one_extlayer(plane_id):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'AR24', 320, 240, 60, __get_property_one_extendlayer('AR24'))
    conn = Connector(0, crtc, 'AR30', 1920, 1080, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'one_ext_LINEAR'))

def __get_case_two_extlayer(plane_id):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'AR24', 320, 240, 60, __get_property_two_extendlayer('AR24'))
    conn = Connector(0, crtc, 'AR30', 1920, 1080, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'two_ext_LINEAR'))

def __get_case_three_extlayer(plane_id):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'AR24', 320, 240, 60, __get_property_three_extendlayer('AR24'))
    conn = Connector(0, crtc, 'AR30', 1920, 1080, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'three_ext_LINEAR'))

def __get_case_dec_extlayer(plane_id, resource, decType, tileMode, tileStatus):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'NV12', 640, 480, 60, __get_property_dec_extendlayer('NV12'), crtcWidth=640, crtcHeight=480, resource=resource, decType=decType, tileMode=tileMode, tileStatus=tileStatus)
    conn = Connector(0, crtc, 'AR30', 3840, 2160, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'DEC_LINEAR'))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case_one_extlayer(i) for i in (0, 3) ] \
           + [__get_case_two_extlayer(i) for i in (1, 4) ] \
           + [__get_case_three_extlayer(i) for i in (2, 5) ] \
           + [__get_case_dec_extlayer(i, "compress_NV12_TILE_32X8_YUVSP_8X8_640X480_default.raw", "DEC", "TILE_32X8_YUVSP8X8", [("tileStatus","compress_NV12_TILE_32X8_YUVSP_8X8_640X480_default.raw.y.ts"), ("tileStatus1","compress_NV12_TILE_32X8_YUVSP_8X8_640X480_default.raw.uv.ts")]) for i in (0, 1, 2) ]
