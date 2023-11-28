#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../libgymbo/pipeline.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

using namespace std;
namespace py = pybind11;

template <typename... Args>
using overload_cast_ = pybind11::detail::overload_cast_impl<Args...>;

PYBIND11_MODULE(pylibgymbo, m) {
    m.doc() = R"pbdoc(
        python API for libgymbo
    )pbdoc";

    m.def("gcompile", &gymbo::gcompile, R"pbdoc(gcompile)pbdoc");
    m.def("gexecute", &gymbo::gexecute, R"pbdoc(gexecute)pbdoc");

    py::class_<gymbo::GDOptimizer>(m, "GDOptimizer")
        .def(py::init<int, float, float, float, float, bool, int>());
    py::class_<gymbo::SymState>(m, "SymState");
    py::class_<gymbo::Prog>(m, "Prog");
    py::class_<gymbo::PathConstraintsTable>(m, "PathConstraintsTable");
    py::class_<gymbo::Token>(m, "Token");
    py::class_<gymbo::Instr>(m, "Instr")
        .def("toString", &gymbo::Instr::toString);
    py::class_<gymbo::InstrType>(m, "InstrType");

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
