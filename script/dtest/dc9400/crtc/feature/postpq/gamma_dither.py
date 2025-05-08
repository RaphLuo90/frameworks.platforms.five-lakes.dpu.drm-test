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
    return 'crtc%d.feature.postpq.'%id + subname

def __get_case(id, conn_id):
    def __get_property():
        p = OrderedDict()
        p["index_type"] = 0
        p["sw_index"] = 0
        p["table_low"] = [2562581776, 986306598, 1894013137]
        p["table_high"] = [1674212594, 1098604272, 2946220643]
        return OrderedDict([('GAMMA_DITHER', p)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc = Crtc(id, __get_property())
    plane = Plane(0, crtc, 'AR24', 640, 480, 60)
    conn = Connector(conn_id, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    if id == 1 and conn_id == 1:
        return DTestUnit(plane, conn, __get_name(id, 'gamma_dither_connector1'))
    else:
        return DTestUnit(plane, conn, __get_name(id, 'gamma_dither'))
@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(0,0), __get_case(1, 0), __get_case(1, 1)]
