#ifndef WINDYCONV_OUTPUTDATA_H
#define WINDYCONV_OUTPUTDATA_H

#include <string>
#include "formats/FormatReaderAdapter.h"

/**
 * Writer
 */
class OutputWriter {
public:

    OutputWriter(const std::string &path);

    void setDebugging(bool isEnabled);
    void writeLayer(const FormatReaderAdapter::Layer &layer);

protected:
    std::string outputPath;
    bool isDebugging;
};


#endif //WINDYCONV_OUTPUTDATA_H
