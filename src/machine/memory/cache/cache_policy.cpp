// SPDX-License-Identifier: GPL-2.0+
/*******************************************************************************
 * QtMips - MIPS 32-bit Architecture Subset Simulator
 *
 * Implemented to support following courses:
 *
 *   B35APO - Computer Architectures
 *   https://cw.fel.cvut.cz/wiki/courses/b35apo
 *
 *   B4M35PAP - Advanced Computer Architectures
 *   https://cw.fel.cvut.cz/wiki/courses/b4m35pap/start
 *
 * Copyright (c) 2017-2019 Karel Koci<cynerd@email.cz>
 * Copyright (c) 2019      Pavel Pisa <pisa@cmp.felk.cvut.cz>
 * Copyright (c) 2020      Jakub Dupak <dupak.jakub@gmail.com>
 * Copyright (c) 2020      Max Hollmann <hollmmax@fel.cvut.cz>
 *
 * Faculty of Electrical Engineering (http://www.fel.cvut.cz)
 * Czech Technical University        (http://www.cvut.cz/)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 ******************************************************************************/

#include "cache_policy.h"

#include "simulator_exception.h"
#include "utils.h"

namespace machine {

std::unique_ptr<CachePolicy>
CachePolicy::get_policy_instance(const CacheConfig *config) {
    if (config->enabled()) {
        switch (config->replacement_policy()) {
        case CacheConfig::RP_RAND:
            return std::make_unique<CachePolicyRAND>(config->associativity());
        case CacheConfig::RP_LRU:
            return std::make_unique<CachePolicyLRU>(
                config->associativity(), config->set_count());
        case CacheConfig::RP_LFU:
            return std::make_unique<CachePolicyLFU>(
                config->associativity(), config->set_count());
        }
    } else {
        // Disabled cache will never use it.
        return { nullptr };
    }

    Q_UNREACHABLE();
}

CachePolicyLRU::CachePolicyLRU(size_t associativity, size_t set_count)
    : associativity(associativity) {
    stats.resize(set_count);
    for (auto &row : stats) {
        row.reserve(associativity);
        for (size_t i = 0; i < associativity; i++) {
            row.push_back(i);
        }
    }
}

void CachePolicyLRU::update_stats(size_t way, size_t row, bool is_valid) {
    // The following code is essentially a single pass of bubble sort (with
    // temporary variable instead of inplace swapping) adding one element to
    // back or front (respectively) of a sorted array. The sort stops, when the
    // original location of the inserted element is reached and the original
    // instance is moved to the temporary variable (`next_way`).

    try {
        // Statistics corresponding to single cache row
        std::vector<uint32_t> &row_stats = stats.at(row);
        uint32_t next_way = way;

        if (is_valid) {
            ssize_t i = associativity - 1;
            do {
                std::swap(row_stats.at(i), next_way);
                i--;
            } while (next_way != way);
        } else {
            size_t i = 0;
            do {
                std::swap(row_stats.at(i), next_way);
                i++;
            } while (next_way != way);
        }
    } catch (std::out_of_range &e) {
        throw SANITY_EXCEPTION("Out of range: LRU lost the way from priority queue.");
    }
}

size_t CachePolicyLRU::select_way_to_evict(size_t row) const {
    return stats.at(row).at(0);
}

CachePolicyLFU::CachePolicyLFU(size_t associativity, size_t set_count) {
    stats.resize(set_count, std::vector<uint32_t>(associativity, 0));
}

void CachePolicyLFU::update_stats(size_t way, size_t row, bool is_valid) {
    auto &stat_item = stats.at(row).at(way);

    if (is_valid) {
        stat_item += 1;
    } else {
        stat_item = 0;
    }
}

size_t CachePolicyLFU::select_way_to_evict(size_t row) const {
    size_t index = 0;
    try {
        // Statistics corresponding to single cache row
        auto &row_stats = stats.at(row);
        size_t lowest = row_stats.at(0);
        for (size_t i = 0; i < row_stats.size(); i++) {
            if (row_stats.at(i) == 0) {
                // Only invalid blocks have zero stat
                index = i;
                break;
            }
            if (lowest > row_stats.at(i)) {
                lowest = row_stats.at(i);
                index = i;
            }
        }
    } catch (std::out_of_range &e) {
        throw SANITY_EXCEPTION("Out of range: LRU lost the way from priority queue.");
    }
    return index;
}

CachePolicyRAND::CachePolicyRAND(size_t associativity)
    : associativity(associativity) {
    // Reset random generator to make result reproducible.
    // Random is by default seeded by 1 (by cpp standard), so this makes it
    // consistent across multiple runs.
    // NOTE: Reproducibility applies only on the same execution environment.
    std::srand(1); // NOLINT(cert-msc51-cpp)
}

void CachePolicyRAND::update_stats(size_t way, size_t row, bool is_valid) {
    UNUSED(way) UNUSED(row) UNUSED(is_valid)
    // NOP
}

size_t CachePolicyRAND::select_way_to_evict(size_t row) const {
    UNUSED(row)
    return std::rand() % associativity; // NOLINT(cert-msc50-cpp)
}
} // namespace machine
