#pragma once

#include "noncopyable.hpp"


template<class T>
class ModuleFilter : private noncopyable
{
    public:
        virtual ~ModuleFilter(){}
        virtual bool process(const T &in_data, T &out_data) = 0;
};
