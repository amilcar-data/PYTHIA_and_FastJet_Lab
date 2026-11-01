#ifndef PYTHIA_LAB_EVENT_HISTOGRAM_H
#define PYTHIA_LAB_EVENT_HISTOGRAM_H
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <vector>

// One independent observation is an EVENT, including events with zero entries.
// Accumulate each event's bin total before squaring it for the variance.
struct EventHistogram {
    std::vector<double> edges, sum, sum2, current;
    long events = 0, underflow = 0, overflow = 0;
    explicit EventHistogram(const std::vector<double>& e) : edges(e) {
        if (e.size() < 2) throw std::runtime_error("Need at least two bin edges");
        for (size_t i = 0; i < e.size(); ++i)
            if (!std::isfinite(e[i]) || (i && e[i] <= e[i-1]))
                throw std::runtime_error("Bin edges must be finite and increasing");
        sum.assign(e.size()-1, 0.); sum2 = current = sum;
    }
    void fill(double x, double weight = 1.) {
        if (!std::isfinite(x) || !std::isfinite(weight))
            throw std::runtime_error("Nonfinite histogram entry");
        if (x < edges.front()) { ++underflow; return; }
        if (x >= edges.back()) { ++overflow; return; }
        size_t i = std::upper_bound(edges.begin(), edges.end(), x)-edges.begin()-1;
        current[i] += weight;
    }
    void endEvent() {
        ++events;
        for (size_t i = 0; i < sum.size(); ++i) {
            sum[i] += current[i]; sum2[i] += current[i]*current[i];
            current[i] = 0.;
        }
    }
    double mean(size_t i) const { return sum[i]/events; }
    double error(size_t i) const {
        if (events < 2) throw std::runtime_error("Need >=2 events for a standard error");
        return std::sqrt(std::max(0., sum2[i]-sum[i]*sum[i]/events)
                         / (double(events)*(events-1)));
    }
    void write(const std::string& path, const std::string& info, double extra=1.) const {
        if (events < 2 || extra <= 0.) throw std::runtime_error("Invalid normalization");
        std::ofstream out(path);
        out.exceptions(std::ios::failbit | std::ios::badbit);
        out << "# " << info << "\n# events=" << events << " underflow_entries="
            << underflow << " overflow_entries=" << overflow
            << "\n# low high centre value mc_stat_error\n" << std::setprecision(10);
        for (size_t i=0; i<sum.size(); ++i) {
            double scale=(edges[i+1]-edges[i])*extra;
            out << edges[i] << ' ' << edges[i+1] << ' ' << (edges[i]+edges[i+1])/2
                << ' ' << mean(i)/scale << ' ' << error(i)/scale << '\n';
        }
    }
};

inline std::vector<double> readEdges(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Missing " + path + "; run make first");
    std::vector<double> edges;
    double x;
    while (in >> x) edges.push_back(x);
    if (!in.eof()) throw std::runtime_error("Malformed edge file: " + path);
    return edges;
}
#endif
