#include "pybind11/pybind11.h"
#include "device/device.h"
int add(int i, int j) {
	return i + j;
}
PYBIND11_MODULE(example, m) {
	m.doc() = "pybind11 example plugin"; // optional module docstring
	m.def("add", &add, "A function that adds two numbers");
}

void ii()
{
	PyImport_AppendInittab("example", PyInit_example);
}
