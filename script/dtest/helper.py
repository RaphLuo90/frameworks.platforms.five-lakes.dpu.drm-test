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
import os, re, sys

class Crtc:
    def __init__(self, id : int, properties=None):
        self.id = id
        self.properties = properties.copy() if properties is not None else OrderedDict()

    def obj_gen(self):
        elements = []
        if len(self.properties) > 0:
            elements.append(('property', self.properties.copy()))
        return OrderedDict(elements)

class Cursor:
    def __init__(self, id : int, crtc : Crtc, format : str, cursorPosX=None, cursorPosY=None, cursorWidth=None, cursorHeight=None,
                 cursorHotX=None, cursorHotY=None, properties=None):
        self.id = id
        self.crtc = crtc
        self.format = format
        self.cursorPosX = cursorPosX
        self.cursorPosY= cursorPosY
        self.cursorWidth  = cursorWidth
        self.cursorHeight = cursorHeight
        self.cursorHotX = cursorHotX
        self.cursorHotY = cursorHotY
        self.properties = properties.copy() if properties is not None else OrderedDict()

    def obj_gen(self):
        elements = []
        elements = [
            ('crtc', self.crtc.id),
            ('format', self.format),
            ('cursorPosX', (self.cursorPosX != None) and self.cursorPosX or 0),
            ('cursorPosY', (self.cursorPosY != None) and self.cursorPosY or 0),
            ('cursorWidth', (self.cursorWidth != None) and self.cursorWidth or 0),
            ('cursorHeight', (self.cursorHeight != None) and self.cursorHeight or 0),
            ('cursorHotX', (self.cursorHotX != None) and self.cursorHotX or 0),
            ('cursorHotY', (self.cursorHotY != None) and self.cursorHotY or 0),
        ]
        if len(self.properties) > 0:
            elements.append(('property', self.properties.copy()))
        return OrderedDict(elements)

class Plane:
    def __init__(self, id : int, crtc : Crtc, format : str,
            width : int, height : int, refresh : int, properties=None, resource="defaultPattern", decType="NONE", tileMode="LINEAR",
            crtcX=None, crtcY=None, crtcWidth=None, crtcHeight=None, ori_properties=None, tileStatus=None):
        self.id = id
        self.crtc = crtc
        self.format = format
        self.resource = resource
        self.tileStatus = tileStatus
        self.decType = decType
        self.tileMode = tileMode
        self.width = width
        self.height = height
        self.refresh = refresh
        self.ori_properties = ori_properties.copy() if ori_properties is not None else OrderedDict()
        self.properties = properties.copy() if properties is not None else OrderedDict()
        self.crtc_x = crtcX
        self.crtc_y = crtcY
        self.crtc_w = crtcWidth
        self.crtc_h = crtcHeight

    def obj_gen(self):
        elements = [
            ('crtc', self.crtc.id),
            ('format', self.format),
            ('resource', self.resource),
            ('decType', self.decType),
            ('tileMode', self.tileMode),
            ('width', self.width),
            ('height', self.height),
            ('refresh', self.refresh),
            ('crtcX', (self.crtc_w != None) and self.crtc_x or 0),
            ('crtcY', (self.crtc_w != None) and self.crtc_y or 0),
            ('crtcWidth', (self.crtc_w != None) and self.crtc_w or self.width),
            ('crtcHeight', (self.crtc_w != None) and self.crtc_h or self.height),
            ]

        if self.tileStatus is not None:
            start_index = 3
            if isinstance(self.tileStatus, list):
                for ts in self.tileStatus:
                    elements.insert(start_index, ts)
                    start_index +=1
            elif isinstance(self.tileStatus, str):
                elements.insert(start_index, ('tileStatus', self.tileStatus))
            else:
                raise NotImplementedError('Unsupport tileStatus type {}'.format(type(self.tileStatus)))

        if len(self.ori_properties) > 0:
            for key, value in self.ori_properties.items():
                elements.append((key, value))
        if len(self.properties) > 0:
            elements.append(('property', self.properties.copy()))
        return OrderedDict(elements)

class Connector:
    def __init__(self, id : int, crtc : Crtc, format : str,
        width : int, height : int, refresh : int, properties=None, golden=None, other_prop=None):
        self.id = id
        self.crtc = crtc
        self.format = format
        self.width = width
        self.height = height
        self.refresh = refresh
        self.properties = properties.copy() if properties is not None else OrderedDict()
        self.other_prop = other_prop.copy() if other_prop is not None else OrderedDict()
        if golden is not None:
            self.golden = golden.copy()

    def obj_gen(self, name, frameID):
        self.name = name
        elements = [
            ('crtc', self.crtc.id),
            ('format', self.format),
            ('width', self.width),
            ('height', self.height),
            ('refresh', self.refresh),
            ]
        if str(self.name).startswith('connector') and 'scaler' in self.name:
            elements.append(('wb_width', self.properties["SCALER"]["dst_w"]))
            elements.append(('wb_height', self.properties["SCALER"]["dst_h"]))
        elif str(self.name).startswith('connector') and 'spliter' in self.name:
            elements.append(('wb_width', self.other_prop["wb_width"]))
            elements.append(('wb_height', self.other_prop["wb_height"]))
        elif len(self.other_prop) > 0:
            for key, value in self.other_prop.items():
                elements.append((key, value))

        if len(self.properties) > 0:
            elements.append(('property', self.properties.copy()))

        if hasattr(self, 'golden'):
            self.golden = self.get_golden_unify(frameID)
            if len(self.golden) > 0:
                elements.append(('golden', self.golden.copy()))
        return OrderedDict(elements)

    def get_golden_unify(self, frameID):
        golden_file_cid = './script/dtest/{}/.golden/golden_{}_{}.txt'.format(chip_name, chip_name, customer_id)
        golden_file_common = './script/dtest/{}/.golden/golden_{}.txt'.format(chip_name, chip_name)
        golden_file_fusa = './script/dtest/{}/.golden/golden_{}_fs.txt'.format(chip_name, chip_name)
        fusa_cid_list = ['0x31b', '0x333', '0x20000008', '0x325']

        if customer_id in fusa_cid_list:
            golden_file = golden_file_fusa
        elif os.path.exists(golden_file_cid):
            golden_file = golden_file_cid
        elif os.path.exists(golden_file_common):
            golden_file = golden_file_common
        else:
            golden_file = './script/dtest/{}/.golden/golden_*.txt'.format(chip_name)
            print('Oops!! Not find {}.\nIt is in "Vivante_DPU_DRM_tst_src_*.tgz". Please double check. Exit.'.format(golden_file))
            sys.exit(-1)
        if frameID == 0:
            dump_name = self.name + '_' + str(self.id)
        else:
            dump_name = self.name + '_' + str(self.id) + '_frame' + str(frameID)
        with open(golden_file) as f:
            content = f.read()
            if re.search("(.*)  "+dump_name, content, re.I|re.M):
                md5sum = re.search("(.*)  "+dump_name, content, re.I|re.M).group(1)
                return OrderedDict([('md5sum', md5sum)])
            else:
                return OrderedDict([('md5sum', 'not_support')])

class DTestUnit:
    def __init__(self, planes, connectors, name : str, frameID = 0, cursors=None):
        if isinstance(planes, Plane):
            planes = [planes]
        if isinstance(cursors, Cursor):
            cursors = [cursors]
        if isinstance(connectors, Connector):
            connectors = [connectors]
        if type(planes) not in [list, tuple]:
            raise NotImplementedError('Unsupport plane type {}'.format(type(planes)))
        if type(connectors) not in [list, tuple]:
            raise NotImplementedError('Unsupport connectors type {}'.format(type(connectors)))
        if cursors is not None:
            if type(cursors) not in [list, tuple]:
                raise NotImplementedError('Unsupport cursors type {}'.format(type(cursors)))

        self.planes = tuple(planes)
        self.cursors = tuple(cursors)  if cursors is not None else None
        self.connectors = tuple(connectors)
        self.name = name
        self.id = frameID

    @property
    def crtcs(self):
        plane_crtcs = set([p.crtc for p in self.planes])
        conn_crtcs = set([c.crtc for c in self.connectors])
        # while plane side by side split enable, one plane select one crtc(suppose crtc0), two crtcs(crtc0 and crtc1) can get data from one plane.
        temp_crtcs = plane_crtcs
        if len(plane_crtcs - conn_crtcs) > 0:
            temp_crtcs = plane_crtcs
        if len(conn_crtcs - plane_crtcs) > 0:
            temp_crtcs = conn_crtcs
        return tuple(sorted(temp_crtcs, key=lambda x:x.id))

    def obj_gen(self):
        elements = []
        planes = OrderedDict(((str(p.id), p.obj_gen()) for p in self.planes))
        cursors = OrderedDict()
        if self.cursors is not None:
            cursors = OrderedDict(((str(cs.id), cs.obj_gen()) for cs in self.cursors))
        connectors = OrderedDict(((str(c.id), c.obj_gen(self.name, self.id)) for c in self.connectors))
        crtcs = OrderedDict(((str(c.id), c.obj_gen()) for c in self.crtcs))
        elements.append(('plane', planes))
        if cursors:
            elements.append(('cursor', cursors))
        elements.append(('connector', connectors))
        elements.append(('crtc', crtcs))
        return OrderedDict(elements)

class DTestFrameUnit:
    def __init__(self, frames, name : str):
        if isinstance(frames, DTestUnit):
            frames = [frames]
        self.frames = tuple(frames)
        self.name = name

    def obj_gen(self):
        elements = []
        frames = OrderedDict(((str(f.id), f.obj_gen()) for f in self.frames))
        elements.append(('frame', frames))
        return OrderedDict(elements)

class DTestCasesServer:
    def __init__(self):
        self.__cases_generator = {}

    def add_case_generator(self, chip, gen):
        if chip not in self.__cases_generator:
            self.__cases_generator[chip] = []
        self.__cases_generator[chip].append(gen)

    def case_gen(self, chip_info, cid):
        global chip_name
        chip_name = chip_info.name
        global customer_id
        customer_id = cid
        if chip_info.name not in self.__cases_generator:
            raise Exception('No case loaded for "%s"'%chip_info.name)
        cases = []
        for gen in self.__cases_generator[chip_info.name]:
            ret = gen(chip_info)
            if type(ret) in [list, tuple]:
                cases.extend(list(ret))
            elif type(ret) in [DTestUnit]:
                cases.append(ret)
            elif type(ret) in [DTestFrameUnit]:
                cases.append(ret)
            else:
                raise Exception('Unsupport case type {}'.format(type(ret)))
        return cases

case_server = DTestCasesServer()
def get_case_server():
    global case_server
    return case_server

def dtest_unit_group(chip):
    def _wrapper(func):
        global case_server
        case_server.add_case_generator(chip, func)
    return _wrapper

def dtest_unit(chip):
    def _wrapper(func):
        global case_server
        case_server.add_case_generator(chip, func)
    return _wrapper
