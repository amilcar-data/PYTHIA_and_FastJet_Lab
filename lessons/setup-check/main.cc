// =============================================================================
// Setup check: is Pythia + FastJet installed and working?
//
// Generates ONE dijet event with Pythia, groups the final-state particles into
// jets with FastJet (anti-kt, R = 0.4) and prints the three hardest jets.
//
// If this program compiles, links and prints jets, both libraries work.
// Explanation: docs/installation.md (and docs/what-are-pythia-and-fastjet.md)
// =============================================================================

#include <iostream>
#include <vector>
#include "Pythia8/Pythia.h"          // the event generator
#include "fastjet/ClusterSequence.hh" // the jet-finding library

int main()
{
    // --- Pythia: one hard QCD scattering at 13 TeV ----------------------------
    Pythia8::Pythia pythia("../share/Pythia8/xmldoc", false);   // false: no banner
    pythia.readString("Print:quiet = on");
    pythia.readString("Beams:idA = 2212");
    pythia.readString("Beams:idB = 2212");
    pythia.readString("Beams:eCM = 13000.");
    pythia.readString("HardQCD:all = on");
    pythia.readString("PhaseSpace:pTHatMin = 200.");   // outgoing partons with pT > 200 GeV
    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = 42");
    if (!pythia.init()) { std::cout << "Pythia failed to initialize\n"; return 1; }
    if (!pythia.next()) { std::cout << "Pythia failed to generate an event\n"; return 1; }

    std::cout << "Pythia version " << pythia.settings.parm("Pythia:versionNumber") << "\n";
    std::cout << fastjet::fastjet_version_string() << "\n";

    // --- Hand the particles from Pythia to FastJet ----------------------------
    // FastJet only needs four-momenta (px, py, pz, E). We use every final-state
    // particle that a detector could see (neutrinos are excluded by isVisible()).
    std::vector<fastjet::PseudoJet> particles;
    for (int i = 0; i < pythia.event.size(); ++i)
    {
        const Pythia8::Particle& p = pythia.event[i];
        if (!p.isFinal() || !p.isVisible()) continue;
        particles.emplace_back(p.px(), p.py(), p.pz(), p.e());
    }

    // --- Cluster into jets ----------------------------------------------------
    fastjet::JetDefinition jetDef(fastjet::antikt_algorithm, 0.4);   // anti-kt, R = 0.4
    // FastJet prints a short banner (with the citation request) the first time it is used.
    // To silence it: fastjet::ClusterSequence::set_fastjet_banner_stream(nullptr);
    fastjet::ClusterSequence clusterSequence(particles, jetDef);
    std::vector<fastjet::PseudoJet> jets =
        fastjet::sorted_by_pt(clusterSequence.inclusive_jets(20.0)); // keep jets with pT > 20 GeV

    std::cout << "Particles given to FastJet: " << particles.size() << "\n";
    std::cout << "Jets found (pT > 20 GeV)  : " << jets.size() << "\n";
    std::cout << "Hardest jets:\n";
    for (size_t i = 0; i < jets.size() && i < 3; ++i)
        std::cout << "  jet " << i << ": pT = " << jets[i].pt()
                  << " GeV, eta = " << jets[i].eta()
                  << ", phi = " << jets[i].phi_std()
                  << ", constituents = " << jets[i].constituents().size() << "\n";

    if (jets.empty()) {
        std::cerr << "Setup check failed: no jets above 20 GeV.\n";
        return 1;
    }
    std::cout << "\nSetup check passed: Pythia and FastJet both work.\n";
    return 0;
}
