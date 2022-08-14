# File Common Print Dialog Backend

This repository contains one of the components Ayush Bansal worked on as part of his Google Summer of Code 2018 project with the Linux Foundation, the complete documentation can be found [here](https://github.com/ayush268/GSoC_2018_Documentation).

This repository hosts the code for the File **C**ommon **P**rint **D**ialog **B**ackend. This backend manages and provides information about printing to a file via the print dialog.

## Background

The [Common Print Dialog Backends](https://openprinting.github.io/achievements/#common-print-dialog-backends) project aims to move the responsability on the part of the print dialog which communicates with the print system away from the GUI toolkit/app developers to the print system's developers and also to bring all print technologies available to the user (CUPS, cloud printing services, ...) into all application's print dialogs.

## Dependencies

- [cpdb-libs](https://github.com/OpenPrinting/cpdb-libs): Version >= 2.0.0 (or GIT Master)

- GLIB 2.0:
`sudo apt install libglib2.0-dev`

## Build and installation

```
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
```

## Running

The backend is auto-activated when a frontend (like a CPDB-supporting print dialog or the example frontend `demo/print_frontend` of cpdb-libs) is started, so there is no need to run it explicitly.

However, if you wish to see the debug statements in the backend code, you can run `/usr/local/lib/print-backends/file`.
