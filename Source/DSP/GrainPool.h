/*
  ==============================================================================
    GrainPool.h
    Pre-allocated pool of grains with free-list management.
  ==============================================================================
*/

#pragma once

#include "Grain.h"
#include "../Utils/Constants.h"
#include <array>

class GrainPool
{
public:
    GrainPool()
    {
        for (auto& g : grains)
            g.reset();
    }

    /** Acquire a free grain from the pool. Returns nullptr if all are active. */
    Grain* acquire()
    {
        for (auto& g : grains)
        {
            if (! g.active)
            {
                g.active = true;
                g.currentSample = 0;
                return &g;
            }
        }
        return nullptr;
    }

    /** Process all active grains and mix into output buffer.
        Callback signature: void(Grain&, float& leftOut, float& rightOut)
    */
    template <typename ProcessFunc>
    void processAll (ProcessFunc&& func)
    {
        for (auto& g : grains)
        {
            if (g.active)
                func (g);
        }
    }

    int getActiveCount() const
    {
        int count = 0;
        for (const auto& g : grains)
            if (g.active) ++count;
        return count;
    }

    /** Get read-only access to grains (for visualizer) */
    const std::array<Grain, GranularConstants::kMaxGrains>& getGrains() const { return grains; }

    void resetAll()
    {
        for (auto& g : grains)
            g.reset();
    }

private:
    std::array<Grain, GranularConstants::kMaxGrains> grains;
};
