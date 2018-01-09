﻿/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2018                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#ifndef __OPENSPACE_MODULE_MULTIRESVOLUME___HISTOGRAMMANAGER___H__
#define __OPENSPACE_MODULE_MULTIRESVOLUME___HISTOGRAMMANAGER___H__

#include <fstream>
#include <modules/multiresvolume/rendering/tsp.h>
#include <openspace/util/histogram.h>

namespace openspace {

class HistogramManager {
public:
    HistogramManager(TSP * tsp);
    virtual ~HistogramManager();

    virtual bool buildHistograms(int numBins);
    virtual const Histogram* getHistogram(unsigned int brickIndex) const;

    virtual bool loadFromFile(const std::string& filename);
    virtual bool saveToFile(const std::string& filename);

    virtual const char * getName() const;
protected:
    TSP* _tsp;
    std::ifstream* _file;

    std::vector<Histogram> _histograms;
    float _minBin;
    float _maxBin;
    int _numBins;

    virtual bool initHistogramVars(int numBins);
    virtual bool buildHistogram(unsigned int brickIndex);
    virtual std::vector<float> readValues(unsigned int brickIndex) const;

    const char * _name = "histogram";
};

} // namespace openspace

#endif // __OPENSPACE_MODULE_MULTIRESVOLUME___HISTOGRAMMANAGER___H__
