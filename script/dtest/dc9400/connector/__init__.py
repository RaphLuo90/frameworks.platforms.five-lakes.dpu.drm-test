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

############################ Feature.postwb ############################
from dtest.dc9400.connector.feature.postwb.data_extend import *
from dtest.dc9400.connector.feature.postwb.data_trunc import *
from dtest.dc9400.connector.feature.postwb.wb_dither import *
from dtest.dc9400.connector.feature.postwb.uv_downsample import *
from dtest.dc9400.connector.feature.postwb.r2y import *
from dtest.dc9400.connector.feature.postwb.gamma_dither import *
from dtest.dc9400.connector.feature.postwb.gamma import *
############################ Feature.crop ############################
from dtest.dc9400.connector.feature.wbCrop.crop import *
############################ Feature.format ############################
from dtest.dc9400.connector.feature.format.rgb import *
############################ Feature.compress ############################
from dtest.dc9400.connector.feature.compress.compress import *
############################ Feature.stripe ############################
from dtest.dc9400.connector.feature.stripe.stripe import *