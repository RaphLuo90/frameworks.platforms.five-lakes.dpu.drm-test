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
    def __get_property(LumaMode, ProtThreshold, EnableRoi0, EnableRoi1):
        brt = OrderedDict()
        brt["brtTargetGain"] = 864
        brt["lumaMode"] = LumaMode
        brt["protThreshold"] = ProtThreshold

        brtRoi0 = OrderedDict()
        brtRoi1 = OrderedDict()
        brtRoi0["enable"] = EnableRoi0
        brtRoi1["enable"] = EnableRoi1
        if EnableRoi0:
            if EnableRoi1:
                brtRoi0["x"] = 400
                brtRoi0["y"] = 400
                brtRoi0["w"] = 256
                brtRoi0["h"] = 256
            else:
                brtRoi0["x"] = 640
                brtRoi0["y"] = 360
                brtRoi0["w"] = 640
                brtRoi0["h"] = 480
        else:
            brtRoi0["x"] = 0
            brtRoi0["y"] = 0
            brtRoi0["w"] = 0
            brtRoi0["h"] = 0
        if EnableRoi1:
            if EnableRoi0:
                brtRoi1["x"] = 100
                brtRoi1["y"] = 100
                brtRoi1["w"] = 256
                brtRoi1["h"] = 256
            else:
                brtRoi1["x"] = 640
                brtRoi1["y"] = 360
                brtRoi1["w"] = 640
                brtRoi1["h"] = 480
        else:
            brtRoi1["x"] = 0
            brtRoi1["y"] = 0
            brtRoi1["w"] = 0
            brtRoi1["h"] = 0
        brtMask = OrderedDict()
        brtMask["file"] = "brightness_mask_1920x1080.raw"
        brtMask["format"] = "C8"
        brtMask["width"] = 1920
        brtMask["height"] = 1080
        brtMask["tileMode"] = "LINEAR"

        if EnableRoi0:
            brt["ROI0"] = brtRoi0
        if EnableRoi1:
            brt["ROI1"] = brtRoi1
        brt["MASK"] = brtMask

        return OrderedDict([('BRIGHTNESS', brt)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    case_list = []
    lumamode_list = ['MAX', 'WEIGHT']
    protthreshold_list = [512,1024,16382]
    for roi0 in [0,1]:
        for roi1 in [0,1]:
            if not (roi0 or roi1): ###skip roi0 and roi1 both disable.
                continue
            for lma in lumamode_list:
                for pth in protthreshold_list:
                    crtc = Crtc(id, __get_property(lma, pth, roi0, roi1))
                    plane = Plane(0, crtc, 'AR24', 1920, 1080, 60, resource="VV_Background_1920x1080.ARGB8.bmp")
                    conn = Connector(0, crtc, 'AR30', 1920, 1080, 60, golden=__get_golden())
                    case_list.append(DTestUnit(plane, conn, __get_name(id, 'brightness_Roi0{}_Roi1{}_{}_{}'.format(roi0, roi1, lma, pth))))

    return case_list

def __get_case_dec(id):
    def __get_property(LumaMode, ProtThreshold, EnableRoi0, EnableRoi1):
        brt = OrderedDict()
        brt["brtTargetGain"] = 864
        brt["lumaMode"] = LumaMode
        brt["protThreshold"] = ProtThreshold

        brtRoi0 = OrderedDict()
        brtRoi1 = OrderedDict()
        brtRoi0["enable"] = EnableRoi0
        brtRoi1["enable"] = EnableRoi1
        if EnableRoi0:
            if EnableRoi1:
                brtRoi0["x"] = 400
                brtRoi0["y"] = 400
                brtRoi0["w"] = 256
                brtRoi0["h"] = 256
            else:
                brtRoi0["x"] = 640
                brtRoi0["y"] = 360
                brtRoi0["w"] = 640
                brtRoi0["h"] = 480
        else:
            brtRoi0["x"] = 0
            brtRoi0["y"] = 0
            brtRoi0["w"] = 0
            brtRoi0["h"] = 0
        if EnableRoi1:
            if EnableRoi0:
                brtRoi1["x"] = 100
                brtRoi1["y"] = 100
                brtRoi1["w"] = 256
                brtRoi1["h"] = 256
            else:
                brtRoi1["x"] = 640
                brtRoi1["y"] = 360
                brtRoi1["w"] = 640
                brtRoi1["h"] = 480
        else:
            brtRoi1["x"] = 0
            brtRoi1["y"] = 0
            brtRoi1["w"] = 0
            brtRoi1["h"] = 0
        brtMask = OrderedDict()
        brtMask["file"] = "brightness_mask_1920x1080_dec.raw"
        brtMask["format"] = "C8"
        brtMask["width"] = 1920
        brtMask["height"] = 1080
        brtMask["tileMode"] = "TILE8X8"

        if EnableRoi0:
            brt["ROI0"] = brtRoi0
        if EnableRoi1:
            brt["ROI1"] = brtRoi1
        brt["MASK"] = brtMask

        return OrderedDict([('BRIGHTNESS', brt)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    case_list = []
    lumamode_list = ['MAX', 'WEIGHT']
    protthreshold_list = [1024]
    for roi0 in [0,1]:
        for roi1 in [0,1]:
            if roi0 ^ roi1: ### for DEC case, only test one roi to reduce case number.
                for lma in lumamode_list:
                    for pth in protthreshold_list:
                        crtc = Crtc(id, __get_property(lma, pth, roi0, roi1))
                        plane = Plane(0, crtc, 'AR24', 1920, 1080, 60, resource="VV_Background_1920x1080.ARGB8.bmp")
                        conn = Connector(0, crtc, 'AR30', 1920, 1080, 60, golden=__get_golden())
                        case_list.append(DTestUnit(plane, conn, __get_name(id, 'brightness_dec_Roi0{}_Roi1{}_{}_{}'.format(roi0, roi1, lma, pth))))

    return case_list

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return  __get_case(0) + __get_case(1) \
            + __get_case_dec(0) + __get_case_dec(1)
