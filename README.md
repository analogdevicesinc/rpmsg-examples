# ADSP-SC5xx RPMsg Examples Repository

This repository contains example SHARC firmware/baremetal applications that are
tested with the Linux remoteproc interface as part of generic
[OpenAMP](https://www.openampproject.org/). The remoteproc driver requires
that a RPMsg resource table has been added to the device-tree and into the
SHARC firmware by default. These examples are not compatible with default
baremetal examples created with CCES studio.

| Core 0 | Core 1     | Core 2     | Example name           |
|--------|------------|------------|------------------------|
| N/A    | Bare Metal | Bare Metal | `echo_examples`[^1]    |
| N/A    | Bare Metal | Bare Metal | `fir_example`          |
| N/A    | Bare Metal | Bare Metal | `icap_example`[^2]     |

[^2]: ICAP SHARC-ALSA multi-channel audio example (SC598 only). See [`icap_example/README.md`](icap_example/README.md).

[^1]: Included in default ADSP Yocto build
