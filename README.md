# Rpmsg echo example for ADI sc5xx-sharc platforms.
The example shows how to use rpmsg endpoints to communicate between ARM and other SHARC cores.
The example uses [fork of rpmsg-lite library by NXP with adsp-sc5xx-sharc platform support added (feature/add_sc5xx_sharc_platform branch).](https://github.com/analogdevicesinc/rpmsg-lite)

* Rpmsg-lite documentation: [https://nxpmicro.github.io/rpmsg-lite/index.html](https://nxpmicro.github.io/rpmsg-lite/index.html "rpmsg-lite documentation").


## Rpmsg-lite integration into a CCES project
1. Create CCES project for sc5xx-sharc platform.
2. Remove MCAPI module, add TRU module.
3. Include rpmsg-lite dir into the project, as a git submodule, copy of the rpmsg-lite dir or as a link to the rpmsg-lite dir.
4. In the project exclude from build unused platforms and features:
	* exclude all dirs from rpmsg-lite/lib/include/platform except adsp-sc5xx-sharc
	* exclude all dirs files from rpmsg-lite/lib/rpmsg_lite/porting/platform except adsp-sc5xx-sharc
	* exclude all *.c files from rpmsg-lite/lib/rpmsg_lite/porting/environment except rpmsg_env_bm.c
	* exclude rpmsg_queue.c file from rpmsg-lite/lib/rpmsg_lite
5. Create rpmsg_config.h file which defines rpmsg-lite configuration. In the file define `#define RL_USE_STATIC_API (1)`, currently only static API is supported on sc5xx-sharc platform.
6. Add paths to include directories for all build configurations (DEBUG and RELEASE):
	* dir containing newly created rpmsg_config.h
	* rpmsg-lite/lib/include
	* rpmsg-lite/lib/include/platform/adsp-sc5xx-sharc
7. If you plan to load sharc firmware using remoteproc from Linux configure project to produce ldr file:
	* [ADI remoteproc wiki](https://wiki.analog.com/resources/tools-software/linuxdsp/docs/linux-kernel-and-drivers/remoteproc/remoteproc)
	* [Steps to configure CCES project for LDR binary file](https://wiki.analog.com/resources/tools-software/linuxdsp/docs/linux-kernel-and-drivers/remoteproc/remoteproc_ldr_generate)


## Echo example usage ##
### Initialization ###
Using Linux command line start cores with the echo example.
The example firmware announces created endpoints on rpmsg bus,
Linux creates a rpmsg device for each endpoint in `/sys/bus/rpmsg/devices`
To communicate to the endpoints from userspace kernel must be compiled with `CONFIG_RPMSG_CHAR` which enables rpmsg-char driver. Bind a rpmsg device to the driver manually or using `rpmsg-bind-chardev` which is a helper application for binding rpmsg devices to the rpmsg-char driver. It can be found in the rpmsg-utils package from meta-adi Yocto layer. The bind creates a `/dev/rpmsgX` file which an user space application can use to communicate with an endpoint created in SHARC firmware.

SHARC Core1 initialization script for the echo example:
```shell
#!/bin/sh
echo rpmsg_echo_example_Core1.ldr > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state

RPMSG_EP=$(basename $(ls -d /sys/bus/rpmsg/devices/*.sharc-echo.-1.151))
./rpmsg-bind-chardev -d ${RPMSG_EP} -a 50

RPMSG_EP=$(basename $(ls -d /sys/bus/rpmsg/devices/*.sharc-echo-cap.-1.161))
./rpmsg-bind-chardev -d ${RPMSG_EP} -a 61
```

SHARC Core2 initialization script for the echo example:
```shell
#!/bin/sh
echo rpmsg_echo_example_Core2.ldr > /sys/class/remoteproc/remoteproc1/firmware
echo start > /sys/class/remoteproc/remoteproc1/state

RPMSG_EP=$(basename $(ls -d /sys/bus/rpmsg/devices/*.sharc-echo.-1.152))
./rpmsg-bind-chardev -d ${RPMSG_EP} -a 51

RPMSG_EP=$(basename $(ls -d /sys/bus/rpmsg/devices/*.sharc-echo-cap.-1.162))
./rpmsg-bind-chardev -d ${RPMSG_EP} -a 62
```

### Communicate ###
It is possible to send a messages to an endpoint on SHARC using regular echo
```console
root@adsp-sc594-som-ezkit:~# echo "Hello!" > /dev/rpmsg0
```
But the messages coming from the endpoint are being dropped if the /dev/rpmsg0 file is closed. Therefore the rpmsg-utils package also contains an rpmsg-xmit helper application which reads a message from stdin, writes it to the /dev/rpmsgX file, while keeping the file open reads response and prints on stdout.

```console
root@adsp-sc594-som-ezkit:~# echo hello | rpmsg-xmit -n 5 /dev/rpmsg0
hello => echo from Core1
root@adsp-sc594-som-ezkit:~# echo hello | rpmsg-xmit -n 5 /dev/rpmsg1
HELLO => capitalized echo from Core1
root@adsp-sc594-som-ezkit:~# echo hello | rpmsg-xmit -n 5 /dev/rpmsg2
hello => echo from Core2
root@adsp-sc594-som-ezkit:~# echo hello | rpmsg-xmit -n 5 /dev/rpmsg3
HELLO => capitalized echo from Core2
```
