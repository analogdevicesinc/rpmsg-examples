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

[^1]: Included in default ADSP Yocto build
