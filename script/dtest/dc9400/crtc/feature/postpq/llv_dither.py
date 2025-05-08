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

def __get_case(id):
    def __get_property():
        p = OrderedDict()
        p["index_type"] = 1
        p["sw_index"] = 0
        p["threshold"] = 200
        p["linear_threshold"] = 800
        p["start_x"] = 0
        p["start_y"] = 0
        p["mask"] = 0
        seed = OrderedDict()
        seed["hash_seed_x_enable"] = 1
        seed["hash_seed_y_enable"] = 1
        seed["permut_seed1_enable"] = 1
        seed["permut_seed2_enable"] = 1
        seed["hash_seed_x"] = [45031, 47417, 37483, 43973, 37517, 45319, 41203, 49261],
        seed["hash_seed_y"] = [14463, 56533, 24659, 16843, 20789, 22447, 18371, 32503],
        seed["permut_seed1"] = [30340, 60458, 19992, 32054, 45828, 55594, 31668, 17290],
        seed["permut_seed2"] = [27779, 27179, 30013, 29179, 29363, 30529, 27253, 23599]
        p["seed"] = seed
        return OrderedDict([('LLV_DITHER', p)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc = Crtc(id, __get_property())
    plane = Plane(0, crtc, 'AR24', 640, 480, 60)
    conn = Connector(0, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(id, 'llv_dither'))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(0), __get_case(1) ]
