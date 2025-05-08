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

import sys
import os
import hashlib
import json
import argparse
import itertools

###readMe################################################################################################################################
# This function to check the md5sum of result file whether matched with the md5sum golden in test case
# --case case_dir, the case you wanted to check
# --output_dir, the result path dir, we will look for and specify the result file under the naming rules, you do not need to customize
#
# For example:
#   python3 md5_check.py --case /data/dpu/user/DRM/drm-test/debug_cases/multiWriteback.2wb.crtc0Conn0Crtc1Conn1.json --output_dir /data/dpu01/user/DRM/result/
#########################################################################################################################################

def parse_args():
    parser = argparse.ArgumentParser(description='DRM test md5 sum check tool')
    parser.add_argument('--case', help='the case path')
    parser.add_argument('--output_dir', help='Output dir of the results')
    args = parser.parse_args()
    args.parser = parser
    return args

#generte the corresponding filename/path
def generte_dump_name(case, connect_id, frame_id):
    str_temp = case.split("/", -1)
    str_temp = str_temp[-1]
    index = str_temp.rfind(".")
    case_name = str_temp[:index]
    dump_name = case_name + "_" + str(connect_id)
    if frame_id > 0:
        dump_name = dump_name + "_frame" + str(frame_id)
    #print("dump name:%s"%(dump_name))
    return dump_name
def generte_streams_dump_name(case, connect_id, stream_id):
    str_temp = case.split("/", -1)
    str_temp = str_temp[-1]
    index = str_temp.rfind(".")
    case_name = str_temp[:index]
    dump_name = case_name + "_stream_"+ str(stream_id) + "_" + str(connect_id)
    print("dump name:%s"%(dump_name))
    return dump_name
#generte the corresponding stream filename/path
def generte_stream_dump_name(case, connect_id, frame_id):
    str_temp = case.split("/", -1)
    str_temp = str_temp[-1]
    index = str_temp.rfind(".")
    case_name = str_temp[:index]
    dump_name = case_name+ "_" + str(connect_id) + "_frame" + str(frame_id)
    return dump_name


#find the dump result file
def find_dump_file(output_dir, dump_name, for_stream=False):
    found = False
    for file in sorted(os.listdir(output_dir)):
        if dump_name in file and 'ts' not in file:
            found = True
            file_path = output_dir + file
            #print(file_path)
            return file_path
    if not found:
        if not for_stream:
            print("Not find the %s in %s, please check!"%(dump_name, output_dir))
        return None

#calculate the md5 vaule of the file
def cal_md5sum_for_file(file):
    with open(file, "rb") as f:
        data = f.read()
        md5 = hashlib.md5()
        md5.update(data)
        md5sum = md5.hexdigest()
        #print("the %s md5:%s"%(file, md5sum))
        return md5sum

#calculate the frame count of the stream case
def cal_stream_frame_count(case):
    str_temp = case.split("/", -1)
    str_temp = str_temp[-1]
    index = str_temp.rfind(".")
    case_name = str_temp[:index]
    frame_count_temp = case_name.split("_", -1)
    frame_count_temp = frame_count_temp[-1]
    frame_count_index = frame_count_temp.rfind("f")
    frame_count_name = frame_count_temp[:frame_count_index]
    frame_count = int(frame_count_name)
    return frame_count

#print md5 value each frame about stream case
def print_md5_result_each_frame(output_dir, case, data):
        connectors = data["connector"]
        stream_frame_count = cal_stream_frame_count(case)
        md5sums_result = list(itertools.repeat(None, stream_frame_count))
        #print("stream_frame_count= %d\n"%(stream_frame_count))
        for i in range(stream_frame_count):
            #find the dump result file name/path
            dump_name = generte_stream_dump_name(case, 0, i)
            file_path = find_dump_file(output_dir, dump_name, True)
            if file_path is None:
                return False
            #calculate the md5 vaule of the file
            md5sums_result[i] = cal_md5sum_for_file(file_path)
            print("the %s md5 result:%s"%(file_path, md5sums_result[i]))
        return True

#check the result of one frame
def check_md5_result_of_frame(output_dir, pass_dir, fail_dir, case, frame, frame_id, is_stream, stream_id):
    connectors = frame["connector"]
    count = len(connectors)
    md5sums_golden = list(itertools.repeat(None,count))
    md5sums_result = list(itertools.repeat(None,count))
    connect_ids = sorted(list(connectors.keys()))
    for i in range(count):
        connect = connectors.get(connect_ids[i])
        if "globalDisable" in connect:
            continue
        if "golden" not in connect:
            print("no golden in frame%s-wb%s."%(frame_id, connect_ids[i]))
            continue
        golden = connect["golden"]
        #get the expected md5 value in case
        if "md5sum" not in golden:
            print("no md5 golden in case.")
            return False
        md5sums_golden[i] = golden["md5sum"]
        #find the dump result file name/path
        connect_id = int(connect_ids[i])
        if is_stream:
            dump_name = generte_streams_dump_name(case, connect_id, stream_id)
        else:
            dump_name = generte_dump_name(case, connect_id, frame_id)
        file_path = find_dump_file(output_dir, dump_name)
        if file_path is None:
            return False
        #calculate the md5 vaule of the file
        md5sums_result[i] = cal_md5sum_for_file(file_path)
        if md5sums_golden[i] == md5sums_result[i]:
            print("frame%s-wb%s: md5 golden match passed!"%(frame_id, connect_ids[i]))
            os.system('mv {}* {}'.format(file_path, pass_dir))
        else:
            print("frame%s-wb%s: the result %s and the golden %s match failed!"%(frame_id, connect_ids[i], md5sums_result[i], md5sums_golden[i]))
            os.system('mv {}* {}'.format(file_path, fail_dir))
            return False
    return True

#check the md5sum which calculated from the dump result file with the expected md5 golden in case
def check_md5_with_case_golden(output_dir, case):
    if output_dir[-1] != "/":
        output_dir = output_dir + "/"
    pass_dir = os.path.join(output_dir, 'pass')
    fail_dir = os.path.join(output_dir, 'fail')
    if not os.path.exists(pass_dir):
        os.mkdir(pass_dir)
    if not os.path.exists(fail_dir):
        os.mkdir(fail_dir)
    with open(case) as f:
        data = json.load(f)
        if 'frame' in data:
            frames = data["frame"]
            count = len(frames)
            frame_ids = sorted(list(frames.keys()))
            rets = True
            for i in range(count):
                frame = frames.get(frame_ids[i])
                ret = check_md5_result_of_frame(output_dir, pass_dir, fail_dir, case, frame, int(frame_ids[i]), 0, 0)
                rets = rets & ret
            return rets
        elif 'stream' in data:
            streams = data["stream"]
            count = len(streams)
            stream_ids = sorted(list(streams.keys()))
            for i in range(count):
                stream = streams.get(stream_ids[i])
                ret = check_md5_result_of_frame(output_dir, pass_dir, fail_dir, case, stream, 0, 1, int(stream_ids[i]))
                rets = ret & ret
                ##print_md5_result_each_frame(output_dir, case, stream)
            return rets
        else:
            ret = check_md5_result_of_frame(output_dir, pass_dir, fail_dir, case, data, 0, 0, 0)
            return ret

###### use for man set debug#####
def main(output_dir,case_path):
    check_md5_with_case_golden(output_dir, case_path)

if __name__ == '__main__':
    args = parse_args()
    if args.case and args.output_dir is not None:
        main(args.output_dir, args.case)
    else:
        args.parser.print_help()
