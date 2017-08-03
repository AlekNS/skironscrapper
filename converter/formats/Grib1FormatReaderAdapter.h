#ifndef WINDYCONV_GRIB1FILEADAPTER_H
#define WINDYCONV_GRIB1FILEADAPTER_H

#include "FormatReaderAdapter.h"
#include <libgrib1/grib1.h>
#include <vector>

/**
 * GRIB1 Reader
 */
class Grib1FormatReaderAdapter: public FormatReaderAdapter {
public:
    Grib1FormatReaderAdapter(std::istream &in, std::initializer_list<Parameter> parameters);

    Grib1FormatReaderAdapter(const Grib1FormatReaderAdapter &) = delete;
    Grib1FormatReaderAdapter& operator=(const Grib1FormatReaderAdapter &) = delete;

    virtual bool readNext();
    virtual const Layer getLayerData();

private:
    static int readStream(void * buf, unsigned int len, void * ptr);

    GRIBRecord record;

    std::set<int> gribParameters;
};

#endif //WINDYCONV_GRIB1FILEADAPTER_H
