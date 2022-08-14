backenddir = $(CPDB_BACKEND_DIR)
backend_PROGRAMS = file
file_SOURCES = print_backend_file.c \
	       print_backend_file.h \
	       backend_helper.c \
	       backend_helper.h
file_CPPFLAGS  = $(CPDB_CFLAGS)
file_CPPFLAGS += $(GLIB_CFLAGS)
file_CPPFLAGS += $(GIO_CFLAGS)
file_CPPFLAGS += $(GIOUNIX_CFLAGS)

file_LDADD  =  $(CPDB_LIBS)
file_LDADD += -lpthread -lm -lcrypt
file_LDADD += $(GLIB_LIBS)
file_LDADD += $(GIO_LIBS)
file_LDADD += $(GIOUNIX_LIBS)
