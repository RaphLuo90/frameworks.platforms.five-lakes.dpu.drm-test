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

def __get_name(con_id, subname):
    return 'connector%d.feature.postwb.'%con_id + subname

def __get_property(prop_name):
    p = OrderedDict()
    p["in_bit"] = 14
    p["out_bit"] = 14
    p["seg_point"] = [ 2, 4, 16, 128, 256, 512, 2048 ]
    p["seg_step"] = [ 1, 2, 4, 8, 16, 8, 128, 256 ]
    p1 = OrderedDict()
    p1["wbPoint"] = 'displayIn'
    return OrderedDict([(prop_name, p), ('WB_POINT', p1)])

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

def __get_case(ctrc_id, name, prop_name, con_id):
    crtc = Crtc(ctrc_id)
    plane = Plane(0, crtc, 'AR24', 640, 480, 60)
    conn = Connector(con_id, crtc, 'AR30', 640, 480, 60, properties=__get_property(prop_name), golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(con_id, name))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    name = 'wb_gamma'
    prop_name = 'WB_GAMMA'
    return [ __get_case(0, name, prop_name, 0), __get_case(1, name, prop_name, 1) ]

