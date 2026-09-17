#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>
#include <unordered_set>

#include "boost/thread/mutex.hpp"
#include "DataSet.h"

struct MarginKey {
    count_t S;
    count_t T;

    bool operator==(const MarginKey& other) const {
        return S == other.S && T == other.T;
    }
};
struct MarginKeyHash {
    std::size_t operator()(const MarginKey& key) const {
        std::size_t h1 = std::hash<count_t>{}(key.S);
        std::size_t h2 = std::hash<count_t>{}(key.T);
        return h1 ^ (h2 << 1);
    }
};
struct AnalyticCalibrationCell {
    std::unordered_map<MarginKey, std::uint64_t, MarginKeyHash> marginCounts;
    mutable std::unordered_map<MarginKey, std::uint64_t, MarginKeyHash> marginCountsMisses;
    std::uint64_t totalCandidates = 0;
};

class AnalyticSizeConditionalCalibration {
public:
    enum Mode { OFF = 0, BUILD, APPLY };
private:
    count_t _C;
    std::size_t _nSpatialBins;
    std::size_t _nTemporalBins;
    std::vector<AnalyticCalibrationCell> _cells;
    Mode _mode;
    bool _finalized;
    mutable boost::mutex _mutex;

    std::size_t clampBin(double q, std::size_t nBins) const {
        if (q <= 0.0) return 0;
        if (q >= 1.0) return nBins - 1;
        std::size_t idx = static_cast<std::size_t>(std::floor(q * nBins));
        return std::min(idx, nBins - 1);
    }
    std::size_t index(double qS, double qT) const {
        const std::size_t s = clampBin(qS, _nSpatialBins);
        const std::size_t t = clampBin(qT, _nTemporalBins);
        return s * _nTemporalBins + t;
    }

public:
    AnalyticSizeConditionalCalibration(
        count_t C, std::size_t nSpatialBins = 20, std::size_t nTemporalBins = 20
    ) : _C(C), _nSpatialBins(nSpatialBins), _nTemporalBins(nTemporalBins), _cells(nSpatialBins* nTemporalBins),
        _mode(OFF), _finalized(false) {}

    void reset() {
        for (auto& cell : _cells) {
            cell.marginCounts.clear();
            cell.marginCountsMisses.clear();
        }
        _finalized = false;
        _mode = OFF;
    }

    void setMode(Mode mode) { _mode = mode; }
    Mode mode() const { return _mode; }
    bool finalized() const { return _finalized; }
    void finalize() { _finalized = true; }

    void observeCandidate(count_t S, count_t T) {
        const double qS = static_cast<double>(S) / static_cast<double>(_C);
        const double qT = static_cast<double>(T) / static_cast<double>(_C);
        auto& cell = _cells[index(qS, qT)];
        ++cell.marginCounts[{S, T}];
        ++cell.totalCandidates;
    }

    double calibrate(count_t observedS, count_t observedT, double observedRawScore, const HypergeometricProbabilityLookup& lookup) const {
        const double qS = static_cast<double>(observedS)/static_cast<double>(_C);
        const double qT = static_cast<double>(observedT)/static_cast<double>(_C);
        const auto& cell = _cells[index(qS, qT)];

        // Nyall is trying to figure out how to deal with misses discovered during simulations.
        if (cell.totalCandidates == 0) {
            //throw prg_error("Empty analytic calibration cell.", "AnalyticSizeConditionalCalibration::calibrate");
            ++cell.marginCountsMisses[{observedS, observedT}];
            return -std::numeric_limits<double>::max();
        }

        long double weightedTail = 0.0L;
        for (const auto& entry : cell.marginCounts) {
            const MarginKey& margin = entry.first;
            const std::uint64_t multiplicity = entry.second;
            const double probability = probabilityRawScoreAtLeast(margin.S, margin.T, observedRawScore, lookup);
            weightedTail += static_cast<long double>(multiplicity) * static_cast<long double>(probability);
        }

        const long double pLoc = weightedTail / static_cast<long double>(cell.totalCandidates);
        const double bounded = std::max(static_cast<double>(pLoc), std::numeric_limits<double>::min());

        return -std::log(bounded);
    }

    /** Hypergeometric point mass P{X = x} for X ~ HG(C,S,T), where C is the total cases, S the cases in the
        spatial region, T the cases in the temporal interval and x the cases in both:
            P{X = x} = choose(S,x) * choose(C-S,T-x) / choose(C,T)
        Evaluated in log space to avoid overflow of the individual binomial coefficients. Returns zero for x
        outside the support [max(0,S+T-C), min(S,T)]. */
    long double hypergeometricPMF(count_t C, count_t S, count_t T, count_t x) const {
        if (x < std::max<count_t>(0, S + T - C) || x > std::min(S, T))
            return 0.0L;
        auto logChoose = [](count_t n, count_t k) -> long double {
            return std::lgamma(static_cast<long double>(n) + 1.0L)
                 - std::lgamma(static_cast<long double>(k) + 1.0L)
                 - std::lgamma(static_cast<long double>(n - k) + 1.0L);
        };
        return std::exp(logChoose(S, x) + logChoose(C - S, T - x) - logChoose(C, T));
    }

    // A simple correctness-first implementation.
    double probabilityRawScoreAtLeast(count_t S, count_t T, double scoreThreshold, const HypergeometricProbabilityLookup& lookup) const {
        const count_t xmin = std::max<count_t>(0, S + T - _C);
        const count_t xmax = std::min(S, T);
        long double probability = 0.0L;
        double rawScore;

        for (count_t x = xmin; x <= xmax; ++x) {
            rawScore = -std::log(-lookup.getProbabilityFor_Checked(T, S, x)); //rawHypergeometricScore(C, S, T, x);
            if (rawScore >= scoreThreshold) {
                probability += hypergeometricPMF(_C, S, T, x);
            }
        }
        return static_cast<double>(probability);
    }

    void print(FILE* f) const {
        std::stringstream out;
        out << std::endl << "AnalyticSizeConditionalCalibration" << std::endl;
        out << "_nSpatialBins=" << _nSpatialBins << ",_nTemporalBins=" << _nTemporalBins << std::endl;
        std::size_t memoryCost = sizeof(*this) + _cells.capacity() * sizeof(AnalyticCalibrationCell);
        out << "_cells.size()=" << _cells.size() << std::endl;
        size_t nonzero = 0;
        for (size_t t = 0; t < _cells.size(); ++t) {
            auto& cell = _cells[t];
            if (!cell.totalCandidates) {
                ++nonzero;
                if (!cell.marginCountsMisses.size())
                    continue;
            }
            memoryCost += sizeof(cell.marginCounts) +
                cell.marginCounts.bucket_count() * sizeof(void*) +
                cell.marginCounts.size() * (sizeof(std::unordered_map<MarginKey, std::uint64_t, MarginKeyHash>::value_type) + 2U * sizeof(void*));
            out << "cell[" << t << "],size=" << cell.marginCounts.size() << ",totalCandidates=" << cell.totalCandidates << std::endl;
            if (cell.marginCounts.size()) {
                out << "marginCounts: ";
                for (auto& mc : cell.marginCounts)
                    out << "S=" << mc.first.S << ",T=" << mc.first.T << ",count=" << mc.second << " :: ";
                out << std::endl;
            }
            if (cell.marginCountsMisses.size()) {
                out << "marginCountsMisses: ";
                for (auto& mc : cell.marginCountsMisses)
                    out << "S=" << mc.first.S << ",T=" << mc.first.T << ",count=" << mc.second << " :: ";
                out << std::endl;
            }
        }
        out << "cells with margins=" << (_cells.size() - nonzero) << ", without=" << nonzero << std::endl;
        out << "memory cost=" << (static_cast<double>(memoryCost) / 1000000.0) << " MB" << std::endl;
        std::string temp_str = out.str();
        std::cout << temp_str << std::endl;
        if (f) fwrite(temp_str.data(), sizeof(char), temp_str.size(), f);
    }
};


class SizeConditionalCalibration {
public:
    enum Mode { OFF = 0, BUILD, APPLY };

    struct PairHash {
        std::size_t operator()(const std::pair<double, double>& p) const {
            // Hash both elements individually
            std::size_t h1 = std::hash<double>{}(p.first);
            std::size_t h2 = std::hash<double>{}(p.second);
            // Combine them using a standard bit-shifting formula
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

    struct Cell {
        std::vector<double> rawScores;
        std::unordered_set<std::pair<double, double>, PairHash> proportions;
    };

private:
    std::size_t _nSpatialBins;
    std::size_t _nTemporalBins;
    std::vector<Cell> _cells;
    Mode _mode;
    bool _finalized;
    std::size_t _minimumCellSize;
    mutable boost::mutex _mutex;

    std::size_t clampBin(double q, std::size_t nBins) const {
        if (q <= 0.0) return 0;
        if (q >= 1.0) return nBins - 1;
        std::size_t idx = static_cast<std::size_t>(std::floor(q * nBins));
        return std::min(idx, nBins - 1);
    }

    std::size_t index(double qS, double qT) const {
        const std::size_t s = clampBin(qS, _nSpatialBins);
        const std::size_t t = clampBin(qT, _nTemporalBins);
        return s * _nTemporalBins + t;
    }

public:
    SizeConditionalCalibration(std::size_t nSpatialBins = 20, std::size_t nTemporalBins = 20, std::size_t minimumCellSize = 100) :
        _nSpatialBins(nSpatialBins), _nTemporalBins(nTemporalBins), _cells(nSpatialBins* nTemporalBins),
        _mode(OFF), _finalized(false), _minimumCellSize(minimumCellSize) {
    }

    void setMode(Mode mode) { _mode = mode; }
    Mode mode() const { return _mode; }
    bool finalized() const { return _finalized; }

    void reset() {
        for (auto& cell : _cells)
            cell.rawScores.clear();
        _finalized = false;
        _mode = OFF;
    }

    // Called for EVERY candidate in the Bcal null simulations.
    void observe(double qS, double qT, double rawScore) {
        if (_mode != BUILD) return;
        if (!std::isfinite(rawScore)) return;

        boost::mutex::scoped_lock lock(_mutex);
        auto& cell = _cells[index(qS, qT)];
        cell.rawScores.push_back(rawScore);
        cell.proportions.insert(std::make_pair(qS, qT));
    }

    // Called once after all Bcal simulations are complete.
    void finalize() {
        for (auto& cell : _cells)
            std::sort(cell.rawScores.begin(), cell.rawScores.end());
        _finalized = true;
    }

    // Empirical size-conditional upper-tail probability:
    // p_loc = (1 + #{null rawScore >= observed rawScore}) / (N + 1)
    double localPValue(double qS, double qT, double rawScore) const {
        if (!_finalized) throw std::runtime_error("Size calibration has not been finalized.");

        const Cell& cell = _cells[index(qS, qT)];

        // Prototype behaviour. Production code should replace this with
        // adaptive neighbouring-bin expansion or smoothing.
        if (cell.rawScores.size() < _minimumCellSize)
            return 1.0;

        const auto it = std::lower_bound(cell.rawScores.begin(), cell.rawScores.end(), rawScore);
        const std::size_t nGE = static_cast<std::size_t>(cell.rawScores.end() - it);
        const std::size_t N = cell.rawScores.size();

        return static_cast<double>(1 + nGE) / static_cast<double>(N + 1);
    }

    double score(double qS, double qT, double rawScore) const {
        if (_mode == OFF) return rawScore;

        double p = localPValue(qS, qT, rawScore);
        return p == 1 ? 0.0 : -std::log(p);
    }

    void print(FILE * f) const {
        std::stringstream out;
        out << std::endl << "SizeConditionalCalibration" << std::endl;
        out << "_minimumCellSize=" << _minimumCellSize << ", _nSpatialBins=" << _nSpatialBins << ", _nTemporalBins=" << _nTemporalBins << std::endl;
        size_t nonzero = 0;
        std::size_t memoryCost = sizeof(*this) + _cells.capacity() * sizeof(Cell);
        out << "_cells.size()=" << _cells.size() << std::endl;
        for (size_t t = 0; t < _cells.size(); ++t) {
            auto& cell = _cells[t];
            if (cell.rawScores.empty()) ++nonzero;
            memoryCost += cell.rawScores.capacity() * sizeof(double);
            if (cell.rawScores.empty()) continue;
            out << "cell[" << t << "], size=" << cell.rawScores.size() <<
                ", lowest=" << cell.rawScores.front() << ", highest=" << cell.rawScores.back() << std::endl;
            out << "proportions(qS,qT): ";
            for (auto p : cell.proportions) {
                out << p.first << ": " << p.second << ", ";
            }
            std::map<double, unsigned int> frequency;
            for (auto entry : cell.rawScores) {
                frequency[entry]++;
            }
            out << std::endl << "freq: ";
            for (auto pair : frequency)
                out << pair.first << ": " << pair.second << ", ";
            out << std::endl;
        }
        out << "cells with scores=" << (_cells.size() - nonzero) << ", without=" << nonzero << std::endl;
        out << "memory cost=" << (static_cast<double>(memoryCost) / 1000000.0) << " MB" << std::endl;
        std::string temp_str = out.str();
        std::cout << temp_str << std::endl;
        if (f) fwrite(temp_str.data(), sizeof(char), temp_str.size(), f);
    }
};
