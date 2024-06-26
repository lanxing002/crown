#pragma once
#include "py_evn.h"
#include <type_traits>
#include <vector>

namespace crown
{

template<typename Integer, std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
PyObject* create_pyobj(const Integer val)
{
	return Py_BuildValue("l", val);
}

template<typename Floating, std::enable_if_t<std::is_floating_point<Floating>::value, bool> = true>
PyObject* create_pyobj(const Floating  val)
{
	return Py_BuildValue("d", val);
}


template<typename Frist, typename...Rest>
void pack_params_inl(std::vector<PyObject*>& objs, Frist p1, Rest... ps);

template <>
void pack_params_inl(std::vector<PyObject*>& objs, const char* param);

template <typename T>
void pack_params_inl(std::vector<PyObject*>& objs, T param)
{
	objs.push_back(create_pyobj(param));
}

template<typename Frist, typename...Rest>
void pack_params_inl(std::vector<PyObject*>& objs, Frist p1, Rest... ps)
{
	objs.push_back(create_pyobj(p1));
	pack_params_inl(objs, ps...);
}

template<typename ...Ts>
void PyWrapper::invoke(const char* name, Ts...params)
{
	std::vector<PyObject*> objs;
	pack_params_inl(objs, params...);

	PyObject* params = PyTuple_New(objs.size());
	for (size_t i = 0; i < objs.size(); i++)
		PyTuple_SetItem(params, i, objs[i]);
	PyObject* func = query(name);
	if (func)
	{
		PyObject* result = PyObject_CallObject(func, params);
		Py_DECREF(params);
		Py_XDECREF(func);
		Py_XDECREF(result);

		if (PyErr_Occurred())
			PyErr_Print();
	}
}

} // namespace crown

