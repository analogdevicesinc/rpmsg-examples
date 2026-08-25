# Download & Build Instructions

Guidelines for workspace setup, project import, code build process, and LDR file generation using CCES 3.0.3.

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