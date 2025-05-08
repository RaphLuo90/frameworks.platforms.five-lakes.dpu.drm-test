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
    def __get_property(cfg_dict):
        blur = OrderedDict()
        blurRoi = OrderedDict()
        blurRoi["x"] = cfg_dict['roix']
        blurRoi["y"] = cfg_dict['roiy']
        blurRoi["w"] = cfg_dict['roiwidth']
        blurRoi["h"] = cfg_dict['roiheight']
        blurMask = OrderedDict()
        blurMask["file"] = "blur_mask_0xFF_1280x720.raw"
        blurMask["format"] = "C8"
        blurMask["width"] = 1280
        blurMask["height"] = 720
        blurMask["tileMode"] = "LINEAR"

        blur["ROI"] = blurRoi
        blur["MASK"] = blurMask
        return OrderedDict([('BLUR', blur)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    case_list = []
    cfg_list = [(0,0,1280,720),
                (0,0,600,600),
                (0,120,600,600),
                (1160,0,100,100),
                (1160,600,100,100),
                (20,0,100,100),
                (20,600,100,100),
                (340,60,600,600),
                (590,310,100,100),
                (680,0,600,600),
                (680,120,600,600)]

    for c in cfg_list:
        cfg_dict = dict()
        cfg_dict['roix'] = c[0]
        cfg_dict['roiy'] = c[1]
        cfg_dict['roiwidth'] = c[2]
        cfg_dict['roiheight'] = c[3]
        crtc = Crtc(id, __get_property(cfg_dict))
        plane = Plane(0, crtc, 'AR24', 1280, 720, 60, resource="ARGB8888_linear_1280x720_bear.bmp")
        conn = Connector(0, crtc, 'AR30', 1280, 720, 60, golden=__get_golden())
        case_list.append(DTestUnit(plane, conn, __get_name(id, 'blur_Rx{}_Ry{}_Rw{}_Rh{}'.format(
            cfg_dict['roix'],cfg_dict['roiy'],cfg_dict['roiwidth'],cfg_dict['roiheight'] ))))

    return case_list


def __get_case_dec(id):
    def __get_property(cfg_dict):
        blur = OrderedDict()
        blurRoi = OrderedDict()
        blurRoi["x"] = cfg_dict['roix']
        blurRoi["y"] = cfg_dict['roiy']
        blurRoi["w"] = cfg_dict['roiwidth']
        blurRoi["h"] = cfg_dict['roiheight']
        blurMask = OrderedDict()
        blurMask["file"] = "blur_mask_0xFF_1280x720_dec.raw"
        blurMask["format"] = "C8"
        blurMask["width"] = 1280
        blurMask["height"] = 720
        blurMask["tileMode"] = "TILE8X8"

        blur["ROI"] = blurRoi
        blur["MASK"] = blurMask
        return OrderedDict([('BLUR', blur)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    case_list = []
    cfg_list = [(1160,0,100,100),
                (20,0,640,480)]

    for c in cfg_list:
        cfg_dict = dict()
        cfg_dict['roix'] = c[0]
        cfg_dict['roiy'] = c[1]
        cfg_dict['roiwidth'] = c[2]
        cfg_dict['roiheight'] = c[3]
        crtc = Crtc(id, __get_property(cfg_dict))
        plane = Plane(0, crtc, 'AR24', 1280, 720, 60, resource="ARGB8888_linear_1280x720_bear.bmp")
        conn = Connector(0, crtc, 'AR30', 1280, 720, 60, golden=__get_golden())
        case_list.append(DTestUnit(plane, conn, __get_name(id, 'blur_dec_Rx{}_Ry{}_Rw{}_Rh{}'.format(
            cfg_dict['roix'],cfg_dict['roiy'],cfg_dict['roiwidth'],cfg_dict['roiheight'] ))))

    return case_list

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return __get_case(0) + __get_case_dec(0)
