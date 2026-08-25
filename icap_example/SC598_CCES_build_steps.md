# Download & Build Instructions

Guidelines for workspace setup, project import, code build process, and LDR file generation using CCES 3.0.3.

---

## 0. Apply Required Linux Kernel Patch

The ALSA example commands (`aplay`/`arecord`) in `README.md` require a
larger ALSA buffer than the stock `sharc-alsa-asoc-card` driver allows.
Apply the patch **before** booting the target kernel:

```bash
cd <linux-kernel-source>
git am <path-to-rpmsg-examples>/icap_example/0001-ALSA-buffer-size-change-for-mutichannel.patch
make -j$(nproc)
```

What the patch changes in `sound/soc/adi/sharc-alsa-asoc-card.c`:

| Field | Before | After |
|-------|-------:|------:|
| `period_bytes_max` | 0x10000 (64 kB) | 0x40000 (256 kB) |
| `periods_max` | `PAGE_SIZE/32` (128) | `PAGE_SIZE/4` (1024) |
| `buffer_bytes_max` | 0x20000 (128 kB) | 0x400000 (4 MB) |

Without this patch, the `aplay --buffer-size=524288` command requests
4 194 304 B (0x400000) and `arecord --buffer-size=65536` requests
524 288 B (0x80000) — both exceed the unpatched 128 kB maximum and
will fail at runtime.

---

## 1. Extract Project Files
- Decompress the file into the target directory.
- Download the icap and rpmsg-lite from [develop/yocto5.0.0](https://github.com/analogdevicesinc/lnxdsp-examples/tree/develop/yocto-5.0.0) ( [ICAP](https://github.com/analogdevicesinc/icap/tree/b0017b5392d377806ac0eed6574fdfe302a843bc) and [rpmsg-lite](https://github.com/analogdevicesinc/rpmsg-lite/tree/b22e61b7f8d290685f26cb596a6521ddd4199e3d) ) to icap and rpmsg-lite folders respectively.
---

## 2. Launch CCES
- Launch CCES 3.0.3.

---

## 3. Workspace Setup
- Select a Workspace.
- It is recommended to set the workspace the same as the directory where the project contents are saved.

---

## 4. Import Projects
- From the main menu, follow:
  `File -> Import -> General -> Existing Projects into Workspace -> Next -> Select Root Directory -> Finish`

---

## 5. Build the Project
- From the main menu, select:
  `Project -> Build All`

- Alternatively, right-click on each project and select the build option to build each project if the change is localized to a specific project.

- Wait until the build is completed.

---

## 6. Output
- After the build is completed, the LDR files(icap-sharc-alsa_Core1.ldr and icap-sharc-alsa_Core2.ldr) will be created under the `Debug` folder.