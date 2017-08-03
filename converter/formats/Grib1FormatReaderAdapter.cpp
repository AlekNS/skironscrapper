#include "Grib1FormatReaderAdapter.h"
#include <grib1_unpack.h>
#include <cstring>
#include <list>
#include <algorithm>


tm convertFromGribTimestamp(const GRIBRecord &record) {
    tm stamp = {0};

    stamp.tm_year = record.yr;
    stamp.tm_mon = record.mo;
    stamp.tm_mday = record.dy;

    stamp.tm_hour = record.time / 6000; // / 100 / 60

    time_t incr = record.p1 * 60;
    if (record.fcst_units > 0) {
        incr *= 60;
    }
    if (record.fcst_units > 1) {
        incr *= 24;
    }
    if (record.fcst_units > 2) {
        throw std::runtime_error("not supported fcst_units");
    }

    time_t timeSinceEpoch = mktime(&stamp) + incr;

    return *localtime(&timeSinceEpoch);
}

constexpr u_int convertToGribParameter(const FormatReaderAdapter::Parameter &parameter) {
    switch (parameter) {
        case FormatReaderAdapter::Parameter::UWIND: return 33;
        case FormatReaderAdapter::Parameter::VWIND: return 34;
        default: return 0;
    }
}

constexpr FormatReaderAdapter::Parameter convertFromGribParameter(const GRIBRecord &record) {
    switch (record.param) {
        case 33: return FormatReaderAdapter::Parameter::UWIND;
        case 34: return FormatReaderAdapter::Parameter::VWIND;
        default: return FormatReaderAdapter::Parameter::UNKNOWN;
    }
}


Grib1FormatReaderAdapter::Grib1FormatReaderAdapter(std::istream &in, std::initializer_list<Parameter> parameters) :
        FormatReaderAdapter(in, parameters) {
    record = {0};

    std::for_each(parameters.begin(), parameters.end(), [this](const Parameter p) {
        gribParameters.insert(convertToGribParameter(p));
    });
}


bool Grib1FormatReaderAdapter::readNext() {
    while (grib1_unpack(&record, &Grib1FormatReaderAdapter::readStream, this) != -1) {
        if (gribParameters.find(record.param) == gribParameters.end()) {
            continue;
        }
        return true;
    }

    return false;
}


const FormatReaderAdapter::Layer Grib1FormatReaderAdapter::getLayerData() {
    FormatReaderAdapter::Layer layer = {0};

    layer.timestamp = convertFromGribTimestamp(record);
    layer.parameter = convertFromGribParameter(record);

    layer.height = record.lvl1;

    layer.minLat = record.slat;
    layer.minLon = record.slon;
    layer.maxLat = record.elat;
    layer.maxLon = record.elon;
    layer.dLat = record.lainc;
    layer.dLon = record.loinc;

    layer.sizeX = static_cast<u_int>(record.nx);
    layer.sizeY = static_cast<u_int>(record.ny);
    layer.zeroValue = record.ref_val;
    layer.data = record.gridpoints;

    return layer;
}


int Grib1FormatReaderAdapter::readStream(void *buf, unsigned int len, void *ptr) {
    Grib1FormatReaderAdapter *_this = static_cast<Grib1FormatReaderAdapter*>(ptr);

    return _this->in.bad() ? 0 : static_cast<int>(_this->in.read((char*)buf, len).gcount());
}
