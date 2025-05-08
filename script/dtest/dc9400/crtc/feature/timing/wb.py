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
    return 'crtc%d.feature.timing.'%id + subname

def __get_property():
    return OrderedDict()

def __get_golden():
    return OrderedDict()

def __get_case(plane_id: int, format : str, refresh : int, resource="defaultPattern", width=640, height=480, crtc_id=4, connector_id=2, connector_format='AR30'):
    crtc = Crtc(crtc_id)
    plane = Plane(plane_id, crtc, format, width, height, refresh, __get_property(), resource)
    conn = Connector(connector_id, crtc, connector_format, width, height, refresh, golden=__get_golden())
    return DTestUnit(plane, conn, __get_name(crtc_id, '{}x{}@{}'.format(width, height, refresh)))

@dtest_unit_group('dc9400')
def __gen(chip_info):
    return  [__get_case(0, 'AR24', 60, "defaultPattern", 320, 480, 4, 2, connector_format='AR24')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 720, 480, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 720, 1612, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 800, 600, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 1080, 2400, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 1024, 768, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 1280, 1024, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 1400, 1050, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 1440, 3520, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 1440, 3216, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 1440, 3360, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 1680, 1050, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 1600, 1200, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 1920, 1200, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 2340, 3404, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 2500, 2820, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 2700, 2600, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 3200, 1920, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 3840, 2160, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 4096, 2160, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 60, "defaultPattern", 1080, 2340, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 30, "defaultPattern", 7680, 4320, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 120, "defaultPattern", 1280, 720, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 120, "defaultPattern", 1920, 1080, 4, 2, connector_format='AR30')] \
            +[__get_case(0, 'AR24', 120, "defaultPattern", 3840, 2160, 4, 2, connector_format='AR30')]