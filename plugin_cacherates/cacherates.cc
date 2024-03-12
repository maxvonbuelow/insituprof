/*
 * Copyright (C) 2022, Max von Buelow
 * Technical University of Darmstadt - Interactive Graphics Systems Group
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "plugin.h"

#include "meminf.h"

#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#define pictureSize 512

struct hit_requests{
    std::vector<uint32_t> hit, request;
    hit_requests(uint32_t n) : hit(n, 0), request(n, 0)
    {}
};


struct BufAccu {
    double sdcm_sum;
    uint64_t n;
    BufAccu() : sdcm_sum(0), n(0)
    {}
    void account(double sum, uint64_t nn)
    {
        sdcm_sum += sum;
        n += nn;
    }
};
void accumulate_hit_rates(const MA *memaccs, uint32_t n, BufAccu *accus_l1, BufAccu *accus_l2)
{
    for (uint32_t i = 0; i < n; ++i) {
        const MA &ma = memaccs[i];
        int bufid = ma.bufid;
        accus_l1[bufid].account(ma.get_l1hit(), 1);
        if (ma.goes_to_l2()) accus_l2[bufid].account(ma.get_l2hit(), 1);
    }
}
void before_sim(CacheSim &cachesim)
{
    cachesim.conf.record_gwarpid = true;
    cachesim.conf.record_coalescing = true;
}
void after_sim(CacheSim &cachesim)
{
    int w = pictureSize, h = pictureSize;
    std::vector<hit_requests> states(cachesim.allocs.size(), hit_requests(w * h));
    hit_requests branching(w * h);
    for (uint64_t i = 0; i < cachesim.size(); ++i) {
        CUDADim dims[32];
        if (cachesim.memaccs[i].bufid < 0 || cachesim.memaccs[i].bufid >= states.size()) continue;
        int n = cachesim.dims_at(i, dims);
        for (int j = 0; j < n; ++j) {
            const CUDADim &dim = dims[j];

            uint64_t threadIDx = dim.bx * cachesim.griddim.tx + dim.tx;

            uint16_t x = threadIDx % pictureSize;
            uint16_t y = threadIDx / pictureSize;

            if(cachesim.memaccs[i].get_l1hit()) {
                states[cachesim.memaccs[i].bufid].hit[x + y * w]++;
            }
            states[cachesim.memaccs[i].bufid].request[x + y * w]++;
            branching.hit[x + y * w] += 1;
        }
        CUDADim basedim = cachesim.basedim(i);
        uint64_t threadIDx = basedim.bx * cachesim.griddim.tx + basedim.tx;
        uint16_t x = threadIDx % pictureSize;
        uint16_t y = threadIDx / pictureSize;
        for (int k = 0; k < 32; ++k) {
            branching.request[x + k + y * w] += 1;
        }
    }



   for(int i = 0; i < states.size(); i++){
        auto itmi = meminfs.find(cachesim.allocs[i].off);
        if (itmi != meminfs.end()) {
            itmi->second.desc;
        }
   }
    ofstream outputFile;
    outputFile.open("countedCacheMiss.txt");

    outputFile << pictureSize << ":" << pictureSize << endl;

    for(int h = 0; h < states.size(); h++) {
        auto itmi = meminfs.find(cachesim.allocs[h].off);
        if (itmi == meminfs.end()) continue;
        outputFile << "newBuffer:" << itmi->second.desc << endl;
        for (int i = 0; i < pictureSize; i++) {
            for (int j = 0; j < pictureSize; j++) {
                if (states[h].request[i + j * w] == 0) {
                    outputFile << i << ":" << j << ":" << 0 << endl;
                } else {
                    outputFile << i << ":" << j << ":" << ((double) states[h].hit[i + j * w] / (double) states[h].request[i + j * w]) << endl;
                }
            }
        }
    }
    outputFile << "newBuffer:12345" << endl;
    for (int i = 0; i < pictureSize; i += 1) {
        for (int j = 0; j < pictureSize; j++) {
            if (branching.request[i + j * w] == 0) {
                outputFile << i << ":" << j << ":" << 1 << endl;
            } else {
                outputFile << i << ":" << j << ":" << ((double) branching.hit[i + j * w] / (double) branching.request[i + j * w]) << endl;
            }
        }
    }
    outputFile << "newBuffer:12346" << endl;
    for (int i = 0; i < pictureSize; i += 1) {
        for (int j = 0; j < pictureSize; j++) {
            outputFile << i << ":" << j << ":" << branching.hit[i + j * w] << endl;
        }
    }
    outputFile.close();
}
