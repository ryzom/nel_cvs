#############################################################################
# THIS makefile is temporary, Actually we can't compile login service without
# mysql so we need to add the stuffs in auto* file to handle that.

#############################################################################
# Setting up the compiler settings...

# The names of the executables
CXX          = c++
RM           = rm -f
MAKE         = make

# The flags for the C++ compiler
CXXFLAGS     = -g -pipe -D_REENTRANT -D_GNU_SOURCE  \
				-I$(HOME)/cvs/code/nel/include \
				-I/home/installs/STLport-4.5.1/stlport \
				-I/usr/local/include \
				-I/usr/include/mysql

# The flags for the linker
LDFLAGS      = -L$(HOME)/install/release/lib \
				-L/home/installs/STLport-4.5.1/lib \
				-L/usr/local/lib \
				-L/home/nevrax/cvs/code/ryzom/src_v2/game_share \
				-lnelmisc \
				-lnelnet \
				-lmysqlclient \
				-lstlport_gcc \
				-lpthread

#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= login_service

# The default build rule
all:			$(TARGETS)

OBJS	= connection_web.o connection_ws.o login_service.o

login_service: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)


#############################################################################
# A few basic default rules and intrinsic rules

# Start off by over-riding the default build rules with our own intrinsics
.SUFFIXES:
.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -c $(CXXFLAGS) $<


# remove object files and core (if any)
clean:
	$(RM) *.o core

# remove object files, core dump, and executable (if any)
distclean:
	$(MAKE) clean
	$(RM) $(TARGET)

# make the thing again from scratch
again:
	$(MAKE) distclean
	$(MAKE) $(TARGET)
