#pragma once

#include "noncopyable.hpp"


template<class T>
class ModuleSink : private noncopyable
{
    public:
        virtual ~ModuleSink(){}
        virtual bool process(const T &in_data) = 0;
};
