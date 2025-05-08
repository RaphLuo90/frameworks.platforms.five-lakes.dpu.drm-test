import linecache
import re
import os
import json
from collections import OrderedDict

def init_crc_state():
    cmd = './script/dtest/set_crc.sh 1'
    os.system(cmd)

def check_crc_case_golden(output_dir, case, cases_crc_golden):
    case_split = case.split('/')
    case_name = case_split[-1]
    with open("./script/dtest/dc9000nano/result.txt", "r", encoding="utf8") as f:
        cases_first_crc_golden = json.load(f, object_pairs_hook=OrderedDict)

    filename = "/sys/kernel/debug/dri/1/crtc-0/CRC"
    crc_temp_last = read_crc(filename)
    crc_temp = OrderedDict()
    crc_temp['a'] = crc_temp_last["last-alpha-crc"]
    crc_temp['r'] = crc_temp_last["last-red-crc"]
    crc_temp['g'] = crc_temp_last["last-green-crc"]
    crc_temp['b'] = crc_temp_last["last-blue-crc"]
    cases_first_crc_golden[case_name] = crc_temp

    with open('./script/dtest/dc9000nano/result.txt', 'w') as fx:
        json.dump(cases_first_crc_golden, fx, indent=4, separators=(',', ':'))

    if case_name in cases_crc_golden:
        case_crc_golden = cases_crc_golden[case_name]
        result_matched = check_md5_with_case_golden_crc(output_dir, case, case_crc_golden)
    else:
        print("crc golden not found, match failed!")
        result_matched = False

    return result_matched

def read_crc_golden():
    golden_path = "./script/dtest/dc9000nano/.golden/golden_dc9000nano.txt"

    with open(golden_path, "r", encoding="utf8") as f:
        cases_crc_golden = json.load(f)

    return cases_crc_golden

def read_crc(filename):
	result = {}
	with open(filename, 'r') as f:
		lines = f.readlines()
	for line in lines:
		if (line.find("last-alpha-crc") != -1 or line.find("last-red-crc") != -1
		or line.find("last-green-crc") != -1 or line.find("last-blue-crc") != -1):
			line = re.sub(r'\s+', '', line)
			line = line.replace("[", "")
			line = line.replace("]", "")
			splited = line.split('=')

			result[splited[0]] = splited[1]

	return result

#check the md5sum which calcuate form the dump result copied crc file with the expect md5 golden in case
def check_md5_with_case_golden_crc(output_dir, case, case_crc_golden):
    if output_dir[-1] != "/":
        output_dir = output_dir + "/"
    pass_dir = os.path.join(output_dir, 'pass')
    fail_dir = os.path.join(output_dir, 'fail')
    if not os.path.exists(pass_dir):
        os.mkdir(pass_dir)
    if not os.path.exists(fail_dir):
        os.mkdir(fail_dir)

    crc_file = "/sys/kernel/debug/dri/1/crtc-0/CRC"

    crc_dict = read_crc(crc_file)
    if(case_crc_golden["a"] == crc_dict["last-alpha-crc"] and \
	    case_crc_golden["r"] == crc_dict["last-red-crc"] and \
	    case_crc_golden["g"] == crc_dict["last-green-crc"] and \
	    case_crc_golden["b"] == crc_dict["last-blue-crc"]):
	    print("crc golden match passed!")
	    return True
    else:
	    print("crc golden match failed!")
	    return False
