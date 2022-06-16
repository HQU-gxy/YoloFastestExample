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
          YoloApp Python Binding
          -----------------------

          .. currentmodule:: yolo_app

          .. autosummary::
             :toctree: _generate
      )pbdoc";
  py::class_<y::Options>(m, "Options")
      .def_readwrite("scaled_coeffs", &y::Options::scaledCoeffs)
      .def_readwrite("threshold_NMS", &y::Options::thresholdNMS)
      .def_readwrite("is_border", &y::Options::isBorder)
      .def_static("init", &y::Options::fromPyDict);

  py::class_<m::MainWrapper>(m, "MainWrapper")
      .def(py::init<y::Options &>())
      .def("init", &m::MainWrapper::init)
      .def("run_push", &m::MainWrapper::pushRunDetach)
      .def("run_pull", &m::MainWrapper::pullRunDetach)
      .def("run_alt_pull", &m::MainWrapper::altPullRunDetach)
      .def("get_handler", &m::MainWrapper::getHandler)
      .def("get_pull_job", &m::MainWrapper::getPullJob)
      .def("get_alt_pull_job", &m::MainWrapper::getAltPullJob);

  /**
   default pybind11 using unique_ptr
   so the ownership has been transferred to python side
   MainWrapper can't access it anymore
   See https://pybind11.readthedocs.io/en/stable/advanced/smart_ptrs.html?highlight=shared_ptr#std-shared-ptr
  **/
  py::class_<y::PullTask, std::shared_ptr<y::PullTask>>(m, "PullTask")
      .def("set_on_poll_complete", &y::PullTask::setOnPollComplete)
      .def("clear_queue", &y::PullTask::clearQueue)
      .def("start_poll", &y::PullTask::startPoll)
      .def("reset_poll", &y::PullTask::resetPoll)
      .def("is_running", &y::PullTask::isRunning)
      .def_readwrite("max_poll", &y::PullTask::maxPoll)
      .def_readonly("poll", &y::PullTask::poll);

  py::class_<y::VideoHandler, std::shared_ptr<y::VideoHandler>>(m, "VideoHandler")
      .def("set_on_detect_yolo", &y::VideoHandler::setOnDetectYolo)
      .def("set_on_detect_door", &y::VideoHandler::setOnDetectDoor)
      .def("set_crop_rect", &y::VideoHandler::setCropRect)
      .def_readwrite("is_yolo", &y::VideoHandler::isYolo);

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
