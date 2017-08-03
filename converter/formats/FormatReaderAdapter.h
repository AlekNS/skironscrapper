#ifndef WINDYCONV_FILEADAPTER_H
#define WINDYCONV_FILEADAPTER_H

#include <iostream>
#include <set>

/**
 * Reader
 */
class FormatReaderAdapter {
public:
    enum Parameter {
        UNKNOWN,
        UWIND,
        VWIND,
    };

    struct Layer {
        u_int parameter;
        int height;
        tm timestamp;
        double minLat;
        double minLon;
        double maxLat;
        double maxLon;
        double dLat;
        double dLon;
        u_int sizeX;
        u_int sizeY;
        double zeroValue;
        double **data;
    };

    FormatReaderAdapter(std::istream &in, std::initializer_list<Parameter> parameters);

    FormatReaderAdapter(const FormatReaderAdapter &) = delete;
    FormatReaderAdapter& operator=(const FormatReaderAdapter &) = delete;

    virtual bool readNext() = 0;
    virtual const Layer getLayerData() = 0;

protected:

    std::istream &in;
    std::set<Parameter> parameters;
};


#endif //WINDYCONV_FILEADAPTER_H
