// Lesson 01: ATLAS primary charged particles, published bins, event-level errors.
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include "Pythia8/Pythia.h"
#include "../run_settings.h"
#include "../common/event_histogram.h"
#include "atlas_selection.h"

int main(int argc, char* argv[]) try {
    int requested;
    if (!readEventCount(argc, argv, 100000, requested)) return 1;
    if (requested < 2) { std::cerr << "Need at least 2 events for uncertainties.\n"; return 1; }
    EventHistogram eta(readEdges("data/atlas_eta.edges"));
    EventHistogram pt(readEdges("data/atlas_pt.edges"));
    EventHistogram nch(readEdges("data/atlas_nch.edges"));
    EventHistogram oldEta(eta.edges), oldPt(pt.edges), oldNch(nch.edges);

    Pythia8::Pythia pythia;
    pythia.readString("Beams:idA = 2212");
    pythia.readString("Beams:idB = 2212");
    pythia.readString("Beams:eCM = 13000.");
    pythia.readString("SoftQCD:inelastic = on");
    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = 42");
    // Keep standard decays so the old and new selections see the SAME events.
    // Long-lived muons, charged pions and charged kaons remain stable by default.
    pythia.readString("ParticleDecays:limitTau0 = off");
    pythia.readString("Next:numberShowEvent = 0");
    if (!pythia.init()) return 1;

    long generated=0, selected=0, oldSelected=0, particles=0, oldParticles=0;
    auto fill = [&](const std::vector<int>& indices, EventHistogram& hEta,
                    EventHistogram& hPt, EventHistogram& hNch) {
        hNch.fill(indices.size());
        for (int i : indices) {
            const auto& p=pythia.event[i];
            hEta.fill(p.eta());
            hPt.fill(p.pT(), 1./(2.*std::acos(-1.)*p.pT()));
        }
        hEta.endEvent(); hPt.endEvent(); hNch.endEvent();
    };
    for (int ev=0; ev<requested; ++ev) {
        if (!pythia.next()) continue;
        ++generated;
        AtlasSelection selection(pythia.event);
        std::vector<int> primary, inclusive;
        for (int i=1; i<pythia.event.size(); ++i) {
            const auto& p=pythia.event[i];
            if (p.isFinal() && p.isCharged() && p.pT()>=0.5 && std::abs(p.eta())<2.5)
                inclusive.push_back(i);
            if (selection.accepts(i)) primary.push_back(i);
        }
        if (!inclusive.empty()) {
            ++oldSelected; oldParticles += inclusive.size();
            fill(inclusive, oldEta, oldPt, oldNch);
        }
        if (primary.empty()) continue;
        ++selected; particles += primary.size();
        fill(primary, eta, pt, nch);
    }
    if (selected<2 || oldSelected<2) {
        std::cerr << "Fewer than 2 selected events; no histograms written.\n"; return 1;
    }
    const std::string info="PYTHIA " + std::to_string(PYTHIA_VERSION)
        + ", pp 13 TeV, SoftQCD:inelastic, Tune:pp=" + std::to_string(pythia.settings.mode("Tune:pp"))
        + ", seed=42, attempts=" + std::to_string(requested)
        + ", generated=" + std::to_string(generated);
    eta.write("simulation_eta.dat",info+", ATLAS primary, Tables 3-5 v1");
    pt.write("simulation_pt.dat",info+", ATLAS primary, Tables 3-5 v1",5.);
    nch.write("simulation_nch.dat",info+", ATLAS primary, Tables 3-5 v1");
    oldEta.write("baseline_eta.dat",info+", old inclusive selection; same events, same bins");
    oldPt.write("baseline_pt.dat",info+", old inclusive selection; same events, same bins",5.);
    oldNch.write("baseline_nch.dat",info+", old inclusive selection; same events, same bins");
    std::cout << "\n==== Selection comparison (same generated events) ====\n"
              << "Attempts / generated / failed: " << requested << " / " << generated
              << " / " << requested-generated << '\n'
              << "Old inclusive: selected=" << oldSelected << ", particles=" << oldParticles
              << ", mean n_ch=" << double(oldParticles)/oldSelected << '\n'
              << "ATLAS primary: selected=" << selected << ", particles=" << particles
              << ", mean n_ch=" << double(particles)/selected
              << ", mean dN/deta=" << double(particles)/selected/5. << '\n'
              << "Excluded particles: " << oldParticles-particles << '\n'
              << "Out-of-range entries (eta / pt / nch): " << eta.underflow+eta.overflow
              << " / " << pt.underflow+pt.overflow << " / " << nch.underflow+nch.overflow << '\n';
    pythia.stat();
    return 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
