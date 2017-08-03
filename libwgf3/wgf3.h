//
//  WGF2.hpp
//  WGF2
//
//  Created by Igor Kamenev on 03/04/2017.
//  Copyright © 2017 Igor Kamenev. All rights reserved.
//

#ifndef WGF2_hpp
#define WGF2_hpp

#include <stdio.h>
#include <stdlib.h>
#include <cmath>


struct WGF3Header {
    
    int latMin;
    int latMax;
    int lonMin;
    int lonMax;
    int dLat;
    int dLon;
    float emptyValue;
    
};

class WGF3 {
    
private:

    WGF3Header header;

    bool shouldBeRewrittenOnClose;
    float* data;


public:
    
    FILE* fp;

    ~WGF3(void);

    bool open(const char* pDbFileName);
    bool open(const char* pDbFileName, WGF3Header config, bool forceRecreate);
    bool open(const char* pDbFileName,
              int latMin,
              int latMax,
              int lonMin,
              int lonMax,
              int dLat,
              int dLon,
              float emptyValue,
              bool forceRecreate);

    int width();
    int height();
    
    int indexForLatLonF(float lat, float lon);
    int indexForLatLon(int lat, int lon);

    void write(int lat, int lon, float value);
    void writeAvg(int lat, int lon, float value);
    float read(int lat, int lon);
    
    void writeF(float lat, float lon, float value);
    void writeAvgF(float lat, float lon, float value);
    float readF(float lat, float lon);
    
    void close(void);
    void drawToBitmap(const char* imagePath, float maxValue);
    
    WGF3Header getHeader();
    float getEmptyValue(void);
    
    static bool test();
    static bool testFileCreate();
    static bool testReadWrite();
};

#endif /* WGF2_hpp */

