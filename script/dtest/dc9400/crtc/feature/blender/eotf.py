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
    return 'crtc%d.feature.blender.eotf'%id

def __get_property():
    p = OrderedDict()
    p["seg_point"] = [524288, 786432, 13369344, 13631488, 15335424, 15466496, 15728640, 15859712]
    p["seg_step"] = [524288, 262144, 131072, 65536, 131072, 65536, 131072, 65536, 131072]
    p["entry"] = [0, 40580, 61415, 73332, 86498, 100949,
                  116718, 133837, 152339, 172252, 193606, 216430,
                  240751, 266595, 293990, 322960, 353530, 385724,
                  419568, 455082, 492291, 531217, 571882, 614306,
                  658512, 704520, 752351, 802024, 853559, 906976,
                  962294, 1019531, 1078707, 1139839, 1202946, 1268045,
                  1335155, 1404291, 1475472, 1548715, 1624036, 1701451,
                  1780978, 1862631, 1946427, 2032383, 2120513, 2210832,
                  2303357, 2398102, 2495082, 2594313, 2695808, 2799582,
                  2905650, 3014026, 3124724, 3237758, 3353142, 3470889,
                  3591014, 3713530, 3838450, 3965787, 4095555, 4227767,
                  4362436, 4499574, 4639195, 4781311, 4925935, 5073079,
                  5222755, 5374976, 5529754, 5687102, 5847030, 6009552,
                  6174679, 6342423, 6512795, 6685808, 6861472, 7039800,
                  7220802, 7404491, 7590876, 7779971, 7971784, 8166329,
                  8363615, 8563654, 8766456, 8972033, 9180394, 9391552,
                  9605515, 9822296, 10041904, 10152771, 10264349, 10376640,
                  10489643, 10717796, 10948817, 11182717, 11419506, 11659195,
                  11901793, 12147310, 12395757, 12647142, 12901477, 13158771,
                  13419033, 13682273, 13815013, 13948501, 14217727, 14489960,
                  14627208, 14765210, 15043486, 15324797, 15609154, 15896564,
                  16187039, 16480586, 16777215]
    return OrderedDict([('BLD_EOTF', p)])

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

def __get_case(crtc_id):
    crtc = Crtc(crtc_id, __get_property())
    plane = Plane(0, crtc, 'AR24', 1280, 720, 60, resource = "VIV_Background_1280_720.bmp")
    conn = Connector(0, crtc, 'AR30', 1280, 720, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(crtc_id))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [__get_case(2), __get_case(3)]
