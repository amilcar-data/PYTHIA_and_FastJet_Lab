// Focused regression checks for the physics definition and event statistics.
#include <cassert>
#include <cmath>
#include <iostream>
#include "../lessons/01-compare-to-data/atlas_selection.h"
#include "../lessons/common/event_histogram.h"
#include "fastjet/ClusterSequence.hh"

int main() {
    Pythia8::Pythia pythia("../share/Pythia8/xmldoc",false);
    Pythia8::Event event;
    event.init("selection fixtures", &pythia.particleData); event.reset();
    auto add = [&](int id, int status, int mother, double pt) {
        double m=pythia.particleData.m0(id);
        return event.append(id,status,mother,0,0,0,0,0,pt,0.,0.,std::sqrt(pt*pt+m*m),m);
    };
    int pion=add(211,81,0,1.), proton=add(2212,81,0,1.), electron=add(11,81,0,1.);
    int ks=add(310,-81,0,1.), ksPi=add(211,91,ks,1.);
    int lambda=add(3122,-81,0,1.), lambdaProton=add(2212,91,lambda,1.);
    int rho=add(113,-81,0,1.), rhoPi=add(211,91,rho,1.);
    int b=add(511,-81,0,1.), bPi=add(211,91,b,1.);
    int middle=add(113,-91,ks,1.), indirect=add(211,91,middle,1.);
    int sigma=add(3112,81,0,1.), photon=add(22,81,0,1.);
    int oldMuon=add(13,-23,0,1.), radiatingMuon=add(13,51,oldMuon,1.);
    int decayedMuon=add(13,-81,0,1.), muonElectron=add(11,91,decayedMuon,1.);
    int boundary=add(211,81,0,0.5), soft=add(211,81,0,0.49);
    int oldPion=add(211,-81,0,1.);
    AtlasSelection select(event);
    for (int i : {pion,proton,electron,rhoPi,bPi,radiatingMuon,boundary}) assert(select.accepts(i));
    for (int i : {ksPi,lambdaProton,indirect,sigma,photon,muonElectron,soft,oldPion}) assert(!select.accepts(i));
    std::cout << "Primary selection: direct, short decay, secondary cascade, stable species, FSR and pT boundary pass\n";

    EventHistogram hist({0.,1.,2.});
    hist.fill(.5); hist.fill(.5); hist.endEvent(); hist.endEvent();
    assert(std::abs(hist.mean(0)-1.)<1e-12 && std::abs(hist.error(0)-1.)<1e-12);
    assert(hist.mean(1)==0. && hist.error(1)==0.);
    EventHistogram weighted({0.,1.});
    weighted.fill(.5,1.); weighted.fill(.5,2.); weighted.endEvent();
    weighted.fill(.5,1.); weighted.endEvent();
    assert(std::abs(weighted.mean(0)-2.)<1e-12 && std::abs(weighted.error(0)-1.)<1e-12);
    EventHistogram boundaryHist({0.,1.,2.});
    boundaryHist.fill(1.); boundaryHist.fill(2.); boundaryHist.fill(-1.); boundaryHist.endEvent();
    assert(boundaryHist.sum[0]==0 && boundaryHist.sum[1]==1);
    assert(boundaryHist.underflow==1 && boundaryHist.overflow==1);
    std::cout << "Statistics: whole-event correlation, weights, zeros, bin edges and overflow pass\n";

    std::vector<fastjet::PseudoJet> inputs;
    inputs.emplace_back(40.,0.,0.,40.);
    inputs.emplace_back(40.*std::cos(.1),40.*std::sin(.1),0.,40.);
    fastjet::JetDefinition def(fastjet::antikt_algorithm,.4,fastjet::E_scheme);
    fastjet::ClusterSequence cluster(inputs,def);
    auto jets=cluster.inclusive_jets(30.);
    assert(jets.size()==1 && jets[0].constituents().size()==2);
    assert(std::abs(jets[0].e()-80.)<1e-10);
    assert(std::abs(jets[0].m2()-3200.*(1.-std::cos(.1)))<1e-9);
    std::cout << "FastJet: constituent recombination and nonzero mass from massless particles pass\n";
}
