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

############################ Feature.multiCrtc ############################
from dtest.dc9400.scenario.scenario1_1x1024x768_60hz_2surfaces_no_rotate import *
from dtest.dc9400.scenario.scenario2_1x1920x1080_120hz_2surfaces_no_rotate import *
from dtest.dc9400.scenario.scenario3_2x2560x1080_120hz_4surfaces_rotate180 import *
from dtest.dc9400.scenario.scenario4_1x3440x1440_160hz_2surfaces_no_rotate import *
from dtest.dc9400.scenario.scenario5_4x3840x2160_120hz_8surfaces_rotate90_crtc01 import *
from dtest.dc9400.scenario.scenario5_4x3840x2160_120hz_8surfaces_rotate90_crtc23 import *
from dtest.dc9400.scenario.scenario6_1x4096x2160_120hz_2surfaces_flipx import *
from dtest.dc9400.scenario.scenario7_1x5120x2880_120hz_2surfaces_flipy import *
from dtest.dc9400.scenario.scenario8_1x7680x4320_30hz_4surfaces_no_rotate_crtc01 import *
from dtest.dc9400.scenario.scenario8_1x7680x4320_30hz_4surfaces_no_rotate_crtc23 import *
