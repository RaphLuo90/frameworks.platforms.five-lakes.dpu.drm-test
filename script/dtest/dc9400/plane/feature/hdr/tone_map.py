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
    return 'plane%d.feature.hdr.'%id + subname

def __get_property():
    p = OrderedDict()
    pseudo_y = OrderedDict()
    pseudo_y["mode"] = "LINEAR"
    pseudo_y["coef0"] = 1225
    pseudo_y["coef1"] = 2404
    pseudo_y["coef2"] = 467
    pseudo_y["weight"] = 0
    p["pseudo_y"] = pseudo_y
    p["seg_point"] = [ 16384,   32768,   49152,   65536,   81920,   98304,   131072,  196608,  262144,  524288, 786432,  851968,  884736,  917504,  1048576, 1179648, 1310720, 1441792, 1572864, 1835008, 2097152, 2359296, 2621440, 3145728, 3670016,              4194304, 5242880, 6291456, 8388608, 10485760, 12582912, 16777216 ]
    p["entry"] = [ 1540934, 1540934, 1225479, 1115156, 1059995, 1026898, 1004833, 977253, 949672, 935881, 915196, 908301, 907240, 906769, 899367, 795973, 715759, 651742, 599479, 556006, 487815, 436695, 396827, 364718, 315634, 278967, 249558, 204737, 173376, 131070, 104856, 87380, 0 ]
    return OrderedDict([('HDR', 1), ('TONE_MAP', p)])

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

def __get_case(plane_id):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'AR24', 640, 480, 60, __get_property())
    conn = Connector(0, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'tone_map'))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(i) for i in range(0, chip_info.plane_num) ]
