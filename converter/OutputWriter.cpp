#include "OutputWriter.h"
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <libwgf3/wgf3.h>


bool ensurePath(const std::string path, mode_t mode = 0776) {
    std::istringstream ss(path);
    std::string token;
    struct stat st;

    std::string checkPath("");
    while(std::getline(ss, token, '/')) {
        checkPath.append(token);
        checkPath.append("/");

        if(stat(checkPath.c_str(), &st) != 0 && mkdir(checkPath.c_str(), mode) != 0 && errno != EEXIST) {
            return false;
        }
    }

    return true;
}


OutputWriter::OutputWriter(const std::string &path): outputPath(path), isDebugging(false) {
}

struct fillandw
{
    fillandw( char f, int w )
            : fill(f), width(w) {}

    char fill;
    int width;
};

std::ostream& operator<<( std::ostream& o, const fillandw& a )
{
    o.fill(a.fill);
    o.width(a.width);
    return o;
}

std::string ensureConversionPathForLayer(tm timestamp, const std::string &outputPath) {
    std::ostringstream pathToSave;

    time_t timestampAsEpoch = mktime(&timestamp);

    pathToSave << outputPath;
    if (outputPath[outputPath.size() - 1] != '/') {
        pathToSave << "/";
    }
    // without usage of sprintf_s
    pathToSave << fillandw('0', 2) << timestamp.tm_mday << "-";
    pathToSave << fillandw('0', 2) << timestamp.tm_mon << "-" << timestamp.tm_year;
    pathToSave << "-";
    pathToSave << fillandw('0', 2) << timestamp.tm_hour << "_" << timestampAsEpoch << "/";

    if (!ensurePath(pathToSave.str())) {
        throw std::runtime_error("Can't create output folder");
    }

    return pathToSave.str();
}

void OutputWriter::writeLayer(const FormatReaderAdapter::Layer &layer) {
    std::string wgf3Path = ensureConversionPathForLayer(layer.timestamp, outputPath);

    switch(layer.parameter) {
        case FormatReaderAdapter::Parameter::UWIND:
            wgf3Path.append("UGRD.wgf3");
            break;
        case FormatReaderAdapter::Parameter::VWIND:
            wgf3Path.append("VGRD.wgf3");
            break;
        default:
            throw std::runtime_error("Passed unknown parameter");
    }

    WGF3 wgf3;
    wgf3.open(wgf3Path.c_str(),
              static_cast<int>(layer.minLat * 1000), static_cast<int>(layer.maxLat * 1000),
              static_cast<int>(layer.minLon * 1000), static_cast<int>(layer.maxLon * 1000),
              static_cast<int>(layer.dLat * 1000), static_cast<int>(layer.dLon * 1000),
              static_cast<float>(layer.zeroValue),
              false
    );

    double lat, lon = layer.minLon;

    for(int y = 0; y < layer.sizeY; y += 1) {
        lat = layer.minLat;

        for(int x = 0; x < layer.sizeX; x += 1) {
            wgf3.writeAvgF(static_cast<float>(lat), static_cast<float>(lon), static_cast<float>(layer.data[y][x]));

            lat += layer.dLat;
        }

        lon += layer.dLon;
    }

    if (isDebugging) {
        wgf3Path.append(".bmp");
        wgf3.drawToBitmap(wgf3Path.c_str(), 3.0f);
    }

    wgf3.close();
}

void OutputWriter::setDebugging(bool isEnabled) {
    isDebugging = isEnabled;
}
