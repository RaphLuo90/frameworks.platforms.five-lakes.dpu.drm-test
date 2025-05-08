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

def __get_name(id):
    return 'layer%d.feature.ccm'%id

def __get_property():
        p = OrderedDict()
        p["coef"] = [1684, 364, 0, 68, 1980, 0, 35, 148, 1865]
        p["offset"] = [0, 0, 0]
        return OrderedDict([('LAYER_CCM', p)])

def __get_case(plane_id):
    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'AR24', 640, 480, 60, __get_property())
    conn = Connector(0, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return[ __get_case(i) for i in range(0, 6)]
