/*
 * timeseries.h
 *
 * Authors:     206534299 Ido Tziony
 *              206821258 Idan Simai
 *
 *
 */

#ifndef TIMESERIES_CPP
#define TIMESERIES_CPP

#include "anomaly_detection_util.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>



class TimeSeries {
private:
    int rowCount;
    int columnCount;
    std::ifstream fileReader;
    //This will contain all the features and the vectors they have.
    std::map<std::string, std::vector<float>> featureMap;
    //A vector, same siz as rows that contains the list of features.
    std::vector<std::string> features;

private:
    std::vector<std::string> readLine(bool firstTime);
    void initMap();
    void initVectors();

public:
explicit TimeSeries(const char *csvFileName);
//Get features.
const std::vector<std::string>& getFeatures() const {
    return features;
    }
//Get feature by a number.
const std::string& getFeature(int featureNumber) const {
    return features[featureNumber];
    }
//Get column given a feature.
const std::vector<float>& getColumn(std::string feature) const {
    return featureMap.at(feature);
    }
//Get a column by a feature number.
const std::vector<float>& getColumn(int featureNumber) const {
     return featureMap.at(getFeature(featureNumber));
    }
//Gets the row count.
int getRowCount() const {
     return rowCount;
    }
//Gets the column count.
int getColumnCount() const {
     return features.size();
    }
//For debug purposes.
friend std::ostream& operator << (std::ostream& out, const TimeSeries& time_series);
};
#endif


