#include <iostream>
#include <fstream>
#include <converter/formats/Grib1FormatReaderAdapter.h>
#include <converter/OutputWriter.h>

int main(int argc, const char ** argv)
{
    using namespace std;

    if (argc < 3) {
        cout << "Usage: " << argv[0] << " GRIB1FILE OUTPATH [exportBmpFile y?]" << endl;
        return 0;
    }

    ifstream in;
    in.open(argv[1], ios::binary);
    if (!in.is_open()) {
        cout << "Can't access to " << argv[1] << endl;
        return 1;
    }

    Grib1FormatReaderAdapter reader(in, {
        FormatReaderAdapter::Parameter::UWIND,
        FormatReaderAdapter::Parameter::VWIND
    });

    OutputWriter writer(argv[2]);

    if (argc == 4 && (argv[3][0] == 'y' || argv[3][0] == '1' || argv[3][0] == 'Y')) {
        writer.setDebugging(true);
    }

    // It could be extracted to the service layer
    try {
        while(reader.readNext()) {
            FormatReaderAdapter::Layer layer = reader.getLayerData();
            writer.writeLayer(layer);
        }
    } catch (exception &e) {
        cout << "Error was occurred: " << e.what() << endl;
        return 1;
    }

    return 0;
}
