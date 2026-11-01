// Lesson 03: many collisions -> jet distributions, with event-level errors.
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "Pythia8/Pythia.h"
#include "fastjet/ClusterSequence.hh"
#include "../run_settings.h"
#include "../common/event_histogram.h"

int main(int argc, char* argv[]) try {
    int requested;
    if (!readEventCount(argc, argv, 5000, requested)) return 1;
    if (requested<2) { std::cerr << "Need at least 2 events for uncertainties.\n"; return 1; }
    double radius = 0.4;
    if (const char* value = std::getenv("JET_RADIUS")) {
        std::string text(value); size_t used=0;
        radius=std::stod(text,&used);
        if (used!=text.size() || !std::isfinite(radius) || radius<=0 || radius>2)
            throw std::runtime_error("JET_RADIUS must be in (0,2]");
    }
    std::ofstream rows("jets.csv"), events("jet_events.csv");
    rows.exceptions(std::ios::failbit | std::ios::badbit);
    events.exceptions(std::ios::failbit | std::ios::badbit);
    rows << "sample,event,rank,pt_GeV,mass_GeV,rapidity,eta,constituents\n" << std::setprecision(10);
    events << "sample,event,njets,leading_pt_GeV\n" << std::setprecision(10);
    const fastjet::JetDefinition definition(fastjet::antikt_algorithm,radius,fastjet::E_scheme);
    std::vector<double> multEdges;
    for (int i=0; i<=16; ++i) multEdges.push_back(i-0.5);

    for (const std::string sample : {"qcd", "top"}) {
        Pythia8::Pythia pythia;
        pythia.readString("Beams:idA = 2212");
        pythia.readString("Beams:idB = 2212");
        pythia.readString("Beams:eCM = 13000.");
        pythia.readString("Random:setSeed = on");
        pythia.readString("Random:seed = 42");
        pythia.readString("ParticleDecays:limitTau0 = on");
        pythia.readString("ParticleDecays:tau0Max = 10.");
        pythia.readString("Next:numberShowEvent = 0");
        if (sample == "qcd") {
            pythia.readString("HardQCD:all = on");
            pythia.readString("PhaseSpace:pTHatMin = 200.");
        } else {
            pythia.readString("Top:gg2ttbar = on");
            pythia.readString("Top:qqbar2ttbar = on");
        }
        if (!pythia.init()) return 1;
        EventHistogram pt({30,40,50,70,100,150,200,300,450,650,1000,1500,2500});
        EventHistogram mass({0,5,10,15,20,30,40,60,80,120,180,250,400});
        EventHistogram multiplicity(multEdges);
        long generated=0, jetCount=0, zeroJets=0;
        for (int ev=0; ev<requested; ++ev) {
            if (!pythia.next()) continue;
            ++generated;
            std::vector<fastjet::PseudoJet> inputs;
            for (int i=1; i<pythia.event.size(); ++i) {
                const auto& p=pythia.event[i];
                // Particle-level definition: visible final particles, excluding muons.
                // No input eta cut; impose acceptance AFTER clustering.
                if (!p.isFinal() || !p.isVisible() || p.idAbs()==13) continue;
                inputs.emplace_back(p.px(),p.py(),p.pz(),p.e());
                inputs.back().set_user_index(i);
            }
            // Keep the ClusterSequence alive while accessing jet constituents.
            fastjet::ClusterSequence sequence(inputs,definition);
            const auto jets=fastjet::sorted_by_pt(sequence.inclusive_jets(30.));
            int accepted=0;
            double leading=0.;
            for (const auto& jet : jets) {
                if (jet.pt()<=30. || std::abs(jet.rap())>=2.5) continue;
                if (accepted==0) leading=jet.pt();
                // Floating-point roundoff can produce a tiny negative mass squared.
                const double m=std::sqrt(std::max(0.,jet.m2()));
                pt.fill(jet.pt()); mass.fill(m);
                rows << sample << ',' << ev << ',' << accepted << ',' << jet.pt()
                     << ',' << m << ',' << jet.rap() << ',' << jet.eta()
                     << ',' << jet.constituents().size() << '\n';
                ++accepted;
            }
            jetCount += accepted;
            if (!accepted) ++zeroJets;
            multiplicity.fill(accepted);
            // Include zero-jet events in EVERY per-event denominator.
            pt.endEvent(); mass.endEvent(); multiplicity.endEvent();
            events << sample << ',' << ev << ',' << accepted << ',' << leading << '\n';
        }
        if (generated<2) { std::cerr << "Too few successful events\n"; return 1; }
        const std::string info="PYTHIA "+std::to_string(PYTHIA_VERSION)+", "
            +fastjet::fastjet_version_string()+", "+sample
            +", pp 13 TeV, seed=42, Tune:pp="+std::to_string(pythia.settings.mode("Tune:pp"))
            +", anti-kt R="+std::to_string(radius)+" E-scheme, jet pt>30 GeV |y|<2.5, stable ctau>=10 mm; no muons/neutrinos"
            +", attempts="+std::to_string(requested)+", failed="+std::to_string(requested-generated);
        pt.write("jets_"+sample+"_pt.dat",info);
        mass.write("jets_"+sample+"_mass.dat",info);
        multiplicity.write("jets_"+sample+"_multiplicity.dat",info);
        std::cout << "\nJET SUMMARY " << sample << ": attempted=" << requested
                  << " generated=" << generated << " failed=" << requested-generated
                  << " accepted_jets=" << jetCount << " zero_jet_events=" << zeroJets
                  << " mean_njets=" << double(jetCount)/generated
                  << " sigma_pb=" << pythia.info.sigmaGen()*1e9 << '\n'
                  << "Overflows (pt,mass,multiplicity): " << pt.overflow << ','
                  << mass.overflow << ',' << multiplicity.overflow << '\n';
        pythia.stat();
    }
    return 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
