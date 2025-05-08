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

import os, sys
import json
from dtest.helper import get_case_server

infos = dict()
if os.path.exists('script/dtest/dc8200'):
    from dtest.dc8200.chip_info import dc8200_info
    infos.update({'dc8200': dc8200_info})
if os.path.exists('script/dtest/dc9400'):
    from dtest.dc9400.chip_info import dc9400_info
    infos.update({'dc9400': dc9400_info})
if os.path.exists('script/dtest/dc9400_g8'):
    from dtest.dc9400_g8.chip_info import dc9400_g8_info
    infos.update({'dc9400_g8': dc9400_g8_info})
if os.path.exists('script/dtest/dc9500'):
    from dtest.dc9500.chip_info import dc9500_info
    infos.update({'dc9500': dc9500_info})
if os.path.exists('script/dtest/dc9000nano'):
    from dtest.dc9000nano.chip_info import dc9000nano_info
    infos.update({'dc9000nano': dc9000nano_info})
if os.path.exists('script/dtest/dc9000sr'):
    from dtest.dc9000sr.chip_info import dc9000sr_info
    infos.update({'dc9000sr': dc9000sr_info})
if os.path.exists('script/dtest/dec_conform'):
    from dtest.dec_conform.chip_info import dec_conform_info
    infos.update({'dec_conform': dec_conform_info})

def _save_json(path, obj):
    print("Generate %s ..."%path)
    with open(path, 'w') as f:
        json.dump(obj, f, indent=2)

class Maker:
    def __init__(self):
        self.cid_chip_dict = {
            '0x311': 'dc8200',
            '0x31b': 'dc8200',
            '0x316': 'dc9400',
            '0x32a': 'dc9400',
            '0x323': 'dc9400',
            '0x325': 'dc8200',
            '0x326': 'dc9400_g8',
            '0x327': 'dc9400_g8',
            '0x329': 'dc9400_g8',
            '0x331': 'dc8200',
            '0x333': 'dc8200',
            '0x334': 'dc9400',
            '0x2000000a': 'dc9400',
            '0x335': 'dc8200',
            '0x20000005': 'dc9400',
            '0x20000003': 'dc9000nano',
            '0x20000007': 'dc9000sr',
            '0x40000003': 'dc9000sr',
            '0x20000008': 'dc8200',
            '0x20000014': 'dc9400',
            '0x20000015': 'dc9400',
            '0x2000000b': 'dc9400',
        }
        pass

    def get_case_list(self, output_dir, cid, dec_conform):
        case_list = []
        walk_path = os.path.join(output_dir, cid)
        list_file = 'list_{}.txt'.format(cid)
        if dec_conform:
            list_file = 'list_{}_dec_conform.txt'.format(cid)
        for root,dirs,files in os.walk(walk_path):
            for file in files:
                case_list.append(os.path.join(root, file))
        with open(list_file, 'w') as f:
            for case in sorted(case_list):
                f.write(str(case) + '\n')
        print('Total {} cases are generated.'.format(len(case_list)))

    def __call__(self, output_dir, cid='', dec_conform=False):
        if not os.path.exists(output_dir):
            raise Exception('Output dir %s isn\'t exist!'%output_dir)
        if cid not in self.cid_chip_dict.keys():
            print('Unknown cid, please check it again. Exit.')
            sys.exit(-1)
        chip_name = self.cid_chip_dict.get(cid)
        if dec_conform:
            chip_name = 'dec_conform'
        if chip_name not in infos:
            print('Not exist script/dtest/{}. It come from Vivante_DPU_DRM_tst_src_*.tgz.\nPlease double check. Exit.'.format(chip_name))
            sys.exit(-1)
        server = get_case_server()
        cases = server.case_gen(infos[chip_name], cid)
        case_dir = os.path.join(output_dir, '{}'.format(cid))
        if not os.path.exists(case_dir):
            os.mkdir(case_dir)
        for case in cases:
            obj = case.obj_gen()
            fname = os.path.join(case_dir, case.name + '.json')
            _save_json(fname, obj)
        self.get_case_list(output_dir, cid, dec_conform)
