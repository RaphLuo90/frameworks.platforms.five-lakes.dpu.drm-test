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

def __get_name(subname, conn_width, conn_height):
    return 'multiFrame.{}_{}x{}'.format(subname,conn_width,conn_height)

def _get_roi_property(roiX, roiY, roiWidth,roiHeight,globalDisable):
    p = OrderedDict()
    if globalDisable:
        p["globalDisable"] = 1
    else:
        p["dmaMode"] ='oneROI'
        p["roiX"] = roiX
        p["roiY"] = roiY
        p["roiWidth"] = roiWidth
        p["roiHeight"] = roiHeight
        p["layerOutX"] = 0
        p["layerOutY"] = 0
        p["layerOutWidth"] = roiWidth
        p["layerOutHeight"] = roiHeight
    return OrderedDict([('DMA_CONFIG', p)])

def __get_property_wb_info(wb_width, wb_height, tilemode, stripe):
    p = OrderedDict()
    if wb_width and wb_height:
        p['wb_width'] = wb_width
        p['wb_height'] = wb_height
    p['decType'] = "PVRIC"
    p['tileMode'] = "TILE16X4"
    p['lossy'] = 0
    if stripe:
        p['WBWithStripe'] = stripe

    return OrderedDict(p)

def __get_pos_property(posX, posY, globalDisable):
    p = OrderedDict()
    if globalDisable:
        p["globalDisable"] = 1
    else:
        p["X"] = posX
        p["Y"] = posY

    return OrderedDict([('WB_STRIPE_POS', p)])

def __get_case(crtc_id, subname,conn_width,conn_height):

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc0 = Crtc(crtc_id)
    crtc1 = Crtc(crtc_id)

    plane0 = Plane(0, crtc0, 'AR30', 2048, 1080, 60, _get_roi_property(0,0,512,1080,0), crtcX = 0, crtcY = 0, crtcWidth = 2048, crtcHeight = 1080, resource = "ARGB2101010_LINEAR_2048x1080.bmp")
    plane1 = Plane(0, crtc0, 'AR30', 2048, 1080, 60, _get_roi_property(512,0,512,1080,0), crtcX = 0, crtcY = 0, crtcWidth = 2048, crtcHeight = 1080, resource = "ARGB2101010_LINEAR_2048x1080.bmp")
    plane2 = Plane(0, crtc0, 'AR30', 2048, 1080, 60, _get_roi_property(1024,0,512,1080,0), crtcX = 0, crtcY = 0, crtcWidth = 2048, crtcHeight = 1080, resource = "ARGB2101010_LINEAR_2048x1080.bmp")
    plane3 = Plane(0, crtc0, 'AR30', 2048, 1080, 60, _get_roi_property(1536,0,512,1080,0), crtcX = 0, crtcY = 0, crtcWidth = 2048, crtcHeight = 1080, resource = "ARGB2101010_LINEAR_2048x1080.bmp")
    plane4 = Plane(0, crtc0, 'AR30', 2048, 1080, 60, _get_roi_property(0,0,512,1080,1), crtcX = 0, crtcY = 0, crtcWidth = 2048, crtcHeight = 1080)

    plane5 = Plane(0, crtc1, 'AR30', 4096, 2160, 60, _get_roi_property(0,0,2048,1080,0), crtcX = 0, crtcY = 0, crtcWidth = 4096, crtcHeight = 2160, resource = "ARGB2101010_LINEAR_4096x2160.bmp")
    plane6 = Plane(0, crtc1, 'AR30', 4096, 2160, 60, _get_roi_property(2048,0,2048,1080,0), crtcX = 0, crtcY = 0, crtcWidth = 4096, crtcHeight = 2160, resource = "ARGB2101010_LINEAR_4096x2160.bmp")
    plane7 = Plane(0, crtc1, 'AR30', 4096, 2160, 60, _get_roi_property(0,1080,2048,1080,0), crtcX = 0, crtcY = 0, crtcWidth = 4096, crtcHeight = 2160, resource = "ARGB2101010_LINEAR_4096x2160.bmp")
    plane8 = Plane(0, crtc1, 'AR30', 4096, 2160, 60, _get_roi_property(2048,1080,2048,1080,0), crtcX = 0, crtcY = 0, crtcWidth = 4096, crtcHeight = 2160, resource = "ARGB2101010_LINEAR_4096x2160.bmp")
    plane9 = Plane(0, crtc1, 'AR30', 4096, 2160, 60, _get_roi_property(0,0,4096,2160,1), crtcX = 0, crtcY = 0, crtcWidth = 4096, crtcHeight = 2160)

    conn0 = Connector(2, crtc0, 'AR30', 512, 1080, 60, __get_pos_property(0,0,0), other_prop=__get_property_wb_info(2048,1080,"TILE16X4",1),golden=__get_golden())
    conn1 = Connector(2, crtc0, 'AR30', 512, 1080, 60, __get_pos_property(512,0,0), other_prop=__get_property_wb_info(2048,1080,"TILE16X4",1),golden=__get_golden())
    conn2 = Connector(2, crtc0, 'AR30', 512, 1080, 60, __get_pos_property(1024,0,0), other_prop=__get_property_wb_info(2048,1080,"TILE16X4",1),golden=__get_golden())
    conn3 = Connector(2, crtc0, 'AR30', 512, 1080, 60, __get_pos_property(1536,0,0), other_prop=__get_property_wb_info(2048,1080,"TILE16X4",1),golden=__get_golden())
    conn4 = Connector(2, crtc0, 'AR30', 2048, 1080, 60, __get_pos_property(0,0,1), other_prop=__get_property_wb_info(0,0,"TILE16X4",0),golden=__get_golden())

    conn5 = Connector(2, crtc1, 'AR30', 2048, 1080, 60, __get_pos_property(0,0,0), other_prop=__get_property_wb_info(4096,2160,"TILE16X4",1),golden=__get_golden())
    conn6 = Connector(2, crtc1, 'AR30', 2048, 1080, 60, __get_pos_property(2048,0,0), other_prop=__get_property_wb_info(4096,2160,"TILE16X4",1),golden=__get_golden())
    conn7 = Connector(2, crtc1, 'AR30', 2048, 1080, 60, __get_pos_property(0,1080,0), other_prop=__get_property_wb_info(4096,2160,"TILE16X4",1),golden=__get_golden())
    conn8 = Connector(2, crtc1, 'AR30', 2048, 1080, 60, __get_pos_property(2048,1080,0), other_prop=__get_property_wb_info(4096,2160,"TILE16X4",1),golden=__get_golden())
    conn9 = Connector(2, crtc1, 'AR30', 4096, 2160, 60, __get_pos_property(0,0,1), other_prop=__get_property_wb_info(0,0,"TILE16X4",0),golden=__get_golden())

    frame0 = DTestUnit([plane0], [conn0], __get_name(subname,conn_width,conn_height), frameID = 0)
    frame1 = DTestUnit([plane1], [conn1], __get_name(subname,conn_width,conn_height), frameID = 1)
    frame2 = DTestUnit([plane2], [conn2], __get_name(subname,conn_width,conn_height), frameID = 2)
    frame3 = DTestUnit([plane3], [conn3], __get_name(subname,conn_width,conn_height), frameID = 3)
    frame4 = DTestUnit([plane4], [conn4], __get_name(subname,conn_width,conn_height), frameID = 4)

    frame5 = DTestUnit([plane5], [conn5], __get_name(subname,conn_width,conn_height), frameID = 0)
    frame6 = DTestUnit([plane6], [conn6], __get_name(subname,conn_width,conn_height), frameID = 1)
    frame7 = DTestUnit([plane7], [conn7], __get_name(subname,conn_width,conn_height), frameID = 2)
    frame8 = DTestUnit([plane8], [conn8], __get_name(subname,conn_width,conn_height), frameID = 3)
    frame9 = DTestUnit([plane9], [conn9], __get_name(subname,conn_width,conn_height), frameID = 4)
    if conn_width == 4096 and conn_height == 2160:
        frames = [frame5, frame6, frame7, frame8, frame9]
    elif conn_width == 2048 and conn_height == 1080:
       frames = [frame0, frame1, frame2, frame3, frame4]
    return DTestFrameUnit(frames, __get_name(subname, conn_width, conn_height))
@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case(4, 'stripe', 2048,1080), __get_case(4, 'stripe', 4096,2160)]
