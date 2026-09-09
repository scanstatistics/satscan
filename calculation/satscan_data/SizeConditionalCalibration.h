#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>
#include <unordered_set>

#include "boost/thread/mutex.hpp"

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
