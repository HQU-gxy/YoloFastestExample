//
// Created by crosstyan on 2022/4/27.
//

#include "../src/include/MainWrapper.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

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
      .def("run_push", &m::MainWrapper::pushRunDetach)
      .def("run_pull", &m::MainWrapper::pullRunDetach)
      .def("_set_pull_writer", &m::MainWrapper::setPullWriter)
      .def("_set_pull_task_state", &m::MainWrapper::setPullTaskState)
      .def("get_pull_task_state", &m::MainWrapper::getPullTaskState)
      .def("set_on_detect_yolo", &m::MainWrapper::setOnDetectYolo)
      .def("set_on_detect_door", &m::MainWrapper::setOnDetectDoor)
      .def("set_on_poll_complete", &m::MainWrapper::setOnPollComplete)
      .def("set_yolo_state", &m::MainWrapper::setYoloState)
      .def("set_crop_rect", &m::MainWrapper::setCropRect)
      .def("get_poll", &m::MainWrapper::getPoll)
      .def("get_max_poll", &m::MainWrapper::getMaxPoll)
      .def("set_max_poll", &m::MainWrapper::setMaxPoll)
      .def("reset_poll", &m::MainWrapper::resetPoll)
      .def("_enable_poll", &m::MainWrapper::enablePoll)
      .def("start_poll", &m::MainWrapper::startPoll)
      .def("clear_queue", &m::MainWrapper::clearQueue);

  py::class_<TargetBox>(m, "TargetBox")
      .def_readwrite("x1", &TargetBox::x1)
      .def_readwrite("y1", &TargetBox::y1)
      .def_readwrite("x2", &TargetBox::x2)
      .def_readwrite("y2", &TargetBox::y2)
      .def_readwrite("cate", &TargetBox::cate)
      .def_readwrite("score", &TargetBox::score);

  py::enum_<YoloApp::Error>(m, "Err")
      .value("SUCCESS", YoloApp::Error::SUCCESS)
      .value("FAILURE", YoloApp::Error::FAILURE)
      .export_values();

  #ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
  #else
  m.attr("__version__") = "dev";
  #endif
}
