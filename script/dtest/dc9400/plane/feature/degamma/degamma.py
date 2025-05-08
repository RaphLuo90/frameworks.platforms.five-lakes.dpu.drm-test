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
    return 'plane%d.feature.'%id + subname

def __get_case(id):
    def __get_property():
        p = OrderedDict()
        p["seg_point"] = [ 2048, 2560, 3072, 3712 ]
        p["seg_step"] = [ 128, 64, 32, 16, 8 ]
        p["entry"] = [ 0, 0, 0, 0, 1, 2, 3, 5, 8, 13, 19, 28, 40, 57, 80, 110, 151, 177, 206, 240, 280, 325, 377, 437, 507, 546, 587, 631, 679, 730, 785, 844, 908, 976, 1049, 1127, 1211, 1301, 1398, 1502, 1614, 1673, 1734, 1797, 1862, 1930, 2001, 2074, 2149, 2228, 2309, 2393, 2480, 2571, 2665, 2762, 2862, 2967, 3075, 3187, 3304, 3425, 3550, 3679, 3814, 3954, 4098, 4248, 4404, 4566, 4733, 4907, 5087, 5274, 5468, 5669, 5878, 6095, 6319, 6553, 6795, 6919, 7046, 7175, 7307, 7441, 7578, 7717, 7859, 8003, 8150, 8300, 8453, 8609, 8768, 8929, 9094, 9262, 9433, 9607, 9785, 9966, 10151, 10339, 10530, 10726, 10925, 11128, 11334, 11545, 11760, 11979, 12202, 12429, 12661, 12897, 13138, 13384, 13634, 13889, 14149, 14415, 14685, 14961, 15241, 15528, 15820, 16118, 16383 ]
        return OrderedDict([('LAYER_DEGAMMA', p)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc = Crtc(0)
    plane = Plane(id, crtc, 'AR24', 640, 480, 60, __get_property())
    conn = Connector(0, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(id, 'degamma'))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(i) for i in range(0, chip_info.plane_num) ]
