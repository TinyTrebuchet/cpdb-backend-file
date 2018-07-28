backenddir = $(CPDB_BACKEND_DIR)
backend_PROGRAMS = file
file_SOURCES = print_backend_file.c backend_helper.c
file_CPPFLAGS  = $(CPDBBACKEND_CFLAGS)
file_CPPFLAGS += $(GLIB_CFLAGS)
file_CPPFLAGS += $(GIO_CFLAGS)
file_CPPFLAGS += $(GIOUNIX_CFLAGS)

file_LDADD  =  $(CPDBBACKEND_LIBS)
file_LDADD += -lpthread -lm -lcrypt
file_LDADD += $(GLIB_LIBS)
file_LDADD += $(GIO_LIBS)
file_LDADD += $(GIOUNIX_LIBS)
