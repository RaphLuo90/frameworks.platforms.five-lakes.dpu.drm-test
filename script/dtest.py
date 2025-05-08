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
import shutil
import argparse
if os.path.exists('script/dtest/maker.py'):
    from dtest.maker import Maker
from dtest.runner import Runner,DriverMod,case_filter

def str2bool(f):
    if isinstance(f, bool):
        return f
    if f.lower() in ('yes', 'true', 'y', 't', '1'):
        return True
    elif f.lower() in ('no', 'false', 'n', 'f', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')

def parse_args():
    parser = argparse.ArgumentParser(description='DRM test tool')
    #parser.add_argument('case', type=str, help='Case to run')
    parser.add_argument('-g', '--generate', action='store_true', help='Generate cases json')
    parser.add_argument('-f', '--filter', help='Run case with filter')
    parser.add_argument('-m', '--mod', help='Driver mod ko')
    parser.add_argument('-p', '--program', help='drmtest program')
    parser.add_argument('-d', '--case-dir', help='Case directory')
    parser.add_argument('--cid', help='Specify the customer id')
    parser.add_argument('--soft', action='store_true', help='Soft reset mode')
    parser.add_argument('--clear', action='store_true', help='Clear all pipes')
    parser.add_argument('--clean', action='store_true', help='Clean all sw state for SR')
    parser.add_argument('--reinstall', action='store_true', help='Reinstall driver mode')
    parser.add_argument('--output_dir', help='Output dir for the results')
    parser.add_argument('--vdp_addr', help='vdp address')
    parser.add_argument('--interrupt', default='', help='for 0x32A: NS=1 TZ=1 GSA=1 AOC=1')
    parser.add_argument('--list', default='', help='Specify the case list')
    parser.add_argument('--crc_golden', action='store_true', help='use CRC to check golden')
    parser.add_argument('--vcmd', type=str2bool, default=False, help='vcmd run mode')
    parser.add_argument('--dec-conform', action='store_true', help='test dec conformance')
    args = parser.parse_args()
    args.parser = parser
    return args

def make_cases(output_dir='auto_cases', cid='', dec_conform=False):
    if dec_conform:
        output_dir = 'dec_conform_cases'
    else:
        output_dir = 'auto_cases'
    case_dir = os.path.join(output_dir, cid)
    if os.path.exists(case_dir):
        shutil.rmtree(case_dir)
    if os.path.exists(case_dir):
        raise Exception("Remove %s dir fail, please remove it manully."%case_dir)
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
    maker = Maker()
    maker(output_dir, cid, dec_conform)

def load_cases(case_dir, case_list):
    cases = []
    if case_list:
        with open(case_list, 'r') as f:
            for line in f.readlines():
                if not line.startswith('#'):
                    cases.append(line.replace('\n', ''))
    else:
        for root,dirs,files in os.walk(case_dir):
            for file in files:
                cases.append(os.path.join(root, file))
    return tuple(sorted(cases))

def run_cases(program, mod, case_dir, output_dir, soft=True, clear=False, clean=False, reinstall=False, filter=None, interrupt='', case_list='', crc_golden=False, vcmd=True, vdp_addr=''):
    if output_dir is not None and not os.path.exists(output_dir):
        raise Exception('Output dir "%s" not found!'%output_dir)
    cases = case_filter(load_cases(case_dir, case_list), filter)
    if len(cases) == 0:
        print('No case to run.')
        return
    mod = DriverMod(mod, interrupt, vcmd)
    runner = Runner(program, soft, clear, clean, reinstall, interrupt, crc_golden)
    runner(cases, mod, output_dir, vdp_addr)

def load_env(program, mod):
    if program is None:
        drmtest = os.environ.get('DRM_TEST_DIR', None)
        if drmtest is None:
            raise Exception('Must provide drmtest program or set DRM_TEST_DIR env.')
        program = os.path.join(drmtest, 'build', 'tools', 'drmtest')
        if not os.path.exists(program):
            raise Exception('Please build drmtest first!')
    if mod is None:
        driver = os.environ.get('DRM_DRIVER_DIR', None)
        if driver is None:
            raise Exception('Must provide driver module or set DRM_DRIVER_DIR env.')
        mod = os.path.join(driver, 'verisilicon', 'vs_drm.ko')
        if not os.path.exists(mod):
            raise Exception('Please build driver module first!')
    return program,mod

def main():
    args = parse_args()
    if args.generate:
        if not args.cid:
            print('Must add "--cid $cid". \nYou could get $cid from release pacakge name: Vivante_DPU_DRM_drv_src_DCF_*.tgz. \nIt start with "0x", such as 0x316. Thanks.')
            sys.exit(-1)
        make_cases(cid=str(args.cid).lower(), dec_conform=args.dec_conform)
    elif args.case_dir is not None:
        program,mod = load_env(args.program, args.mod)
        run_cases(program, mod, args.case_dir, args.output_dir, args.soft,
                args.clear, args.clean, args.reinstall, args.filter, args.interrupt,
                args.list, args.crc_golden, args.vcmd, args.vdp_addr)
    else:
        args.parser.print_help()

if __name__ == '__main__':
    main()
