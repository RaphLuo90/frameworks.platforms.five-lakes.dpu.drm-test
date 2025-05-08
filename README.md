# DRM test

1. About cases

Dtest uses python3 script to generate json case decriptions, so user need to write
case with python3 and run `dtest.py` to generate and run it.

2. Generate cases
First to get $cid from release pacakge name: Vivante_DPU_DRM_drv_src_DCF_*.tgz.
It start with '0x'.

Generate cases command:
    python3 script/dtest.py -g --cid $cid

Then cases are saved in auto_cases/$cid folder.

3. Run cases
First to insmod the vs_drm.ko kernel module.

(Use apb mode as default: insmod vs_drm.ko; if vcmd mode is needed: insmod vs_drm.ko VCMD=1)

Then to setup the `PATH` and `LD_LIBRARY_PATH` environments.
    For example:
        export BO_HELPER_DIR='root@drm-root/bo-helper'
        export LIBDRM_DIR='root@drm-root/libdrm'
        export DRM_TEST_DIR='root@drm-root/drm-test'
        export DRM_DRIVER_DIR='root@drm-root/driver'
        export PATH=$PATH:$LIBDRM_DIR/tests/modetest/.libs:$DRM_TEST_DIR/build/tools
        export LD_LIBRARY_PATH=$BO_HELPER_DIR/sdk/lib:$LIBDRM_DIR/out/lib

4. Common commands

4.1 Run case with command.

```shell
drmtest -case path/to/json -o path/to/dump_dir -reset
Or
./build/tools/drmtest -case path/to/json -o path/to/dump_dir -reset
```
('-reset' means reset all registers after running)
(Before running each case, need to make sure the environment is clean.)

4.2 Run case with python3 script.

4.2.1 Run on fpga
4.2.1.1 For dc9000nano: (DCF package name is Vivante_DPU_DRM_drv_src_DCF_9000nano_*.tgz)
```shell
python3 script/dtest.py -d path/to/case_dir --output_dir path/to/dump_dir --soft --clear --crc_golden
```

4.2.1.2 For dc9000sr: (DCF package name is Vivante_DPU_DRM_drv_src_DCF_9000sr_*.tgz)
```shell(apb)
python3 script/dtest.py -d path/to/case_dir --output_dir path/to/dump_dir --soft
```
```shell(vcmd)
python3 script/dtest.py -d path/to/case_dir --output_dir path/to/dump_dir --clean
```
('--clean' means reset context and command buffer, before running each cases)

4.2.1.3 For other projects:
```shell
python3 script/dtest.py -d path/to/case_dir --output_dir path/to/dump_dir --soft --reinstall
```
('--soft' means reset all registers after running, '--reinstall' means rmmod, insmod driver ko before running each cases)

4.2.2 Run on qemu:
4.2.2.1 For most cases except for vcmd mode :
```shell
python3 script/dtest.py -d path/to/case_dir --output_dir path/to/dump_dir --soft

4.2.2.2 For vcmd mode:
```shell(vcmd)
python3 script/dtest.py -d path/to/case_dir --output_dir path/to/dump_dir --clean

5. Filter cases(optional)

If run case with script, there is an option to filter cases to run.

```shell
# Run cases from list file.
python3 script/dtest.py -d path/to/case_dir --output_dir path/to/dump_dir --soft --reinstall --list $list

# Run plane0 cases
python3 script/dtest.py -d path/to/case_dir --output_dir path/to/dump_dir --soft --reinstall -f *plane0*

# Run plane1 hdr cases
python3 script/dtest.py -d path/to/case_dir --output_dir path/to/dump_dir --soft --reinstall -f *plane1*hdr*

# Run crtc cases
python3 script/dtest.py -d path/to/case_dir --output_dir path/to/dump_dir --soft --reinstall  -f *crtc*
```

6. Clear pipes (optional)

Disable all the objects(plane/crtc) used on all pipes

```shell
# Command run
drmtest -case path/to/json -o path/to/dump_dir -clear

# Script run
python3 script/dtest.py -d path/to/case_dir --output_dir path/to/dump_dir --clear
```
