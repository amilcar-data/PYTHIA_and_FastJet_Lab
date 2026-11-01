# Build:  make     Run:  make run     Clean:  make clean
#
# Needs pythia8-config (from Pythia) and fastjet-config (from FastJet) on your PATH,
# or: make PYTHIA8=<pythia install folder> FASTJET_CONFIG=<path to fastjet-config>

# --- Pythia ------------------------------------------------------------------
# Preferred: ask pythia8-config for the flags. In some installations it cannot find
# its configuration, so we fall back to the PYTHIA8 variable (the install folder),
# set in docs/installation.md:   export PYTHIA8=$HOME/software/pythia-install
PYTHIA8_CONFIG ?= pythia8-config
PYTHIA_CXXFLAGS := $(shell $(PYTHIA8_CONFIG) --cxxflags 2>/dev/null)
PYTHIA_LIBS     := $(shell $(PYTHIA8_CONFIG) --libs 2>/dev/null)
ifneq ($(MAKECMDGOALS),clean)
  ifeq ($(findstring -lpythia8,$(PYTHIA_LIBS)),)
    ifdef PYTHIA8
      PYTHIA_CXXFLAGS = -O2 -std=c++11 -pthread -I$(PYTHIA8)/include
      PYTHIA_LIBS     = -L$(PYTHIA8)/lib -Wl,-rpath,$(PYTHIA8)/lib -lpythia8 -ldl
    else
      $(error Cannot use pythia8-config. Set PYTHIA8 to your Pythia install folder, e.g. export PYTHIA8=$$HOME/software/pythia-install. See docs/installation.md)
    endif
  endif
endif

# --- FastJet -----------------------------------------------------------------
FASTJET_CONFIG ?= fastjet-config
FASTJET_CXXFLAGS := $(shell $(FASTJET_CONFIG) --cxxflags 2>/dev/null)
FASTJET_LIBS     := $(shell $(FASTJET_CONFIG) --libs 2>/dev/null)
ifneq ($(MAKECMDGOALS),clean)
  ifeq ($(findstring -lfastjet,$(FASTJET_LIBS)),)
    $(error Cannot use fastjet-config. Source scripts/env.sh or set FASTJET_CONFIG to its full path)
  endif
endif
FASTJET_LIBS += -Wl,-rpath,$(shell $(FASTJET_CONFIG) --prefix 2>/dev/null)/lib

CXX       ?= g++
CXXFLAGS   = $(PYTHIA_CXXFLAGS) $(FASTJET_CXXFLAGS)
LDFLAGS    = $(PYTHIA_LIBS) $(FASTJET_LIBS)
TARGET     = main
NEVENTS   ?= 5000

all: $(TARGET)

$(TARGET): main.cc ../run_settings.h ../common/event_histogram.h Makefile
	$(CXX) main.cc -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET) $(NEVENTS)

clean:
	rm -f $(TARGET) jets_*.dat jets.csv jet_events.csv

.PHONY: all run clean
