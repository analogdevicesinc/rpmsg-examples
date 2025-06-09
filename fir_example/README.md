# FIR processing via RPMsg

Note: This is only supported/validated for SC598-SOM-EZKIT.

This project showcases as a practical use case for RPMsg - utilizing SHARCs to access peripherals and CCES/ADI libraries unsupported in Linux. 

To build and test this, use the following steps: 

## Building the project

1) clone the git repository
2) initialize the submodule (rpmsg-lite) using
```
git submodule update --init --recursive
```
3) open existing project using CCES and select this directory
4) select rpmsg_shared_mem_example and press build

This will now compile and generate a .ldr file within rpmsg_shared_mem_example/Debug.

4) Copy the generated .ldr file to the board's file system.

NOTE: You can do so by either scp or via the nfs file system when using NFS boot

## Obtaining rpmsg helper tools

Follow the guide for obtaining and compiling [rpmsg-utils](https://github.com/analogdevicesinc/rpmsg-utils/tree/yocto_5?tab=readme-ov-file#rpmsg-xmit-p-usr_data).

Once obtained, transfer these to the board, same as above.

Follow the instructions for [rpmsg-xmit-p-usr_data](https://github.com/analogdevicesinc/rpmsg-utils/tree/yocto_5?tab=readme-ov-file#rpmsg-xmit-p-usr_data) to load the firmware
and start communicating to the SHARCs. 

Use the input file `input.dat` included in the `/src` subdirectory.

Processed data is stored in the `output.dat` file from the mentioned directory.

## Plotting the data

`input.dat` and `output.dat` can now be plotted easily as these are `.csv` values.

The following is a guide for using Pandas and matplotlib libraries within a python interactive notebook (jupyter-lab):

```
import pandas as pd
import matplotlib.pyplot as plt
```

```
df = pd.read_csv("input.dat")
df.plot()
```

```
df = pd.read_csv("output.dat")
df.plot()
```
