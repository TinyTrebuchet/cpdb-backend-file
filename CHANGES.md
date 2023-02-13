# CHANGES - Common Print Dialog Backends - File Backend - v2.0b2 - 2023-02-13

## CHANGES IN V2.0b2 (13th February 2022)

- Let frontend synchronously fetch printer list upon activation (PR #5)

- Added the support of cpdb-libs for translations
  * Using general message string catalogs from CUPS and also message
    string catalogs from individual print queues/IPP printers.
  * Message catalog management done by libcupsfilters 2.x, via the
    `cfCatalog...()` API functions (`catalog.h`).

- Option group support

- Log messages handled by frontend

- Removed hardcoded paths (PR #3)

- Build system: Let "make dist" also create .tar.bz2 and .tar.xz


## CHANGES IN V2.0b1 (13th December 2022)

- Added missing handlers for D-Bus methods added in CPDB 2.x

- Adapted to renamed API functions and data types of cpdb-libs 2.x

- Updated signal names to match those emitted from CPDB frontend

- Made "make dist" generate a complete source tarball

- Updated README.md

  + On update the old version of the backend needs to get killed
  + Common Print Dialog -> Common Print Dialog Backends
  + Requires cpdb-libs 2.0.0 or newer
  + Updated instructions for running the backend.
  + Added link to Gaurav Guleria's and Nilanjana Lodh's GSoC work
  + Put mention of Ayush's GSoC work into third person.
