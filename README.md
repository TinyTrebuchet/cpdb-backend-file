# File Common Print Dialog Backend

This repository contains one of the components I worked on as part of my Google Summer of Code'18 project with the Linux Foundation, the complete documentation can be found [here](https://github.com/ayush268/GSoC_2018_Documentation).
This repository hosts the code for the File **C**ommon **P**rint **D**ialog **B**ackend. This backend manages and provides information about printing to a file via the printing dialog.

## Background

The [Common Printing Dialog](https://wiki.ubuntu.com/CommonPrintingDialog) project aims to provide a uniform, GUI toolkit independent printing experience on Linux Desktop Environments.

## Dependencies

- [cpdb-libs](https://github.com/OpenPrinting/cpdb-libs) : Version >= 1.2.0

- GLIB 2.0:
`sudo apt install libglib2.0-dev`

## Build and installation

```
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
```

## Following the development and updates

The current source code can be found on the [My Github Repo](https://github.com/ayush268/cpdb-backend-file)

## Running

The backend is auto-activated when a frontend runs (which is in cpdb-libs); So no need to run it explicitly.
However, if you wish to see the debug statements in the backend code, you can run `/usr/local/lib/print-backends/file`.
