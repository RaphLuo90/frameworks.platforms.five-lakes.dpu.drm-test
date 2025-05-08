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
    def __get_property(index):
        rcd = OrderedDict()

        rcdBg = OrderedDict()
        rcdBg["rcdBgPnlColor"] = 1073741823
        if index == 4:
            rcdBg["rcdBgRoiEnable"] = 1
            rcdBg["rcdBgRoiColor"] = 1072693248
            rcdBg["rcdBgRoiX"] = 0
            rcdBg["rcdBgRoiY"] = 0
            rcdBg["rcdBgRoiWidth"] = 1280
            rcdBg["rcdBgRoiHeight"] = 720

        rcdRoi = OrderedDict()
        if index ==2:
            rcdRoi["rcdBtmRoiEnable"] = 1
        else:
             rcdRoi["rcdTopRoiEnable"] = 1
        if index in [3,4]:
            rcdRoi["rcdBtmRoiEnable"] = 1
        if index == 2:
            rcdRoi["rcdBtmRoiX"] = 0
            rcdRoi["rcdBtmRoiY"] = 464
            rcdRoi["rcdBtmRoiWidth"] = 1280
            rcdRoi["rcdBtmRoiHeight"] = 256
        else:
            rcdRoi["rcdTopRoiX"] = 0
            rcdRoi["rcdTopRoiY"] = 0
            rcdRoi["rcdTopRoiWidth"] = 1280
            rcdRoi["rcdTopRoiHeight"] = 256

        if index in [3,4]:
            rcdRoi["rcdBtmRoiX"] = 0
            rcdRoi["rcdBtmRoiY"] = 464
            rcdRoi["rcdBtmRoiWidth"] = 1280
            rcdRoi["rcdBtmRoiHeight"] = 256

        rcdMask = OrderedDict()
        if index == 5:
            rcdMask["file"] = "arc_dec.raw"
            rcdMask["tileMode"] = "TILE8X8"
        else:
            rcdMask["file"] = "arc.raw"
            rcdMask["tileMode"] = "LINEAR"

        rcdMask["format"] = "C8"
        rcdMask["width"] = 1280
        rcdMask["height"] = 720

        rcd["BACKGROUND"] = rcdBg
        rcd["ROI"] = rcdRoi
        rcd["MASK"] = rcdMask

        return OrderedDict([('RCD', rcd)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    index =[1,2,3,4,5] ## to write 5 type cases, 1~4 is normal, 5 is DEC
    dec_cnt = 1
    case_list = []
    for i in index:
        crtc = Crtc(id, __get_property(i))
        plane = Plane(0, crtc, 'AR24', 1280, 720, 60, resource="VIV_Background_1280_720.bmp")
        conn = Connector(0, crtc, 'AR30', 1280, 720, 60, golden=__get_golden())
        case_name = __get_name(id, 'rcd_cfg{}'.format(i))
        if i == 5:
            case_name = __get_name(id, 'rcd_dec_cfg{}'.format(dec_cnt))
            dec_cnt+=1

        case_list.append(DTestUnit(plane, conn, case_name))

    return case_list

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return __get_case(0) + __get_case(1)
