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
    return 'crtc%d.feature.blender.matrix_'%id + subname

def __get_nonlinear_gamut_name(id, subname):
    return 'crtc%d.feature.blender.nonlinear_matrix_'%id + subname

def __get_property(gamut):
    p = OrderedDict()
    p["mode"] = gamut
    return OrderedDict([('BLD_GAMUT_MAP', p)])

def __get_nonlinear_gamut_property(gamut):
    p = OrderedDict()
    p["mode"] = gamut
    return OrderedDict([('BLD_NONLINEAR_GAMUT_MAP', p)])

def __get_golden():
    # Fill golden
    #return OrderedDict([('md5sum', '1234567')])
    return OrderedDict()

def __get_case(crtc_id, gamut):
    crtc = Crtc(crtc_id, __get_property(gamut))
    plane = Plane(0, crtc, 'AR24', 1280, 720, 60, resource = "VIV_Background_1280_720.bmp")
    conn = Connector(0, crtc, 'AR30', 1280, 720, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(crtc_id, gamut))

def __get_nonlinear_gamut_case(crtc_id, gamut):
    crtc = Crtc(crtc_id, __get_nonlinear_gamut_property(gamut))
    plane = Plane(0, crtc, 'AR24', 1280, 720, 60, resource = "VIV_Background_1280_720.bmp")
    conn = Connector(0, crtc, 'AR30', 1280, 720, 60, golden=__get_golden())
    return DTestUnit(plane, conn, __get_nonlinear_gamut_name(crtc_id, gamut))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    gamut_mode_list = ["709TO2020", "2020TO709", "2020TOP3", "P3TO2020", "P3TOSRGB", "SRGBTOP3"]
    return [ __get_case(2, mode) for mode in gamut_mode_list ] \
         + [ __get_case(3, mode) for mode in gamut_mode_list ] \
         + [ __get_nonlinear_gamut_case(2, mode) for mode in gamut_mode_list ]