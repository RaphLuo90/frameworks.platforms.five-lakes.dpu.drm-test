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
    return 'crtc%d.feature.postpq.gtm.'%id + subname

def __get_case_normal_1(crtc_id:int):
    def __get_property():
        p_gtm = OrderedDict()
        p_gtm["Mode"] = 1
        p_gtm["VidMode"] = 1
        p_gtm["Strength"] = 4
        p_gtm["width"] = 1920
        p_gtm["height"] = 1280

        return OrderedDict([('GTM', p_gtm)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc = Crtc(crtc_id, __get_property())

    plane = Plane(0, crtc, 'AR24', 1920, 1280, 60, resource = "imgseq2_f_0.bmp")

    conn = Connector(0, crtc, 'AR30', 1920, 1280, 60, golden=__get_golden())

    name_string = "{}_mode_{}_strength_{}_gd_{}_normal_1".format('GTM', 1, 4, 12)
    return DTestUnit(plane, conn, __get_name(1, name_string))

def __get_case_normal_2(crtc_id:int):
    def __get_property():
        p_gtm = OrderedDict()
        p_gtm["Mode"] = 1
        p_gtm["VidMode"] = 1
        p_gtm["Strength"] = 4
        p_gtm["width"] = 1920
        p_gtm["height"] = 1280

        return OrderedDict([('GTM', p_gtm)])

    def __get_golden():
        # Fill golden
        #return OrderedDict([('md5sum', '1234567')])
        return OrderedDict()

    crtc = Crtc(crtc_id, __get_property())

    plane = Plane(0, crtc, 'AR24', 1920, 1280, 60, resource = "imgseq2_f_3.bmp")
    conn = Connector(0, crtc, 'AR30', 1920, 1280, 60, golden=__get_golden())

    name_string = "{}_mode_{}_strength_{}_gd_{}_normal_2".format('GTM', 1, 4, 12)
    return DTestUnit(plane, conn, __get_name(1, name_string))


@dtest_unit_group('dc9400')
def __gen(chip_info):
    return [ __get_case_normal_1(1) ] + \
           [ __get_case_normal_2(1) ]