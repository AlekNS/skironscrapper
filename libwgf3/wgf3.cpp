//
//  WGF3.cpp
//  WGF3
//
//  Created by Igor Kamenev on 03/04/2017.
//  Copyright © 2017 Igor Kamenev. All rights reserved.
//

#include "wgf3.h"
#include "bitmap.h"
#include <iostream>

bool WGF3::open(const char* pDbFileName)
{
    shouldBeRewrittenOnClose = false;
    
    data = nullptr;
    
    fp = fopen(pDbFileName, "r+");
    if (!fp) {
        return false;
    }
    
    fseek(fp, 0, SEEK_SET);
    fread(&header, sizeof(WGF3Header), 1, fp);
    
    data = (float*) malloc(sizeof(float) * width() * height());
    fseek(fp, sizeof(WGF3Header), SEEK_SET);
    fread(data, sizeof(float) * width() * height(), 1, fp);
    return true;
}

bool WGF3::open(const char* pDbFileName,
                int pLatMin,
                int pLatMax,
                int pLonMin,
                int pLonMax,
                int pDLat,
                int pDLon,
                float pEmptyValue,
                bool forceRecreate)
{
    
    data = nullptr;
    
    shouldBeRewrittenOnClose = true;
    
    header.latMin = pLatMin;
    header.latMax = pLatMax;
    header.lonMin = pLonMin;
    header.lonMax = pLonMax;
    header.dLat = pDLat;
    header.dLon = pDLon;
    header.emptyValue = pEmptyValue;

    bool isFileExists = false;
    bool isHeaderEqualWithNewHeader = false;
    
    fp = fopen(pDbFileName, "rb+");
    if (!fp) {
        isFileExists = false;
        isHeaderEqualWithNewHeader = false;
    } else {
        isFileExists = true;
    }
    
    if (!isFileExists || forceRecreate) {
    
        fp = fopen(pDbFileName, "w");

    } else {
    
        WGF3Header prevHeader;
        fseek(fp, 0, SEEK_SET);
        fread(&prevHeader, sizeof(WGF3Header), 1, fp);
        
        if (   prevHeader.latMin        != header.latMin
            || prevHeader.latMax        != header.latMax
            || prevHeader.lonMin        != header.lonMin
            || prevHeader.lonMax        != header.lonMax
            || prevHeader.dLat          != header.dLat
            || prevHeader.dLon          != header.dLon
            || prevHeader.emptyValue    != header.emptyValue)
        {
            
            isHeaderEqualWithNewHeader = false;
            
        } else {
            isHeaderEqualWithNewHeader = true;
        }
        
        fclose(fp);
        
        if (isHeaderEqualWithNewHeader) {
            
            return open(pDbFileName);
            
        } else {
            fp = fopen(pDbFileName, "w");
        }
    }
    
    if (!fp) {
        return false;
    }
    
    fseek(fp, 0, SEEK_SET);
    fwrite(&header, sizeof(WGF3Header), 1, fp);
    
    data = (float*) malloc(sizeof(float) * width() * height());
    for(int i=0; i < width() * height();i++){
        data[i] = header.emptyValue;
    }
    return true;
}

bool WGF3::open(const char* pDbFileName, WGF3Header config, bool forceRecreate)
{
    return open(pDbFileName,
                config.latMin,
                config.latMax,
                config.lonMin,
                config.lonMax,
                config.dLat,
                config.dLon,
                config.emptyValue,
                forceRecreate);
}

WGF3::~WGF3(void) {
    
    close();
}

void WGF3::close(void) {
    
    if (fp && shouldBeRewrittenOnClose) {
        
        fseek(fp, 0, SEEK_SET);
        fwrite(&header, sizeof(WGF3Header), 1, fp);
        
        fseek(fp, sizeof(WGF3Header), SEEK_SET);
        fwrite(data, sizeof(float)*width()*height(), 1, fp);
        
        fclose(fp);
        shouldBeRewrittenOnClose = false;
    }
    if (data) {
        free(data);
        data = 0;
    }
    
}

int WGF3::indexForLatLonF(float lat, float lon)
{
    return indexForLatLon(int(lat*1000.0), int(lon*1000.0));
}

int WGF3::indexForLatLon(int lat, int lon)
{

    if (lon > 180000) {
        lon = (lon + 180000) % 360000 - 180000;
    }
    
    if (lon < -180000) {
        lon = (lon + 180000) % 360000 + 180000;
    }
    
    if (lon < header.lonMin) {
        return -1;
    }
    
    if (lon > header.lonMax) {
        return -1;
    }
    
    if (lat < header.latMin) {
        return -1;
    }
    if (lat > header.latMax) {
        return -1;
    }
    
    int col = round( (double) (lon - header.lonMin) / (double) header.dLon);
    int row = round( (double) (lat - header.latMin) / (double) header.dLat);
    
    int idx = width() * row + col;
    
    return idx;
}


void WGF3::write(int lat, int lon, float value)
{
    shouldBeRewrittenOnClose = true;

    int idx = indexForLatLon(lat, lon);
    
    if (idx == -1) {
        return;
    }
    
    data[idx] = value;
}

void WGF3::writeAvg(int lat, int lon, float value)
{

    shouldBeRewrittenOnClose = true;
    int idx = indexForLatLon(lat, lon);
    
    
    if (idx == -1) {
        return;
    }
    
    float prevVal = data[idx];
    if (prevVal != header.emptyValue) {
        value = prevVal * 0.5 + value * 0.5;
    }

    data[idx] = value;
}

float WGF3::read(int lat, int lon)
{

    int idx = indexForLatLon(lat, lon);
    if (idx == -1) {
        return header.emptyValue;
    }
    
    return data[idx];
}


void WGF3::writeF(float lat, float lon, float value)
{
    int latI = round((lat * 1000.0) / double(header.dLat)) * header.dLat;
    int lonI = round((lon * 1000.0) / double(header.dLon)) * header.dLon;
    
    write(latI, lonI, value);
}

void WGF3::writeAvgF(float lat, float lon, float value)
{
    int latI = round((lat * 1000.0) / double(header.dLat)) * header.dLat;
    int lonI = round((lon * 1000.0) / double(header.dLon)) * header.dLon;

    writeAvg(latI, lonI, value);
}

float WGF3::readF(float lat, float lon)
{
    int latI = round((lat * 1000.0) / double(header.dLat)) * header.dLat;
    int lonI = round((lon * 1000.0) / double(header.dLon)) * header.dLon;

    return read(latI, lonI);
}


WGF3Header WGF3::getHeader()
{
    return header;
}

int WGF3::width() {
    return (header.lonMax - header.lonMin) / header.dLon + 1;
}

int WGF3::height() {
    return (header.latMax - header.latMin) / header.dLat + 1;
}

float WGF3::getEmptyValue(void)
{
    return header.emptyValue;
}

void WGF3::drawToBitmap(const char* imagePath, float maxValue)
{
    
    bitmap::initialize();
    
    // create a new bitmap image
    
    int w = width();
    int h = height();

    int bitmapW = w;
    int bitmapH = h;
    
    if (w % 2) { bitmapW++; }
    if (h % 2) { bitmapH++; }
    
    bitmap::uid_t bmp_id = bitmap::create(bitmapW, bitmapH);
    
    printf("Creating bitmap %dx%d..\n", bitmapW, bitmapH);

    float maxVal = __FLT_MIN__;


    for (int idx=0; idx < width()*height(); idx++) {

        float val = data[idx];

        if (val == header.emptyValue) {
            continue;
        }
        
        if (val > maxVal) {
            maxVal = val;
        }
    }
    
    for(int x=0; x < w; x++) {
        
        for (int y=0; y < h; y++) {
            
            
            int idx = y * w + x;
            float val = data[idx];
            
            if (val == header.emptyValue) {
                continue;
            }

            val = val / maxVal * 255.0;
            
            if (val > 255.0) {
                val = 255.0;
            }
            if (val < 0) {
                val = 0;
            }
            
            uint8_t red = val;
            
            bitmap::pixel_t px = {0x00, 0x00, red};
            
            bitmap::set_pixel(x, y, px, bmp_id);
        }
    }
    
    bitmap::write(imagePath, bmp_id);
    bitmap::remove(bmp_id);
}

bool WGF3::test() {
    
    bool result = true;
    
    if (!testFileCreate()) {
        std::cout << "false\n";
        result = false;
    } else {
        std::cout << "OK\n";
    }
    
    if (!testReadWrite()) {
        std::cout << "false\n";
        result = false;
    } else {
        std::cout << "OK\n";
    }
    
    return result;
}

bool WGF3::testFileCreate() {
    
    std::cout << "Testing file creation...";
    
    WGF3 wgf1;
    WGF3 wgf2;
    
    const char* tempFileName = "/tmp/wgftest.wgf3";

    WGF3Header header1;
    header1.dLat = 100;
    header1.dLon = 100;
    header1.latMin = -90000;
    header1.latMax = 90000;
    header1.lonMin = -180000+header1.dLat;
    header1.lonMax = 180000;
    header1.emptyValue = __FLT_MIN__;
    
    WGF3Header header2;
    header2.dLat = 100;
    header2.dLon = 100;
    header2.latMin = -80000;
    header2.latMax = 80000;
    header2.lonMin = -80000+header1.dLat;
    header2.lonMax = 80000;
    header2.emptyValue = __FLT_MIN__;
    
    bool res1 = wgf1.open(tempFileName, header1, false);
    if (!res1) {
        std::cout << " failed to open temp file. ";
        return false;
    }
    wgf1.write(5, 5, 5);
    wgf1.close();
    
    
    res1 = wgf2.open(tempFileName, header1, false);
    if (!res1) {
        std::cout << " failed to open temp file. ";
        return false;
    }

    if (wgf2.read(5, 5) != 5.0) {
        std::cout << " file was recreated but headers are equal. ";
        remove(tempFileName);
        return false;
    }
    wgf2.close();


    res1 = wgf2.open(tempFileName, header1, true);
    if (!res1) {
        std::cout << " failed to open temp file. ";
        return false;
    }
    
    if (wgf2.read(5, 5) == 5.0) {
        std::cout << " file didn't recreate but forceRecreate was true";
        remove(tempFileName);
        return false;
    }
    wgf2.write(5, 5, 5);
    wgf2.close();
    
    
    
    res1 = wgf2.open(tempFileName, header2, false);
    if (!res1) {
        std::cout << " failed to open temp file. ";
        remove(tempFileName);
        return false;
    }
    
    if (wgf2.read(5, 5) == 5.0) {
        std::cout << " file didn't recreated but headers are not equal. ";
        remove(tempFileName);
        return false;
    }
    wgf2.close();
    
    remove(tempFileName);
    
    return true;
}

bool WGF3::testReadWrite() {

    std::cout << "Testing read/write...";

    WGF3 wgf;
    WGF3Header header;
    header.dLat = 100;
    header.dLon = 100;
    header.latMin = -90000;
    header.latMax = 90000;
    header.lonMin = -180000+header.dLat;
    header.lonMax = 180000;
    header.emptyValue = __FLT_MIN__;
    
    const char* tempFileName = "/tmp/wgftest.wgf3";
    
    bool res = wgf.open(tempFileName, header, false);
    
    if (!res) {
        std::cout << "Failed to open file. ";
        return false;
    }
    
    for (int lat = header.latMin; lat <= header.latMax; lat += header.dLat) {
        
        for (int lon = header.lonMin; lon <= header.lonMax; lon += header.dLon) {
            float origValue = (lat*1000.0 + lon) / 1000.0;
            wgf.write(lat, lon, origValue);
        }
    }
    
    for (int lat = header.latMin; lat <= header.latMax; lat += header.dLat) {
        
        for (int lon = header.lonMin; lon <= header.lonMax; lon += header.dLon) {
            
            float origValue = (lat*1000.0 + lon) / 1000.0;
            float value = wgf.read(lat, lon);
            
            if (origValue != value) {
                remove(tempFileName);
                return false;;
            }
        }
    }
    
    wgf.close();
    
    wgf.open(tempFileName, header, false);
    
    for (int lat = header.latMin; lat <= header.latMax; lat += header.dLat) {
        
        for (int lon = header.lonMin; lon <= header.lonMax; lon += header.dLon) {
            
            float origValue = (lat*1000.0 + lon) / 1000.0;
            float value = wgf.read(lat, lon);
            
            if (origValue != value) {
                remove(tempFileName);
                return false;;
            }
        }
    }
    
    remove(tempFileName);
    
    return true;
}
