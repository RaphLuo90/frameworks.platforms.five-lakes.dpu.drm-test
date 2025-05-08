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

############################ Feature.lbox ###############################
from dtest.dc9400.crtc.feature.lbox import *

from dtest.dc9400.crtc.feature.r2y import *
from dtest.dc9400.crtc.feature.ops import *

############################ Feature.blender ############################
from dtest.dc9400.crtc.feature.blender.dither import *
from dtest.dc9400.crtc.feature.blender.eotf import *
from dtest.dc9400.crtc.feature.blender.oetf import *
from dtest.dc9400.crtc.feature.blender.matrix import *

############################ Feature.postpq ############################
from dtest.dc9400.crtc.feature.postpq.data_extend import *
from dtest.dc9400.crtc.feature.postpq.data_trunc import *
from dtest.dc9400.crtc.feature.postpq.scaler import *
from dtest.dc9400.crtc.feature.postpq.ltm import *
from dtest.dc9400.crtc.feature.postpq.gtm import *
from dtest.dc9400.crtc.feature.postpq.ccm import *
from dtest.dc9400.crtc.feature.postpq.color_calibration_path import *
from dtest.dc9400.crtc.feature.postpq.brightness import *
from dtest.dc9400.crtc.feature.postpq.lut3d import *
from dtest.dc9400.crtc.feature.postpq.degamma import *
from dtest.dc9400.crtc.feature.postpq.gamma import *
from dtest.dc9400.crtc.feature.postpq.gamma_dither import *
from dtest.dc9400.crtc.feature.postpq.panel_dither import *
from dtest.dc9400.crtc.feature.postpq.llv_dither import *
from dtest.dc9400.crtc.feature.postpq.sharpness import *
from dtest.dc9400.crtc.feature.postpq.rcd import *
from dtest.dc9400.crtc.feature.postpq.blur import *
from dtest.dc9400.crtc.feature.postpq.vrr import *
from dtest.dc9400.crtc.feature.postpq.bg_color import *
from dtest.dc9400.crtc.feature.postpq.crop import *
from dtest.dc9400.crtc.feature.postpq.free_sync import *
from dtest.dc9400.crtc.feature.postpq.mode_switch import *
from dtest.dc9400.crtc.feature.postpq.dbi import *

############################ Feature.timing ############################
from dtest.dc9400.crtc.feature.timing.wb import *

############################ Feature.format ############################
from dtest.dc9400.crtc.feature.format.yuv import *
from dtest.dc9400.crtc.feature.format.rgb import *

############################ Feature.histogram ############################
from dtest.dc9400.crtc.feature.histogram.histogram import *
from dtest.dc9400.crtc.feature.histogram.rgbhistogram import *

############################ Feature.display_compression ############################
from dtest.dc9400.crtc.feature.display_compression.dsc import *
from dtest.dc9400.crtc.feature.display_compression.vdcm import *
from dtest.dc9400.crtc.feature.display_compression.dsc_splice import *

############################ Feature.ramless ############################
from dtest.dc9400.crtc.feature.ramless.ramless import *
from dtest.dc9400.crtc.feature.ramless.ramless_spliter import *
