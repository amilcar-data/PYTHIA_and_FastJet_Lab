# Build:  make     Run:  make run     Clean:  make clean
# Quick run: make run NEVENTS=20000

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

CXX       ?= g++
CXXFLAGS   = $(PYTHIA_CXXFLAGS)
LDFLAGS    = $(PYTHIA_LIBS)
TARGET     = main
PYTHON    ?= python3
BINS       = data/atlas_eta.edges data/atlas_pt.edges data/atlas_nch.edges
NEVENTS   ?= 100000

all: $(TARGET)

$(BINS) &: data/atlas_eta.csv data/atlas_pt.csv data/atlas_nch.csv prepare_bins.py
	$(PYTHON) prepare_bins.py

$(TARGET): main.cc ../run_settings.h ../common/event_histogram.h atlas_selection.h Makefile
	$(CXX) main.cc -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

run: $(TARGET) $(BINS)
	./$(TARGET) $(NEVENTS)

clean:
	rm -f $(TARGET) simulation_*.dat baseline_*.dat

.PHONY: all run clean
