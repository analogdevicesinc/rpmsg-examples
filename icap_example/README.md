## ICAP SHARC-ALSA example for Analog Devices Processors

This example implements ICAP device on SHARC0, which communicate over rpmsg "sharc-alsa" endpoints.
SHARC0 initializes and handles audio devices over i2c2 and i2s4, linux must have busses disabled.

### Supported Processor:
  * ADSP-SC598

### Tested with:
  * ADSP-SC598-SOM + CRR-EZKIT

### Build

  - If using git, After cloning the repository, update the git submodules.
      -  git submodule init
      -  git submodule update
  - If downloaded as .zip file, then extract the project. Download the icap and rpmsg-lite from [develop/yocto5.0.0](https://github.com/analogdevicesinc/lnxdsp-examples/tree/develop/yocto-5.0.0) branch ( [ICAP](https://github.com/analogdevicesinc/icap/tree/b0017b5392d377806ac0eed6574fdfe302a843bc) and [rpmsg-lite](https://github.com/analogdevicesinc/rpmsg-lite/tree/b22e61b7f8d290685f26cb596a6521ddd4199e3d) ) to icap and rpmsg-lite folders respectively.
  - Open the project in CCES (version 3.0.3)
  - Select Build project


### Channel Route
"#define ROUTE_ENABLE" Macro in "icap-sharc-alsa_Core1.c" controls the selection which 12 channel should be output on DAC and captured on ARM/Linux. By default this Macro is commented.

In Playback:
- If the ROUTE_ENABLE macro disabled, Channel 1 to 12 will output on DAC 1-12
- If the ROUTE_ENABLE macro enabled, channel 4 to 16 will output on DAC 1-12

In Capture:
- If the ROUTE_ENABLE macro disabled and 16-channel audio is played and recorded using aplay and arecord, the recorded output WAV file will contain data from channels 1-16 mapped directly to output channels 1-16.
- If the ROUTE_ENABLE macro is enabled and 16-channel audio is played and recorded using aplay and arecord, the recorded output WAV file will contain data from channels 4-16 mapped to output channels 1-12. The remaining four channels will be filled with zeros.


To verify, Uncomment the ROUTE_ENABLE in "icap-sharc-alsa_Core1.c", save and build the code.

### Kernel prerequisite

The `aplay` and `arecord` commands shown below request ALSA buffers that exceed
the default `buffer_bytes_max` (0x20000 = 128 kB) of the stock
`sharc-alsa-asoc-card` driver:

| Command | Frames | Bytes (S32_LE, 2 ch) | Stock limit |
|---------|-------:|---------------------:|------------:|
| `aplay  --buffer-size=524288` | 524 288 | 4 194 304 B (0x400000) | **exceeds** 0x20000 |
| `arecord --buffer-size=65536` |  65 536 |   524 288 B (0x80000) | **exceeds** 0x20000 |

Before running the target, apply the kernel patch included in this directory
to your Linux kernel source tree and rebuild the kernel:

```bash
cd <linux-kernel-source>
git am <path-to-rpmsg-examples>/icap_example/0001-ALSA-buffer-size-change-for-mutichannel.patch
make -j$(nproc)
```

The patch raises `buffer_bytes_max` to 0x400000 (4 MB) and
`period_bytes_max` to 0x40000 (256 kB) in
`sound/soc/adi/sharc-alsa-asoc-card.c`.  Without it both example
commands will fail with an `ALSA: buffer size error` or similar.

### Usage
The "#define ICAP_RECORD_EN" macro in context.h enables capture updates in software. By default, this macro is enabled. To validate playback functionality only, disable the ICAP_RECORD_EN macro.

Example playback thru sharc-alsa:<br>
hw:0,0 - playback device<br>

  ``aplay -c2 -fS32_LE -r48000 -D plughw:0,0 --buffer-size=524288 --period-size=32768 ``

Example capture thru sharc-alsa:<br>
hw:1,0 - capture device<br>

  ``arecord -c2 -r48000 -fS32_LE -D plughw:1,0 out.wav -M --buffer-size=65536 --period-size=4096 | aplay 2Ch_L440_R200_48kHz_16bit_6s.wav --buffer-size=524288 --period-size=32768``

### Support

For support with issues related to this module, please post question in the [Analog Devices Linux for ADSP-SC5xx Processors Engineer Zone Forum](https://ez.analog.com/dsp/software-and-development-tools/linux-for-adsp-sc5xx-processors/f/q-a).

### Licensing

The source files included in this project are covered by the license described in the LICENSE file included in this module.  In addition to the source files included in this project the project also pulls in git submodules.  The licensing terms that apply to each submodule are stated in a separate LICENSE file found within that submodule.
