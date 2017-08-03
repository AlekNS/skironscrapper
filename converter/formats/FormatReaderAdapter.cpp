#include "FormatReaderAdapter.h"

FormatReaderAdapter::FormatReaderAdapter(std::istream &in, std::initializer_list<Parameter> parameters): in(in),
parameters(parameters) {
}
