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

############################ Feature.hdr ############################
from dtest.dc9400.plane.feature.hdr.demultiply import *
from dtest.dc9400.plane.feature.hdr.eotf import *
from dtest.dc9400.plane.feature.hdr.gamut_map import *
from dtest.dc9400.plane.feature.hdr.tone_map import *
from dtest.dc9400.plane.feature.hdr.oetf import *
############################ Feature.clear ############################
from dtest.dc9400.plane.feature.clear.red import *
from dtest.dc9400.plane.feature.clear.green import *
from dtest.dc9400.plane.feature.clear.blue import *
############################ Feature.dma ############################
from dtest.dc9400.plane.feature.dma.oneROI import *
from dtest.dc9400.plane.feature.dma.twoROI import *
from dtest.dc9400.plane.feature.dma.extendLayer import *
from dtest.dc9400.plane.feature.dma.extendLayerEx import *
############################ Feature.scale ############################
#from dtest.dc9400.plane.feature.scaler.scalerUp import *
from dtest.dc9400.plane.feature.scaler.baseScaler import *
from dtest.dc9400.plane.feature.scaler.scaler import *
from dtest.dc9400.plane.feature.scaler.baseScaler_filter84 import *
#from dtest.dc9400.plane.feature.scaler.scaler_multiframe import *
############################ Feature format ###########################
#from dtest.dc9400.plane.feature.format.NV16 import *
#from dtest.dc9400.plane.feature.format.AR24 import *
from dtest.dc9400.plane.feature.format.rgb_yuv import *
from dtest.dc9400.plane.feature.format.rgb_yuv_tile import *
############################ Feature y2r ###########################
from dtest.dc9400.plane.feature.y2r.y2r import *
############################ Feature rotation ###########################
from dtest.dc9400.plane.feature.rotation.rotation import *
from dtest.dc9400.plane.feature.rotation.rotation_plus import *
############################ Feature decompress ###########################
from dtest.dc9400.plane.feature.decompress.decompress import *
from dtest.dc9400.plane.feature.decompress.dec400a import *
from dtest.dc9400.plane.feature.decompress.dec400 import *
############################ Feature line padding #########################
from dtest.dc9400.plane.feature.line_padding.line_padding import *
############################ Feature cursor #########################
from dtest.dc9400.plane.feature.cursor.cursor import *
############################ Feature data extend #########################
from dtest.dc9400.plane.feature.data_extend.extend_only import *
from dtest.dc9400.plane.feature.data_extend.extend_alpha import *
############################ Feature multi_extendLayer #####################
from dtest.dc9400.plane.feature.multi_extendlayer.multi_extendlayer import *
############################ Layer CCM #####################
from dtest.dc9400.plane.feature.ccm.ccm import *
############################ Feature layer degamma #####################
from dtest.dc9400.plane.feature.degamma.degamma import *
############################ Feature crop #####################
from dtest.dc9400.plane.feature.crop.crop import *
