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

def _get_name(id, subname):
    return 'connector%d.feature.compress.'%id + subname

writeback_YUV_list = ["NV12", "P010_UV"]
def __get_case(crtc_id, conn_id, conn_format, conn_tile, lossy):
    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    def __get_r2y_property():
        p = OrderedDict()
        p["mode"] = 'F2F'
        p["gamut"] = 'BT2020'
        return OrderedDict([('R2Y', p)])

    def __get_other_property(conn_tile,custom,lossy):
        p = OrderedDict()
        p["decType"] = "PVRIC"
        p["tileMode"] = conn_tile
        if custom:
            p["customFormat"] = 1
        if lossy:
            p["lossy"] = 1
        else:
            p["lossy"] = 0
        return OrderedDict(p)

    if conn_format in writeback_YUV_list:
        crtc = Crtc(crtc_id, properties=__get_r2y_property())
    else:
        crtc = Crtc(crtc_id)
    custom = 0
    if conn_format in custom_format:
        custom =1
    plane = Plane(0, crtc, 'AR24', 640, 480, 60, resource='00021_32bpp_formatARGB8888_640x480_LINEAR.bmp')
    conn = Connector(conn_id, crtc, conn_format, 640, 480, 60, golden=__get_golden(), other_prop = __get_other_property(conn_tile,custom,lossy))
    if lossy:
        return DTestUnit(plane, conn, _get_name(conn_id, "{}_{}_{}_{}".format(conn_format, conn_tile, "PVRIC", "lossy"))) # decType = "PVRIC"
    else:
        return DTestUnit(plane, conn, _get_name(conn_id, "{}_{}_{}_{}".format(conn_format, conn_tile, "PVRIC", "lossless"))) # decType = "PVRIC"

custom_format = ['P010_VU','P210_VU']

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [__get_case(4, 2, "BA24", "TILE16X4", 0)] \
           +[__get_case(4, 2, "BX24", "TILE16X4", 0)] \
           +[__get_case(4, 2, "RG16", "TILE16X4", 0)] \
           +[__get_case(4, 2, "AR30", "TILE16X4", 0)] \
           +[__get_case(4, 2, "NV12", "TILE8X8", 0)] \
           +[__get_case(4, 2, "P010_UV", "TILE8X8", 0)]