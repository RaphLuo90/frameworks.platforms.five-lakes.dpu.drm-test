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
    return 'crtc%d.feature.'%id + subname

def _get_name_new(crtc_id, conn_id, subname):
    return 'crtc%d.feature.'%crtc_id + subname + '.connecor%d'%conn_id

class R2Y:
    @staticmethod
    def get_case(crtc_id, conn_id):
        def __get_property():
            p = OrderedDict()
            p["mode"] = 'USR'
            p["gamut"] = 'BT2020'
            p["busFormat"] = 'YUV422_10BIT'
            p["coef"] = [1076, 2777, 243, -572, -1476, 2048, 2048, -1883, -165, 0, 0, 0, 0, 2048, 2048]
            return OrderedDict([('R2Y', p)])

        def __get_golden():
            # Fill golden
            #return OrderedDict([('md5sum', '1234567')])
            return OrderedDict()

        crtc = Crtc(crtc_id, __get_property())
        plane = Plane(0, crtc, 'AR24', 640, 480, 60)
        conn = Connector(conn_id, crtc, 'YUV444_10BIT', 640, 480, 60, golden=__get_golden())
        return DTestUnit(plane, conn, _get_name(crtc_id, 'r2y'))

class R2Y_crtc4:
    @staticmethod
    def get_case(crtc_id, conn_id):
        def __get_property():
            p = OrderedDict()
            p["mode"] = 'USR'
            p["gamut"] = 'BT2020'
            p["coef"] = [1076, 2777, 243, -572, -1476, 2048, 2048, -1883, -165, 0, 0, 0, 0, 2048, 2048]
            return OrderedDict([('R2Y', p)])

        def __get_golden():
            # Fill golden
            #return OrderedDict([('md5sum', '1234567')])
            return OrderedDict()

        crtc = Crtc(crtc_id, __get_property())
        plane = Plane(0, crtc, 'AR24', 640, 480, 60)
        conn = Connector(conn_id, crtc, 'NV12', 640, 480, 60, golden=__get_golden())
        return DTestUnit(plane, conn, _get_name(crtc_id, 'r2y'))

class R2Y_new:
    @staticmethod
    def get_case(crtc_id, conn_id):
        def __get_property():
            p = OrderedDict()
            p["mode"] = 'USR'
            p["gamut"] = 'BT2020'
            p["busFormat"] = 'YUV422_10BIT'
            p["coef"] = [1076, 2777, 243, -572, -1476, 2048, 2048, -1883, -165, 0, 0, 0, 0, 2048, 2048]
            return OrderedDict([('R2Y', p)])

        def __get_golden():
            # Fill golden
            #return OrderedDict([('md5sum', '1234567')])
            return OrderedDict()

        crtc = Crtc(crtc_id, __get_property())
        plane = Plane(0, crtc, 'AR24', 640, 480, 60)
        conn = Connector(conn_id, crtc, 'NV12', 640, 480, 60, golden=__get_golden())
        return DTestUnit(plane, conn, _get_name_new(crtc_id, conn_id, 'r2y'))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ R2Y.get_case(0, 1), R2Y.get_case(1, 0), R2Y.get_case(2, 0), R2Y.get_case(3, 0), R2Y_crtc4.get_case(4, 2),
        R2Y_new.get_case(0, 0), R2Y_new.get_case(1, 1)]

class R2YUVDownsample:
    @staticmethod
    def get_case(crtc_id, conn_id):
        def __get_property():
            r2y = OrderedDict()
            r2y["mode"] = 'USR'
            r2y["gamut"] = 'BT2020'
            r2y["busFormat"] = 'YUV422_10BIT'
            r2y["coef"] = [1076, 2777, 243, -572, -1476, 2048, 2048, -1883, -165, 0, 0, 0, 0, 2048, 2048]
            uvds = OrderedDict()
            uvds["h_mode"] = 'FILTER'
            uvds["v_mode"] = 'FILTER'
            return OrderedDict([('R2Y', r2y), ('DOWN_SAMPLE', uvds)])

        def __get_golden():
            # Fill golden
            #return OrderedDict([('md5sum', '1234567')])
            return OrderedDict()

        crtc = Crtc(crtc_id, __get_property())
        plane = Plane(0, crtc, 'AR24', 640, 480, 60)
        conn = Connector(conn_id, crtc, 'YUV444_10BIT', 640, 480, 60, golden=__get_golden())
        return DTestUnit(plane, conn, _get_name(crtc_id, 'r2y_uv_downsample'))

class R2YUVDownsample_crtc4:
    @staticmethod
    def get_case(crtc_id, conn_id):
        def __get_property():
            r2y = OrderedDict()
            r2y["mode"] = 'USR'
            r2y["gamut"] = 'BT2020'
            r2y["coef"] = [1076, 2777, 243, -572, -1476, 2048, 2048, -1883, -165, 0, 0, 0, 0, 2048, 2048]
            uvds = OrderedDict()
            uvds["h_mode"] = 'FILTER'
            uvds["v_mode"] = 'FILTER'
            return OrderedDict([('R2Y', r2y), ('DOWN_SAMPLE', uvds)])

        def __get_golden():
            # Fill golden
            #return OrderedDict([('md5sum', '1234567')])
            return OrderedDict()

        crtc = Crtc(crtc_id, __get_property())
        plane = Plane(0, crtc, 'AR24', 640, 480, 60)
        conn = Connector(conn_id, crtc, 'NV12', 640, 480, 60, golden=__get_golden())
        return DTestUnit(plane, conn, _get_name(crtc_id, 'r2y_uv_downsample'))

class R2YUVDownsample_new:
    @staticmethod
    def get_case(crtc_id, conn_id):
        def __get_property():
            r2y = OrderedDict()
            r2y["mode"] = 'USR'
            r2y["gamut"] = 'BT2020'
            r2y["busFormat"] = 'YUV422_10BIT'
            r2y["coef"] = [1076, 2777, 243, -572, -1476, 2048, 2048, -1883, -165, 0, 0, 0, 0, 2048, 2048]
            uvds = OrderedDict()
            uvds["h_mode"] = 'FILTER'
            uvds["v_mode"] = 'FILTER'
            return OrderedDict([('R2Y', r2y), ('DOWN_SAMPLE', uvds)])

        def __get_golden():
            # Fill golden
            #return OrderedDict([('md5sum', '1234567')])
            return OrderedDict()

        crtc = Crtc(crtc_id, __get_property())
        plane = Plane(0, crtc, 'AR24', 640, 480, 60)
        conn = Connector(conn_id, crtc, 'NV12', 640, 480, 60, golden=__get_golden())
        return DTestUnit(plane, conn, _get_name_new(crtc_id, conn_id, 'r2y_uv_downsample'))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ R2YUVDownsample.get_case(0, 0), R2YUVDownsample.get_case(1, 1), R2YUVDownsample.get_case(2, 0), \
            R2YUVDownsample.get_case(3, 0), R2YUVDownsample_crtc4.get_case(4, 2), R2YUVDownsample_new.get_case(0, 0),
            R2YUVDownsample_new.get_case(1, 1)]