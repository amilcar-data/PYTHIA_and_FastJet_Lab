#ifndef PYTHIA_LAB_ATLAS_SELECTION_H
#define PYTHIA_LAB_ATLAS_SELECTION_H
#include "Pythia8/Pythia.h"
#include <cmath>
#include <stdexcept>
#include <vector>

// Pythia tau0 is c*tau in mm, NOT time in ps or the sampled decay distance.
constexpr double ATLAS_PARENT_CTAU = 30. * 0.299792458;
constexpr double ATLAS_PRIMARY_CTAU = 300. * 0.299792458;

class AtlasSelection {
    const Pythia8::Event& event;
    std::vector<int> secondary; // -1 unvisited, -2 active, 0 primary ancestry, 1 secondary
    bool hasSecondaryAncestor(int i) {
        if (i <= 0) return false;
        if (i >= event.size()) throw std::runtime_error("Invalid mother index");
        if (secondary[i] == -2) throw std::runtime_error("Cycle in event ancestry");
        if (secondary[i] >= 0) return secondary[i];
        secondary[i] = -2;
        const auto& p = event[i];
        // 91..97 are decay products. 99 (momentum reshuffling) is NOT a decay.
        const bool decay = p.statusAbs() >= 91 && p.statusAbs() <= 97;
        bool excluded = false;
        for (int mother : p.motherList()) {
            if (mother <= 0) continue;
            if (mother >= event.size()) throw std::runtime_error("Invalid mother index");
            if ((decay && event[mother].tau0() >= ATLAS_PARENT_CTAU)
                || hasSecondaryAncestor(mother)) { excluded = true; break; }
        }
        secondary[i] = excluded;
        return excluded;
    }
public:
    explicit AtlasSelection(const Pythia8::Event& e) : event(e), secondary(e.size(), -1) {}
    bool accepts(int i) {
        const auto& p = event[i];
        // Stable species such as electrons and protons have tau0=0 and no decays.
        const bool longLived = p.tau0() > ATLAS_PRIMARY_CTAU
            || (p.tau0() == 0. && !p.canDecay());
        return p.isFinal() && p.isCharged() && longLived
            && p.pT() >= 0.5 && std::abs(p.eta()) < 2.5
            && !hasSecondaryAncestor(i);
    }
};
#endif
