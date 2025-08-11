# Yocto Linux for ADSP-SC5xx: RPMsg Examples Repository

This repository contains ARM/Linux examples that are tested
with Linux remoteproc interface. Remoteproc driver requires RPMsg resource
table in device-tree and in SHARC firmware by default, not compatible with
default baremetal examples created with CCES studio. 

| Core 0 | Core 1     | Core 2     | Example name                                     |
|--------|------------|------------|--------------------------------------------------|
| N/A    | Bare Metal | Bare Metal | echo_examples, comes in default ADSP-Linux build |
| N/A    | Bare Metal | Bare Metal | fir_example                                      |


