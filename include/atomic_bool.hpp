#pragma once


class atomic_bool
{
private:
    // non copyable class
    atomic_bool(atomic_bool&);
    atomic_bool& operator=(atomic_bool&);

    bool m_value;
    
public:
    atomic_bool()
    {
    }
    
    atomic_bool(bool val)
    {
        __atomic_store(&m_value, &val, __ATOMIC_SEQ_CST);
    }
    
    ~atomic_bool()
    {
    }
    
    inline bool value() const
    {
        bool ret;
        __atomic_load(&m_value, &ret, __ATOMIC_SEQ_CST);
        return ret;
    }
    
    inline void operator=(bool val)
    {
        __atomic_store(&m_value, &val, __ATOMIC_SEQ_CST);
    }
    
    inline operator bool() const
    {
        return value();
    }
    
    inline bool operator==(bool val) const
    {
        return (value() == val);
    }
    
    inline bool operator!=(bool val) const 
    {
        return (m_value != val);
    }
};


inline bool operator==(bool lhs, const atomic_bool rhs)
{
    return (lhs == rhs.value());
}

inline bool operator!=(bool lhs, const atomic_bool rhs)
{
    return (lhs != rhs.value());
}
