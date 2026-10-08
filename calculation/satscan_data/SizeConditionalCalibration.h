#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <unordered_set>

#include "boost/thread/mutex.hpp"
#include "DataSet.h"

////////////////////////////////////// ******* First iteration ****** /////////////////////////////////////////////////////////

class SizeConditionalCalibration_1 {
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
    SizeConditionalCalibration_1(std::size_t nSpatialBins = 20, std::size_t nTemporalBins = 20, std::size_t minimumCellSize = 100) :
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
        //return rawScore;

        if (_mode == OFF) return rawScore;

        double p = localPValue(qS, qT, rawScore);
        return p == 1 ? 0.0 : -std::log(p);
    }

    void print(FILE* f) const {
        std::stringstream out;
        out << std::endl << "SizeConditionalCalibration_1" << std::endl;
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

////////////////////////////////////// ******* Second iteration ****** /////////////////////////////////////////////////////////

class SizeConditionalCalibration_2 {
public:
    enum Mode { OFF = 0, BUILD, APPLY };

    /** Geometry of the per-cell score histogram: how many bins, and what score range they span.

        The histogram stores the NULL pool, never the observed statistic, so the range only has to cover the
        largest raw score a calibration simulation produces - not the largest one attainable. Under the null
        each score is roughly Exp(1), so the maximum over M candidates x B simulations runs about ln(M*B):
        around 23 for a scan of 10^7 candidates over 999 replications. The defaults below give headroom over
        that. They do NOT cover the theoretical ceiling, which is ln C(C,T) - 65 for C=228, T=20, and far
        larger on a big dataset - those scores are reachable in principle but not by typical permutation draws.

        A query above the range is harmless: bin() clamps to the top bin, which holds nothing, so the tail
        count is still zero. Only an OBSERVATION past the range is lossy, and Cell::atCeiling counts those -
        treat a non-zero atCeiling in the print output as a signal that maxScore is set too low. */
    struct HistogramGeometry {
        std::size_t numBins;
        double maxScore;
        double binWidth; // derived: maxScore / numBins

        HistogramGeometry(std::size_t bins, double range) : numBins(bins), maxScore(range) {
            if (!bins) throw std::invalid_argument("Score histogram needs at least one bin.");
            if (!(range > 0.0)) throw std::invalid_argument("Score histogram range must be positive.");
            binWidth = maxScore / static_cast<double>(numBins);
        }

        /** Bin holding rawScore. Scores at or above maxScore fold into the top bin; negative scores are
            rejected by observe() before reaching here. */
        std::size_t bin(double rawScore) const {
            if (rawScore <= 0.0) return 0;
            if (rawScore >= maxScore) return numBins - 1;
            return std::min(static_cast<std::size_t>(rawScore / binWidth), numBins - 1);
        }
    };

    /** Fixed-width histogram of the null raw scores landing in one (qS,qT) cell.

        The raw hypergeometric score is -log(P{X >= x}), a function of (S,T,x) alone, so it is a discrete
        statistic taking comparatively few distinct values. Storing one double per candidate per simulation
        therefore stores the same handful of numbers many millions of times - on a dataset the size of NYC that
        exhausted memory partway through the calibration simulations. Binning caps the cost per occupied cell
        regardless of dataset size, candidate count or number of simulations.

        Bin width matters most in the tail. The empirical p-value moves in steps of 1/(N+1), so the score moves
        in steps of log((k+1)/k) at count k: 0.69 at k=1, 0.01 at k=100. A width near 0.01 therefore resolves
        the data where the counts are small, and is coarser than the data in the body - where the absolute
        score error is negligible because p_loc is O(1) there. */
    struct Cell {
        std::vector<std::uint64_t> counts;    // observations per bin, allocated on this cell's first observation
        std::vector<std::uint64_t> atOrAbove; // suffix sums over counts, built by finalize(); size numBins + 1
        std::uint64_t total;                  // observations binned into counts
        std::uint64_t underflow;              // negative raw scores - see the note in observe()
        std::uint64_t atCeiling;              // observations folded into the top bin - see HistogramGeometry
        double minQS, maxQS, minQT, maxQT;    // range of candidate sizes that landed here

        Cell() : total(0), underflow(0), atCeiling(0), minQS(1.0), maxQS(0.0), minQT(1.0), maxQT(0.0) {}

        bool occupied() const { return total != 0 || underflow != 0; }

        /** Lower edge of the bin containing the q-th quantile of this cell's null score distribution. */
        double quantile(double q, const HistogramGeometry& geometry) const {
            if (!total || atOrAbove.empty()) return std::numeric_limits<double>::quiet_NaN();
            // Smallest bin b with #{score <= b} >= q * total, i.e. atOrAbove[b+1] <= (1-q) * total.
            const double tailAllowed = (1.0 - q) * static_cast<double>(total);
            for (std::size_t b = 0; b < geometry.numBins; ++b)
                if (static_cast<double>(atOrAbove[b + 1]) <= tailAllowed)
                    return static_cast<double>(b) * geometry.binWidth;
            return geometry.maxScore;
        }
    };

private:
    std::size_t _nSpatialBins;
    std::size_t _nTemporalBins;
    std::vector<Cell> _cells;
    HistogramGeometry _geometry;
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
    /** numScoreBins and maxScore set the per-cell histogram geometry; see HistogramGeometry for how to pick
        them and for the tripwire that says maxScore is too low. Cost is 16 bytes per bin per occupied cell
        (the histogram plus its suffix sums), so the defaults run 64 KB per occupied cell. */
    SizeConditionalCalibration_2(
        std::size_t nSpatialBins = 20, std::size_t nTemporalBins = 20, std::size_t minimumCellSize = 100,
        std::size_t numScoreBins = 4000, double maxScore = 40.0
    ) : _nSpatialBins(nSpatialBins), _nTemporalBins(nTemporalBins), _cells(nSpatialBins* nTemporalBins),
        _geometry(numScoreBins, maxScore), _mode(OFF), _finalized(false), _minimumCellSize(minimumCellSize) {
    }

    void setMode(Mode mode) { _mode = mode; }
    Mode mode() const { return _mode; }
    bool finalized() const { return _finalized; }
    const HistogramGeometry& geometry() const { return _geometry; }

    /** The cell a candidate of this size falls in. Exposed for diagnostics and unit tests. */
    const Cell& getCell(double qS, double qT) const { return _cells[index(qS, qT)]; }

    void reset() {
        for (auto& cell : _cells) {
            std::vector<std::uint64_t>().swap(cell.counts); // release, don't just clear
            std::vector<std::uint64_t>().swap(cell.atOrAbove);
            cell.total = cell.underflow = cell.atCeiling = 0;
            cell.minQS = cell.minQT = 1.0;
            cell.maxQS = cell.maxQT = 0.0;
        }
        _finalized = false;
        _mode = OFF;
    }

    // Called for EVERY candidate in the Bcal null simulations.
    void observe(double qS, double qT, double rawScore) {
        if (_mode != BUILD) return;
        if (!std::isfinite(rawScore)) return;

        boost::mutex::scoped_lock lock(_mutex);
        auto& cell = _cells[index(qS, qT)];
        cell.minQS = std::min(cell.minQS, qS); cell.maxQS = std::max(cell.maxQS, qS);
        cell.minQT = std::min(cell.minQT, qT); cell.maxQT = std::max(cell.maxQT, qT);

        // A negative raw score means -log(-PROBABILITY_UNSET) leaked through from an (T,S,x) the hypergeometric
        // lookup never cached. Count those separately rather than folding them into bin 0, where they would
        // silently inflate every tail count in the cell.
        if (rawScore < 0.0) { ++cell.underflow; return; }

        if (cell.counts.empty()) cell.counts.assign(_geometry.numBins, 0);
        if (rawScore >= _geometry.maxScore) ++cell.atCeiling;
        ++cell.counts[_geometry.bin(rawScore)];
        ++cell.total;
    }

    // Called once after all Bcal simulations are complete.
    void finalize() {
        for (auto& cell : _cells) {
            if (cell.counts.empty()) { std::vector<std::uint64_t>().swap(cell.atOrAbove); continue; }
            cell.atOrAbove.assign(_geometry.numBins + 1, 0); // atOrAbove[b] = observations in bins >= b
            for (std::size_t b = _geometry.numBins; b-- > 0; )
                cell.atOrAbove[b] = cell.atOrAbove[b + 1] + cell.counts[b];
        }
        _finalized = true;
    }

    // Empirical size-conditional upper-tail probability:
    // p_loc = (1 + #{null rawScore >= observed rawScore}) / (N + 1)
    double localPValue(double qS, double qT, double rawScore) const {
        if (!_finalized) throw std::runtime_error("Size calibration has not been finalized.");

        const Cell& cell = _cells[index(qS, qT)];

        // Prototype behaviour. Production code should replace this with
        // adaptive neighbouring-bin expansion or smoothing.
        if (cell.atOrAbove.empty() || cell.total < static_cast<std::uint64_t>(_minimumCellSize))
            return 1.0;

        // Bin granularity means observations sharing rawScore's bin but sitting slightly below it are counted
        // as at-or-above. It errs toward a larger p_loc (a smaller calibrated score) rather than a smaller one.
        const std::uint64_t nGE = cell.atOrAbove[_geometry.bin(rawScore)];

        return static_cast<double>(1 + nGE) / static_cast<double>(cell.total + 1);
    }

    double score(double qS, double qT, double rawScore) const {
        if (_mode == OFF) return rawScore;

        double p = localPValue(qS, qT, rawScore);
        return p == 1 ? 0.0 : -std::log(p);
    }

    /** Writes one row of quantiles for an already-suffix-summed cell. */
    void printQuantileRow(std::stringstream& out, const Cell& cell) const {
        out << std::setw(14) << cell.total << std::fixed << std::setprecision(3)
            << std::setw(9) << cell.quantile(0.50, _geometry) << std::setw(9) << cell.quantile(0.95, _geometry)
            << std::setw(9) << cell.quantile(0.99, _geometry) << std::setw(9) << cell.quantile(0.999, _geometry)
            << std::setw(9) << cell.quantile(1.0, _geometry) << std::endl;
        out.unsetf(std::ios_base::floatfield);
    }

    void print(FILE * f) const {
        std::stringstream out;
        out << std::endl << "SizeConditionalCalibration_2" << std::endl;
        out << "_minimumCellSize=" << _minimumCellSize << ", _nSpatialBins=" << _nSpatialBins
            << ", _nTemporalBins=" << _nTemporalBins << ", _cells.size()=" << _cells.size() << std::endl;
        out << "score histogram: numBins=" << _geometry.numBins << ", maxScore=" << _geometry.maxScore
            << ", binWidth=" << _geometry.binWidth << std::endl;

        std::size_t occupied = 0, memoryCost = sizeof(*this) + _cells.capacity() * sizeof(Cell);
        std::uint64_t grandTotal = 0, grandUnderflow = 0, grandCeiling = 0;
        for (const auto& cell : _cells) {
            memoryCost += (cell.counts.capacity() + cell.atOrAbove.capacity()) * sizeof(std::uint64_t);
            grandTotal += cell.total; grandUnderflow += cell.underflow; grandCeiling += cell.atCeiling;
            if (cell.occupied()) ++occupied;
        }

        // Per-cell detail. Quantiles are in -log(p) units, at the lower edge of the containing bin.
        out << std::endl << "occupied cells:" << std::endl;
        out << "  cell sBin tBin       qS range       qT range             N   median      p95      p99    p99.9      max"
            << std::endl;
        for (std::size_t c = 0; c < _cells.size(); ++c) {
            const Cell& cell = _cells[c];
            if (!cell.occupied()) continue;
            out << std::setw(6) << c << std::setw(5) << (c / _nTemporalBins) << std::setw(5) << (c % _nTemporalBins)
                << std::fixed << std::setprecision(4)
                << std::setw(8) << cell.minQS << "-" << std::setw(6) << cell.maxQS
                << std::setw(8) << cell.minQT << "-" << std::setw(6) << cell.maxQT;
            out.unsetf(std::ios_base::floatfield);
            printQuantileRow(out, cell);
            if (cell.underflow || cell.atCeiling)
                out << "        (underflow=" << cell.underflow << ", at ceiling=" << cell.atCeiling << ")" << std::endl;
        }

        // The size diagnostic. If these quantiles rise with qS then larger candidates genuinely reach higher raw
        // scores under the null, the size bias the penalty targets is real, and the shape of the rise says how
        // much correction is warranted. If they are flat there is no per-candidate size bias to remove and large
        // clusters are winning the scan for some other reason.
        out << std::endl << "null score quantiles by spatial bin (aggregated over temporal bins):" << std::endl;
        out << "  sBin       qS range             N   median      p95      p99    p99.9      max" << std::endl;
        for (std::size_t s = 0; s < _nSpatialBins; ++s) {
            Cell agg;
            for (std::size_t t = 0; t < _nTemporalBins; ++t) {
                const Cell& cell = _cells[s * _nTemporalBins + t];
                if (cell.counts.empty()) continue;
                if (agg.counts.empty()) agg.counts.assign(_geometry.numBins, 0);
                for (std::size_t b = 0; b < _geometry.numBins; ++b) agg.counts[b] += cell.counts[b];
                agg.total += cell.total;
            }
            if (!agg.total) continue;
            agg.atOrAbove.assign(_geometry.numBins + 1, 0);
            for (std::size_t b = _geometry.numBins; b-- > 0; )
                agg.atOrAbove[b] = agg.atOrAbove[b + 1] + agg.counts[b];
            out << std::setw(6) << s << std::fixed << std::setprecision(4)
                << std::setw(9) << (static_cast<double>(s) / static_cast<double>(_nSpatialBins))
                << "-" << std::setw(6) << (static_cast<double>(s + 1) / static_cast<double>(_nSpatialBins));
            out.unsetf(std::ios_base::floatfield);
            printQuantileRow(out, agg);
        }

        out << std::endl << "cells with scores=" << occupied << ", without=" << (_cells.size() - occupied)
            << ", observations=" << grandTotal << std::endl;
        if (grandUnderflow)
            out << "WARNING: " << grandUnderflow << " negative raw scores excluded (uncached hypergeometric (T,S,x))"
                << std::endl;
        if (grandCeiling)
            out << "WARNING: " << grandCeiling << " scores at or above the " << _geometry.maxScore
                << " histogram ceiling - raise maxScore" << std::endl;
        out << "memory cost=" << (static_cast<double>(memoryCost) / 1000000.0) << " MB" << std::endl;
        std::string temp_str = out.str();
        std::cout << temp_str << std::endl;
        if (f) fwrite(temp_str.data(), sizeof(char), temp_str.size(), f);
    }
};

////////////////////////////////////// ******* Analytic iteration ****** /////////////////////////////////////////////////////////

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
        _mode(OFF), _finalized(false) {
    }

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
        const double qS = static_cast<double>(observedS) / static_cast<double>(_C);
        const double qT = static_cast<double>(observedT) / static_cast<double>(_C);
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

////////////////////////////////////// ******* Third iteration ****** /////////////////////////////////////////////////////////

class SizeConditionalCalibration_3 {
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
        //std::vector<double> rawScores;
        std::unordered_map<double, unsigned int> rawScores;
        std::vector<std::pair<double, unsigned int>> rawScoresSorted;
        std::size_t totalScores = 0;

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
    SizeConditionalCalibration_3(std::size_t nSpatialBins = 20, std::size_t nTemporalBins = 20, std::size_t minimumCellSize = 100) :
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
        ++cell.rawScores[rawScore];
        ++cell.totalScores;
        cell.proportions.insert(std::make_pair(qS, qT));
    }

    // Called once after all Bcal simulations are complete.
    void finalize() {
        for (auto& cell : _cells) {
            cell.rawScoresSorted.assign(cell.rawScores.begin(), cell.rawScores.end());
            std::sort(cell.rawScoresSorted.begin(), cell.rawScoresSorted.end(), [](auto& a, auto& b) { return a.first < b.first; });
            cell.rawScores.clear(); // No longer needed
        }
        _finalized = true;
    }

    // Empirical size-conditional upper-tail probability:
    // p_loc = (1 + #{null rawScore >= observed rawScore}) / (N + 1)
    double localPValue(double qS, double qT, double rawScore) const {
        if (!_finalized) throw std::runtime_error("Size calibration has not been finalized.");

        const Cell& cell = _cells[index(qS, qT)];

        // Prototype behaviour. Production code should replace this with
        // adaptive neighbouring-bin expansion or smoothing.
        if (cell.totalScores < _minimumCellSize)
            return 1.0;

        auto it = std::lower_bound(cell.rawScoresSorted.begin(), cell.rawScoresSorted.end(),
            rawScore, [](const auto& elem, const double& k) { return elem.first < k; }
        );
        // Advance iterator to next element if score is less than rawScore.
        if (it != cell.rawScoresSorted.end() && it->first < rawScore)
            ++it;
        // Record the number of scores that were equal or greater to rawScore.
        std::size_t nGE = 0;
        while (it != cell.rawScoresSorted.end()) {
            nGE += it->second;
            ++it;
        }
        const std::size_t N = cell.totalScores;

        return static_cast<double>(1 + nGE) / static_cast<double>(N + 1);
    }

    double score(double qS, double qT, double rawScore) const {
        if (_mode == OFF) return rawScore;

        double p = localPValue(qS, qT, rawScore);
        return p == 1 ? 0.0 : -std::log(p);
    }

    void print(FILE* f) const {
        std::stringstream out;
        out << std::endl << "SizeConditionalCalibration_3" << std::endl;
        out << "_minimumCellSize=" << _minimumCellSize << ", _nSpatialBins=" << _nSpatialBins << ", _nTemporalBins=" << _nTemporalBins << std::endl;
        size_t nonzero = 0;
        std::size_t memoryCost = sizeof(*this) + _cells.capacity() * sizeof(Cell);
        out << "_cells.size()=" << _cells.size() << std::endl;
        for (size_t t = 0; t < _cells.size(); ++t) {
            auto& cell = _cells[t];
            if (cell.rawScoresSorted.empty()) ++nonzero;
            memoryCost += cell.rawScoresSorted.capacity() * sizeof(std::pair<double, unsigned int>);
            if (cell.rawScores.empty()) continue;
            out << "cell[" << t << "], size=" << cell.rawScoresSorted.size() <<
                ", lowest=" << cell.rawScoresSorted.front().first << ", highest=" << cell.rawScoresSorted.back().first << std::endl;
            out << "proportions(qS,qT) size: " << cell.proportions.size() << std::endl;
            for (auto p : cell.proportions) {
                out << p.first << ": " << p.second << ", ";
            }
            std::map<double, unsigned int> frequency;
            out << std::endl << "freq: ";
            for (auto& pair : cell.rawScoresSorted)
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
