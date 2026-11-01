// =============================================================================
// Lesson 00 - First collision
//
// Simulates 10 proton-proton collisions with Pythia 8 and prints every entry
// of each event record.
//
// Line-by-line explanation: docs/00-first-collision.md
// =============================================================================

#include <iostream>   // std::cout, std::endl
#include <iomanip>    // std::setw (aligned columns)
#include "Pythia8/Pythia.h"

int main()
{
    // How many collisions (events) to generate.
    const int nEvents = 10;

    // The generator object: holds settings, particle data and the event record.
    Pythia8::Pythia pythia;

    // --- Beams ---------------------------------------------------------------
    pythia.readString("Beams:idA = 2212");     // beam A: proton (PDG code 2212)
    pythia.readString("Beams:idB = 2212");     // beam B: proton
    pythia.readString("Beams:eCM = 14000.");   // collision energy in GeV (14 TeV)

    // --- Physics -------------------------------------------------------------
    pythia.readString("SoftQCD:nonDiffractive = on");  // typical inelastic collision

    // --- Reproducibility -----------------------------------------------------
    pythia.readString("Random:setSeed = on");  // use our own random seed...
    pythia.readString("Random:seed = 42");     // ...so every run gives the same events

    // Prepare the generator. Returns false if something is wrong.
    if (!pythia.init()) return 1;

    // --- Event loop ----------------------------------------------------------
    for (int iEvent = 0; iEvent < nEvents; ++iEvent)
    {
        // Generate one collision. Skip the event if generation failed.
        if (!pythia.next()) continue;

        // Number of entries in the event record (not only final particles!).
        const int nEntries = pythia.event.size();

        std::cout << "Event: " << iEvent << std::endl;
        std::cout << "Event size: " << nEntries << std::endl;
        std::cout << std::setw(6)  << "index"
                  << std::setw(8)  << "id"
                  << std::setw(14) << "name"
                  << std::setw(8)  << "status"
                  << std::setw(14) << "mass[GeV]"
                  << std::setw(14) << "|p|[GeV]" << "\n";

        // Loop over every entry of the event record.
        for (int j = 0; j < nEntries; ++j)
        {
            const Pythia8::Particle& particle = pythia.event[j];

            std::cout << std::setw(6)  << j
                      << std::setw(8)  << particle.id()
                      << std::setw(14) << particle.name()
                      << std::setw(8)  << particle.status()
                      << std::setw(14) << particle.m()
                      << std::setw(14) << particle.pAbs() << "\n";
        }
    }

    return 0;
}
