#pragma once

#include "noncopyable.hpp"


template<class T>
class ModuleSource : private noncopyable
{
    public:
        virtual ~ModuleSource(){}
        virtual bool process(T &out_data) = 0;
};
