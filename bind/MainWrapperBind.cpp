//
// Created by crosstyan on 2022/4/27.
//

#include "../src/include/MainWrapper.h"
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace py = pybind11;
namespace m = YoloApp::Main;
namespace r = sw::redis;
namespace y = YoloApp;

PYBIND11_MODULE(yolo_app, m) {
m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: yolo_app

        .. autosummary::
           :toctree: _generate
    )pbdoc";
py::class_<m::Options>(m, "Options");
m.def("init_options", &m::OptionsFromPyDict);
py::class_<m::MainWrapper>(m, "MainWrapper")
.def(py::init<const m::Options &>())
.def("init", &m::MainWrapper::init)
.def("run_push", &m::MainWrapper::pushRun)
.def("run_pull", &m::MainWrapper::pullRun)
.def("swap_push_writer", &m::MainWrapper::swapPushWriter)
.def("swap_pull_writer", &m::MainWrapper::swapPullWriter)
.def("get_opts", &m::MainWrapper::getOpts)
.def("__repr__", [](const m::MainWrapper &m) {
return "<MainWrapper>";
});

#ifdef VERSION_INFO
m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
m.attr("__version__") = "dev";
#endif
}
