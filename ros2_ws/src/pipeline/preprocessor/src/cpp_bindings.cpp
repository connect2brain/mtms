#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "preprocessor_wrapper.h"

namespace py = pybind11;

void PreprocessorWrapper::log(const std::string& message) {
  RCLCPP_INFO(*logger_ptr, "From Python: %s", message.c_str());
}

PYBIND11_MODULE(cpp_bindings, m) {
    m.def("log", &PreprocessorWrapper::log);
}
