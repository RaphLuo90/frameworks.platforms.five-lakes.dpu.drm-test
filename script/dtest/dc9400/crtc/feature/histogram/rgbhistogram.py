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

def _get_name(id, subname = 'rgbhistogram'):
    return 'crtc%d.feature.rgbhistogram'%id + subname

class rgbhistogram:
    @staticmethod
    def get_case(crtc_id, conn_id, new_histogram = False, subname = 'rgbhistogram'):
        def __get_plane_property():
            clear_color = OrderedDict()
            clear_color["a"] = 128
            clear_color["r"] = 16777215
            clear_color["g"] = 0
            clear_color["b"] = 0

            return OrderedDict([('CLEAR', clear_color)])

        def __get_crtc_property(new_histogram):
            if not new_histogram:
                hist = OrderedDict()
                hist["idx"] = 4
                hist["hist_enable"] = 1
                hist["pos"] = 2
                hist_prot = OrderedDict()
                hist_prot["idx"] = 4
                hist_prot["prot_enable"] = 0

                return OrderedDict([('HIST', hist), ('HIST_PROT', hist_prot)])
            else:
                hist = OrderedDict()
                hist["prot_enable"] = 1
                hist["out_mode"] = 0

                return OrderedDict([('HISTOGRAM_RGB', hist)])

        def __get_golden():
            # Fill golden
            #return OrderedDict([('md5sum', '1234567')])
            return OrderedDict()

        crtc = Crtc(crtc_id, __get_crtc_property(new_histogram))
        plane = Plane(0, crtc, 'AR24', 640, 480, 60, __get_plane_property())
        conn = Connector(0, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
        return DTestUnit(plane, conn, _get_name(crtc_id, subname))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ rgbhistogram.get_case(0, 0), rgbhistogram.get_case(0, 0, True, '.histogram_rgb')]
