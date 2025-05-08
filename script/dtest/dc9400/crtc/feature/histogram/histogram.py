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
    return 'crtc%d.feature.histogram'%id + subname

class histogram:
    @staticmethod
    def get_case(crtc_id, conn_id, new_histogram, subname = 'histogram'):
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
                hist["idx"] = 0
                hist["hist_enable"] = 1
                hist["pos"] = 0
                hist["bin_mode"] = 0
                hist["coef"] = [0, 0, 0]
                hist_roi = OrderedDict()
                hist_roi["idx"] = 0
                hist_roi["x"] = 0
                hist_roi["y"] = 0
                hist_roi["w"] = 640
                hist_roi["h"] = 480
                hist_block = OrderedDict()
                hist_block["idx"] = 0
                hist_block["x"] = 0
                hist_block["y"] = 0
                hist_block["w"] = 0
                hist_block["h"] = 0
                hist_prot = OrderedDict()
                hist_prot["idx"] = 0
                hist_prot["prot_enable"] = 0

                return OrderedDict([('HIST', hist), ('HIST_ROI', hist_roi), ('HIST_BLOCK', hist_block), ('HIST_PROT', hist_prot)])
            else:
                histogram_0 = OrderedDict()
                histogram_0["pos"] = 0
                histogram_0["bin_mode"] = 1
                histogram_0["coef"] = [0, 1, 1023]
                histogram_0["roi_x"] = 0
                histogram_0["roi_y"] = 0
                histogram_0["roi_w"] = 640
                histogram_0["roi_h"] = 480
                histogram_0["block_x"] = 0
                histogram_0["block_y"] = 0
                histogram_0["block_w"] = 0
                histogram_0["block_h"] = 0
                histogram_0["prot_enable"] = 1
                histogram_0["out_mode"] = 0
                histogram_1 = OrderedDict()
                histogram_1["pos"] = 0
                histogram_1["bin_mode"] = 1
                histogram_1["coef"] = [256, 256, 512]
                histogram_1["roi_x"] = 0
                histogram_1["roi_y"] = 0
                histogram_1["roi_w"] = 640
                histogram_1["roi_h"] = 480
                histogram_1["block_x"] = 0
                histogram_1["block_y"] = 0
                histogram_1["block_w"] = 0
                histogram_1["block_h"] = 0
                histogram_1["prot_enable"] = 1
                histogram_1["out_mode"] = 0
                histogram_2 = OrderedDict()
                histogram_2["pos"] = 0
                histogram_2["bin_mode"] = 1
                histogram_2["coef"] = [512, 256, 256]
                histogram_2["roi_x"] = 0
                histogram_2["roi_y"] = 0
                histogram_2["roi_w"] = 640
                histogram_2["roi_h"] = 480
                histogram_2["block_x"] = 0
                histogram_2["block_y"] = 0
                histogram_2["block_w"] = 0
                histogram_2["block_h"] = 0
                histogram_2["prot_enable"] = 1
                histogram_2["out_mode"] = 0
                histogram_3 = OrderedDict()
                histogram_3["pos"] = 0
                histogram_3["bin_mode"] = 1
                histogram_3["coef"] = [1023, 1, 0]
                histogram_3["roi_x"] = 0
                histogram_3["roi_y"] = 0
                histogram_3["roi_w"] = 640
                histogram_3["roi_h"] = 480
                histogram_3["block_x"] = 0
                histogram_3["block_y"] = 0
                histogram_3["block_w"] = 0
                histogram_3["block_h"] = 0
                histogram_3["prot_enable"] = 1
                histogram_3["out_mode"] = 0

                return OrderedDict([('HISTOGRAM_0', histogram_0), ('HISTOGRAM_1', histogram_1), ('HISTOGRAM_2', histogram_2), ('HISTOGRAM_3', histogram_3)])

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
    return [ histogram.get_case(0, 0, False), histogram.get_case(1, 0, False), histogram.get_case(0, 0, True, '.4histogram'), histogram.get_case(1, 0, True, '.4histogram')]
