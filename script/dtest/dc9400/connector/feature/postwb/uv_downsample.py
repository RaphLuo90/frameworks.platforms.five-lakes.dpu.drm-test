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
    return 'connector%d.feature.postwb.uv_downsample.'%id + subname

class R2YUVDownsample:
    @staticmethod
    def get_case(crtc_id, conn_id, mode):
        def __get_property():
            r2y = OrderedDict()
            r2y["mode"] = 'USR'
            r2y["gamut"] = 'BT2020'
            r2y["coef"] = [1076, 2777, 243, -572, -1476, 2048, 2048, -1883, -165, 0, 0, 0, 0, 2048, 2048]
            uvds = OrderedDict()
            uvds["h_mode"] = mode
            uvds["v_mode"] = mode
            return OrderedDict([('R2Y', r2y), ('DOWN_SAMPLE', uvds)])

        def __get_golden():
            # Fill golden
            #return OrderedDict([('md5sum', '1234567')])
            return OrderedDict()

        crtc = Crtc(crtc_id)
        plane = Plane(0, crtc, 'AR24', 640, 480, 60)
        conn = Connector(conn_id, crtc, 'NV12', 640, 480, 60, __get_property(), golden=__get_golden())
        return DTestUnit(plane, conn, _get_name(conn_id, mode))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ R2YUVDownsample.get_case(0, 0, 'FILTER'), R2YUVDownsample.get_case(0, 0, 'AVERAGE'), R2YUVDownsample.get_case(0, 0, 'DROP')]