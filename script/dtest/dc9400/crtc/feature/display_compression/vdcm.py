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
    return 'crtc%d.feature.display_compression.'%id + subname

def __get_case(id):
    def __get_property():
        p = OrderedDict()
        p["slices_per_line"] = 2
        p["slice_height"] = 32
        p["ss_num"] = 2
        p["split_panel_enable"] = False
        p["multiplex_mode_enable"] = False
        p["multiplex_out_sel"] = 0
        p["multiplex_eoc_enable"] = False
        p["hs_split_input_enable"] = True
        p["video_mode"] = True
        return OrderedDict([('VDCM', p)])

    def __get_conn_property():
        p1 = OrderedDict()
        p1["wbPoint"] = 'ofifoOut'
        return OrderedDict([('WB_POINT', p1)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc = Crtc(id, __get_property())
    plane = Plane(0, crtc, 'XR24', 640, 480, 60)
    conn = Connector(0, crtc, 'AR30', 640, 480, 60, __get_conn_property(), golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(id, 'vdcm'))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(0)]
