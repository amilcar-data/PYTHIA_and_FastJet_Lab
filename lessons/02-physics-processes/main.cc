// =============================================================================
// Lesson 02 - Physics processes
//
// Generates five kinds of proton-proton collisions at 13 TeV and measures the
// SAME observables as Lesson 01 for each of them:
//   minimum bias, Z -> mu mu, Higgs (gg -> H), top pairs, and hard dijets.
//
// Output (one file per observable, one column per process):
//   process_eta.dat   (1/N_ev) dN_ch/d(eta)
//   process_pt.dat    (1/N_ev) (1/2 pi pT) d2N_ch/(d eta d pT)
//   process_nch.dat   (1/N_ev) dN_ev/d n_ch
//
// Line-by-line explanation: docs/02-physics-processes.md
// Usage:  ./main [events_per_process]      (default: 10000)
// =============================================================================

#include <algorithm>  // std::upper_bound
#include <cmath>      // std::abs, std::log10, std::pow, M_PI
#include <cstdlib>    // std::atoi
#include <fstream>    // std::ofstream
#include <iomanip>    // std::setw, std::setprecision
#include <iostream>   // std::cout
#include <sstream>    // std::ostringstream (build text in memory)
#include <string>     // std::string
#include <vector>     // std::vector
#include "Pythia8/Pythia.h"
#include "../run_settings.h"

// -----------------------------------------------------------------------------
// Histogram with arbitrary bin edges (new in this lesson).
// Lesson 01 used equal-width bins. The pT spectrum here spans 3 orders of
// magnitude (0.5 to 500 GeV), so we need bins that grow with pT.
// -----------------------------------------------------------------------------
struct Histogram
{
    std::vector<double> edges;     // nBins + 1 increasing edges
    std::vector<double> content;   // sum of weights in each bin

    explicit Histogram(const std::vector<double>& e)
        : edges(e), content(e.size() - 1, 0.0) {}

    int    nBins()      const { return static_cast<int>(content.size()); }
    double width(int i) const { return edges[i + 1] - edges[i]; }

    void fill(double x, double w = 1.0)
    {
        if (x < edges.front() || x >= edges.back()) return;
        // upper_bound: first edge strictly greater than x. One step back = our bin.
        const int bin = static_cast<int>(
            std::upper_bound(edges.begin(), edges.end(), x) - edges.begin()) - 1;
        content[bin] += w;
    }
};

// n equal-width bins between lo and hi
std::vector<double> linearEdges(int n, double lo, double hi)
{
    std::vector<double> e(n + 1);
    for (int i = 0; i <= n; ++i) e[i] = lo + (hi - lo) * i / n;
    return e;
}

// n bins that are equal-width in log10(x): each edge is a fixed factor above the last
std::vector<double> logEdges(int n, double lo, double hi)
{
    std::vector<double> e(n + 1);
    const double logLo = std::log10(lo), logHi = std::log10(hi);
    for (int i = 0; i <= n; ++i) e[i] = std::pow(10.0, logLo + (logHi - logLo) * i / n);
    return e;
}

// Turn summed weights into the published form:
//   value = content / (N_ev * bin width * extra)
std::vector<double> normalize(const Histogram& h, double nEvents, double extra)
{
    std::vector<double> v(h.nBins());
    for (int i = 0; i < h.nBins(); ++i)
        v[i] = h.content[i] / (nEvents * h.width(i) * extra);
    return v;
}

// One text file: x_low x_high x_centre, then one column per process.
void writeColumns(const std::vector<double>& edges,
                  const std::vector<std::vector<double>>& columns,
                  const std::vector<std::string>& labels,
                  const std::string& fileName, const std::string& header)
{
    std::ofstream out(fileName);
    out << "# " << header << "\n# x_low x_high x_centre";
    for (const std::string& label : labels) out << " " << label;
    out << "\n" << std::setprecision(6);
    for (size_t i = 0; i + 1 < edges.size(); ++i)
    {
        out << edges[i] << " " << edges[i + 1] << " "
            << 0.5 * (edges[i] + edges[i + 1]);
        for (const auto& column : columns) out << " " << column[i];
        out << "\n";
    }
}

// A physics process = a name plus the Pythia settings that switch it on.
struct Process
{
    std::string label;
    std::vector<std::string> settings;
};

int main(int argc, char* argv[])
{
    int nEvents;
    if (!readEventCount(argc, argv, 10000, nEvents)) return 1;

    // Same fiducial region as Lesson 01
    const double pTMin  = 0.5;
    const double etaMax = 2.5;
    const int    nChMin = 1;

    // --- The processes --------------------------------------------------------
    const std::vector<Process> processes = {
        {"minbias", {"SoftQCD:inelastic = on"}},
        {"Z",       {"WeakSingleBoson:ffbar2gmZ = on",   // q qbar -> gamma*/Z
                     "PhaseSpace:mHatMin = 80.",         // keep the Z peak region:
                     "PhaseSpace:mHatMax = 100.",        //   80 < mass < 100 GeV
                     "23:onMode = off",                  // switch all Z decays off...
                     "23:onIfAny = 13"}},                // ...except Z -> mu+ mu-
        {"Higgs",   {"HiggsSM:gg2H = on",                // gluon fusion g g -> H
                     "25:m0 = 125."}},                   // Higgs mass in GeV
        {"top",     {"Top:gg2ttbar = on",                // g g -> t tbar
                     "Top:qqbar2ttbar = on"}},           // q qbar -> t tbar
        {"jets",    {"HardQCD:all = on",                 // 2 -> 2 QCD scattering
                     "PhaseSpace:pTHatMin = 200."}},     // outgoing partons pT > 200 GeV
    };

    // Shared binning, so every process is filled and written identically
    const std::vector<double> etaEdges = linearEdges(50, -etaMax, etaMax);
    const std::vector<double> pTEdges  = logEdges(30, pTMin, 500.0);
    // n_ch: 1-wide bins up to 20 (the minimum-bias peak lives there), then 5-wide bins
    // up to 250, because hard processes are broad and need coarser bins to look smooth.
    std::vector<double> nChEdges = linearEdges(20, 0.5, 20.5);
    {
        const std::vector<double> coarse = linearEdges(46, 20.5, 250.5);
        nChEdges.insert(nChEdges.end(), coarse.begin() + 1, coarse.end());  // skip the duplicate 20.5
    }

    std::vector<std::string> labels;
    std::vector<std::vector<double>> etaColumns, pTColumns, nChColumns;

    std::vector<std::string> summaryRows;   // one text line per process, printed at the end

    for (const Process& process : processes)
    {
        // --- Generator for this process --------------------------------------
        // The second argument false = do not print the Pythia banner (we create 5 generators).
        // The first argument is Pythia's default data folder; Pythia finds its own copy if it is not there.
        Pythia8::Pythia pythia("../share/Pythia8/xmldoc", false);
        pythia.readString("Print:quiet = on");     // fewer messages during init and the event loop
        pythia.readString("Beams:idA = 2212");
        pythia.readString("Beams:idB = 2212");
        pythia.readString("Beams:eCM = 13000.");
        for (const std::string& s : process.settings) pythia.readString(s);
        pythia.readString("Random:setSeed = on");
        pythia.readString("Random:seed = 42");
        if (!pythia.init()) return 1;

        Histogram etaHist(etaEdges), pTHist(pTEdges), nChHist(nChEdges);
        long nGenerated = 0, nSelected = 0, nParticles = 0;
        double sumPT = 0.0;

        auto counts = [&](const Pythia8::Particle& p)
        {
            return p.isFinal() && p.isCharged()
                && p.pT() > pTMin && std::abs(p.eta()) < etaMax;
        };

        for (int iEvent = 0; iEvent < nEvents; ++iEvent)
        {
            if (!pythia.next()) continue;
            ++nGenerated;

            // The HARD PROCESS record: the few particles of the core 2 -> n scattering.
            // Print it once per process to see what we switched on.
            if (nGenerated == 1)
            {
                std::cout << "\n--- hard process record, first event of '"
                          << process.label << "' ---\n";
                pythia.process.list();
            }

            int nCh = 0;
            for (int j = 0; j < pythia.event.size(); ++j)
                if (counts(pythia.event[j])) ++nCh;
            if (nCh < nChMin) continue;

            ++nSelected;
            nParticles += nCh;
            nChHist.fill(nCh);

            for (int j = 0; j < pythia.event.size(); ++j)
            {
                const Pythia8::Particle& p = pythia.event[j];
                if (!counts(p)) continue;
                etaHist.fill(p.eta());
                pTHist.fill(p.pT(), 1.0 / (2.0 * M_PI * p.pT()));
                sumPT += p.pT();
            }
        }

        // --- Store the normalized histograms ---------------------------------
        if (nSelected == 0) {
            std::cerr << "No selected events for " << process.label
                      << "; no histograms written. Increase the event count.\n";
            return 1;
        }
        labels.push_back(process.label);
        etaColumns.push_back(normalize(etaHist, nSelected, 1.0));
        pTColumns.push_back(normalize(pTHist, nSelected, 2.0 * etaMax));
        nChColumns.push_back(normalize(nChHist, nSelected, 1.0));

        // --- One line of the summary table -----------------------------------
        // sigmaGen() is in millibarn; 1 mb = 1e9 pb.
        std::ostringstream row;
        row << std::left << std::setw(9) << process.label
            << std::right << std::scientific << std::setprecision(3)
            << std::setw(14) << pythia.info.sigmaGen() * 1e9
            << std::fixed << std::setprecision(3)
            << std::setw(10) << double(nSelected) / nGenerated
            << std::setprecision(2)
            << std::setw(10) << double(nParticles) / nSelected
            << std::setw(12) << double(nParticles) / nSelected / (2.0 * etaMax)
            << std::setw(12) << sumPT / nParticles
            << "   " << pythia.info.name();
        summaryRows.push_back(row.str());
    }

    // --- Print the summary table ----------------------------------------------
    std::cout << "\n" << std::left << std::setw(9) << "process"
              << std::right << std::setw(14) << "sigma [pb]"
              << std::setw(10) << "selected"
              << std::setw(10) << "<n_ch>"
              << std::setw(12) << "<dN/deta>"
              << std::setw(12) << "<pT> [GeV]"
              << "   hard process (last event)\n";
    for (const std::string& row : summaryRows) std::cout << row << "\n";

    // --- Write the three output files ----------------------------------------
    const std::string info = "Pythia8, pp 13 TeV, seed 42, " + std::to_string(nEvents)
                           + " events per process, n_ch>=1, pT>0.5 GeV, |eta|<2.5";
    writeColumns(etaEdges, etaColumns, labels, "process_eta.dat", info);
    writeColumns(pTEdges,  pTColumns,  labels, "process_pt.dat",  info);
    writeColumns(nChEdges, nChColumns, labels, "process_nch.dat", info);

    return 0;
}
