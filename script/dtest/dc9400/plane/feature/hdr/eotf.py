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
    p["seg_point"] =[ 2432, 3040, 3104, 3136, 3696 ]
    p["seg_step"] =[ 128, 32, 16, 32, 16, 8 ]
    p["entry"] =[0, 40580, 86498, 152339, 240751, 353530, 492291, 658512, 853559, 1078707, 1335155, 1624036, 1946428, 2303357, 2695808, 3124724, 3591014, 4095556, 4639196, 5222755, 5374976, 5529754, 5687102, 5847030, 6009552, 6174679, 6342423, 6512795, 6685808, 6861472, 7039800, 7220802, 7404491, 7590877, 7779971, 7971785, 8166329, 8363615, 8563654, 8664709, 8766456, 8868897, 8972033, 9180395, 9285623, 9391552, 9498182, 9605516, 9713553, 9822296, 9931746, 10041904, 10152771, 10264350, 10376640, 10489643, 10603362, 10717796, 10832947, 10948817, 11065407, 11182717, 11300750, 11419506, 11538988, 11659195, 11780130, 11901793, 12024186, 12147310, 12271167, 12395757, 12521082, 12647142, 12773940, 12901477, 13029753, 13158771, 13288530, 13353688, 13419033, 13484563, 13550280, 13616183, 13682273, 13748550, 13815013, 13881664, 13948502, 14015527, 14082739, 14150139, 14217728, 14285503, 14353468, 14421620, 14489961, 14558490, 14627208, 14696115, 14765210, 14834495, 14903969, 14973633, 15043486, 15113529, 15183762, 15254185, 15324798, 15395601, 15466595, 15537779, 15609154, 15680720, 15752477, 15824425, 15896565, 15968896, 16041418, 16114132, 16187039, 16260137, 16333428, 16406910, 16480586, 16554454, 16628515, 16702768, 16777215]
    return OrderedDict([('HDR', 1), ('EOTF', p)])

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

def __get_case(plane_id):
    crtc = Crtc(0)
    plane = Plane(plane_id, crtc, 'AR24', 640, 480, 60, __get_property())
    conn = Connector(0, crtc, 'AR30', 640, 480, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(plane_id, 'eotf'))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(i) for i in range(0, chip_info.plane_num) ]
