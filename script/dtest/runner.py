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

#!/usr/bin/python3
import os
import time
import fnmatch
import subprocess
import json
from .md5_check import check_md5_with_case_golden
from .crc_check import check_crc_case_golden
from .crc_check import init_crc_state
from .crc_check import read_crc_golden

def _exec_cmd(*cmd, check=True):
    return subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            check=check)

class DriverMod:
    def __init__(self, mod_path, interrupt, vcmd):
        self.name = 'vs_drm'
        self.mod_path = mod_path
        self.interrupt = interrupt
        self.vcmd = vcmd
        if not os.path.exists(self.mod_path):
            raise Exception('Driver module %s not found!'%self.mod_path)

    def is_installed(self):
        cmpl = _exec_cmd('lsmod')
        return 'vs_drm'in cmpl.stdout.decode('utf-8')

    def remove(self):
        _exec_cmd('rmmod', self.name)

    def install(self):
        if self.vcmd:
            _exec_cmd('insmod', self.mod_path, self.interrupt, 'VCMD=1')
        else:
            _exec_cmd('insmod', self.mod_path, self.interrupt)

    def reinstall(self):
        if self.is_installed():
            self.remove()
        self.install()

class Runner:
    def __init__(self, program, soft_reset, clear, clean, reinstall, interrupt, crc_check):
        self.program = program
        self.soft_reset = soft_reset
        self.pipe_clear = clear
        self.pipe_clean = clean
        self.reinstall = reinstall
        self.interrupt = interrupt
        self.unsupport = False
        self.no_wb = False
        self.crc_check = crc_check
        if not os.path.exists(program):
            raise Exception('Test program %s not found!'%program)

    def __run_case(self, case, output_dir, soft_reset, pipe_clear, pipe_clean, vdp_addr):
        if not os.path.exists(case):
            print('W Case "%s" not exist\n'%case)
            return False

        #set crc init state
        if self.crc_check:
            init_crc_state()

        cmd = [self.program, '-case', case]
        if output_dir is not None:
            cmd.extend(['-o', output_dir])
        if vdp_addr is not None:
            cmd.extend(['-v', vdp_addr])
        if soft_reset:
            cmd.append('-reset')
        if pipe_clear:
            cmd.append('-clear')
        if pipe_clean:
            cmd.append('-clean')
        #print(cmd)
        cmpl = _exec_cmd(*cmd, check=False)

        if cmpl.returncode != 0:
            print(cmpl.stdout.decode('utf-8','replace'))
            if 'Unsupport' in cmpl.stdout.decode('utf-8'):
                self.unsupport = True
            # return False
        elif 'no wb for bo_dump' in cmpl.stdout.decode('utf-8'):
            print('there is not any writeback for case %s'%case)
            self.no_wb = True;
        return True

    def __call__(self, cases, mod, output_dir, vdp_addr):
        install_by_runner = False
        if self.reinstall:
            if mod.is_installed():
                mod.remove()
            mod.install()
            install_by_runner = True
        elif not mod.is_installed():
            mod.install()
            install_by_runner = True
        fail_cases = []
        pass_cases = []
        unsupport_cases = []

        #read crc golden
        if self.crc_check:
            cases_crc_golden = read_crc_golden()
            with open('./script/dtest/dc9000nano/result.txt', 'w') as file:
                file.write('{}')

        for i,case in enumerate(cases):
            index = i + 1
            print('Run case %4d/%4d - %s ...'%(index, len(cases), case))

            status = self.__run_case(case, output_dir, self.soft_reset, self.pipe_clear, self.pipe_clean, vdp_addr)

            #check the result with golden
            time.sleep(1)
            if self.unsupport:
                unsupport_cases.append((index,case))
                self.unsupport = False
            elif not self.crc_check and self.no_wb:
                unsupport_cases.append((index,case))
                self.no_wb = False
            else:
                if not self.crc_check:
                    result_matched = check_md5_with_case_golden(output_dir, case)
                else:
                    result_matched = check_crc_case_golden(output_dir, case, cases_crc_golden)

                status = status & result_matched
                if status:
                    pass_cases.append((index,case))
                else:
                    fail_cases.append((index,case))
            #close crc
            if self.crc_check:
                cmd = './script/dtest/set_crc.sh 0'
                os.system(cmd)

            if self.reinstall and index < len(cases):
                mod.reinstall()

        if install_by_runner:
            mod.remove()
        # Show results
        print('############################# Results ############################# ')
        for i,case in pass_cases:
            print('%10d - %s [pass]'%(i, case))
        for i,case in fail_cases:
            print('%10d - %s [fail]'%(i, case))
        for i,case in unsupport_cases:
            print('%10d - %s [unsupport]'%(i, case))
        print('Total %d, pass %d, fail %d, unsupport %d'%(len(cases), len(pass_cases), len(fail_cases), len(unsupport_cases)))

def case_filter(cases, filter):
    if filter is None:
        return tuple(cases)
    filtered_cases = []
    for case in cases:
        name = os.path.basename(case)
        if fnmatch.fnmatch(name, filter):
            filtered_cases.append(case)
    return tuple(filtered_cases)
