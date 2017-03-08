#pragma once

#include <string>
#include <pthread.h>
#include <cstdlib>
#include <errno.h>
#include <cstdio>


class mutex
{
private:
    // non copyable class
    mutex(mutex&);
    mutex& operator=(mutex&);

    pthread_mutex_t m_mutex;
    
    std::string throw_error_with_code(const std::string &msg, const int error_code)
    {
        char tmp[10];
        sprintf(tmp, "%d", error_code);
        return std::string("mutex " + msg + " (error " + std::string(tmp) + ")");
    }
    
public:
    mutex()
    {
        int const res = pthread_mutex_init(&m_mutex, NULL);
        if( res )
        {
            throw throw_error_with_code("constructor failed in pthread_mutex_init", res);
        }
    }
    
    ~mutex()
    {
        pthread_mutex_destroy( &m_mutex );
    }

    void lock()
    {
        int res = pthread_mutex_lock( &m_mutex );
        if( res )
        {
            throw throw_error_with_code("lock failed in pthread_mutex_lock", res);
        }
    }

    void unlock()
    {
        int res = pthread_mutex_unlock( &m_mutex );
        if( res )
        {
            throw throw_error_with_code("unlock failed in pthread_mutex_unlock", res);
        }
    }

    bool try_lock()
    {
        int res;
        do
        {
            res = pthread_mutex_trylock( &m_mutex );
        }while( res == EINTR );
        
        if( res == EBUSY )
        {
            return false;
        }

        return !res;
    }

    pthread_mutex_t* native_handle()
    {
        return &m_mutex;
    }
};
