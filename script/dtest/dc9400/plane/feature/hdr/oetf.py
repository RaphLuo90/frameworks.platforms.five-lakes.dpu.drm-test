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
    p["seg_point"] = [ 32768, 49152, 65536, 163840, 393216, 917504, 2097152, 5242880, 12582912 ]
    p["seg_step"] = [ 32768, 16384, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576 ]
    p["entry"] = [0, 423363, 635044, 738276, 833312, 1004415, 1156521, 1294453, 1421310, 1539217, 1649706, 1852742, 2036860, 2206194, 2363581, 2511065, 2650175, 2782087, 3027854, 3253888, 3463975, 3660842, 3846531, 4022613, 4190327, 4350673, 4652392, 4932824, 5195611, 5443475, 5678516, 5902388, 6116424, 6321716, 6519170, 6893502, 7244280, 7575140, 7888882, 8187715, 8473419, 8747451, 9011021, 9265147, 9510694, 9748406, 9978927, 10420572, 10839366, 11238261, 11619630, 11985419, 12337242, 12676459, 13004226, 13321533, 13629241, 13928100, 14218767, 14501825, 14777791, 15310251, 15819318, 16307587, 16777215]
    return OrderedDict([('HDR', 1), ('OETF', p)])

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

def __get_case(plane_id):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'AR24', 640, 480, 60, __get_property())
    conn = Connector(0, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'oetf'))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(i) for i in range(0, chip_info.plane_num) ]
