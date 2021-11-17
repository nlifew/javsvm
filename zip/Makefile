


TARGET	=	lib$(notdir $(shell pwd)).a
CXXFLAGS	+=	-I..

$(TARGET)	:	zip_file.o \
				zip_entry.o \
				zip_utils.o 
	$(AR) cr $@ $^

zip_file.o	:	zip_file.cpp \
				zip_file.h	\
				zip_eocd.h

zip_entry.o	:	zip_entry.cpp \
				zip_entry.h

zip_utils.o	:	zip_utils.cpp \
				zip_utils.h