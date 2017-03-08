#pragma once

#include <queue>
#include <stdint.h>
#include <unistd.h>

#include "Mutex.hpp"
#include "noncopyable.hpp"



template <typename T>
class ThreadSafeQueue : private noncopyable
{
private:
    std::queue<T>           m_queue;
    size_t                  m_queue_size;
    size_t                  m_max_size;
    mutable mutex           m_mutex;
    mutable pthread_cond_t  m_cv_push;
    mutable pthread_cond_t  m_cv_pop;

    
    std::string throw_error_with_code(const std::string &msg, const int error_code)
    {
        char tmp[10];
        sprintf(tmp, "%d", error_code);
        return std::string("ThreadSafeQueue " + msg + " (error " + std::string(tmp) + ")");
    }
    
    timespec add_us_to_current_time(const size_t us)
    {
        timespec res;
        clock_gettime(CLOCK_REALTIME, &res);
        res.tv_sec  = res.tv_sec + (res.tv_nsec + us*1000)/1000000000;
        res.tv_nsec = (res.tv_nsec + us*1000)%1000000000;
        return res;
    }
    
public:
    ThreadSafeQueue(const size_t max_size):
        m_queue(), m_queue_size(0), m_max_size(max_size)
    {
        int ret;
        
        ret = pthread_cond_init(&m_cv_push, NULL);
        if( ret != 0 )
        {
            throw throw_error_with_code("failed to init m_cv_push, error", ret);
        }
        
        ret = pthread_cond_init(&m_cv_pop, NULL);
        if( ret != 0 )
        {
            pthread_cond_destroy( &m_cv_push );
            throw throw_error_with_code("failed to init m_cv_pop, error", ret);
        }
    }
    
    ~ThreadSafeQueue()
    {
        pthread_cond_destroy( &m_cv_push );
        pthread_cond_destroy( &m_cv_pop );
    }
    
    size_t max_size() const
    {
        return m_max_size;
    }
    
    size_t size() const
    {
        try {
            m_mutex.lock();
        } catch(std::string &msg) {
            throw msg;
        }
        
        size_t ret = m_queue_size;
        
        try {
            m_mutex.unlock();
        } catch(std::string &msg) {
            throw msg;
        }
        
        return ret;
    }
    
    void push(const T &data)
    {
        m_mutex.lock();
        
            // wait until there is space in the queue
            while( m_queue_size == m_max_size )
            {
                pthread_cond_wait(&m_cv_push, m_mutex.native_handle());
            }
            
            // push to queue
            m_queue.push( data );
            ++m_queue_size;
            
            // wake up one popping thread
            pthread_cond_signal( &m_cv_pop );
        
        m_mutex.unlock();
    }
    
    bool try_push(const T &data, const size_t duration_us)
    {
        m_mutex.lock();
        
            timespec limit_time = add_us_to_current_time( duration_us );
            
            // if it's impossible to push an element until the duration is over, return false
            if( m_queue_size == m_max_size )
            {
                if( pthread_cond_timedwait(&m_cv_push, m_mutex.native_handle(), &limit_time) == ETIMEDOUT )
                {
                    m_mutex.unlock();
                    return false;
                }
            }
            
            // push to queue
            m_queue.push( data );
            ++m_queue_size;
            
            // wake up one popping thread
            pthread_cond_signal( &m_cv_pop );
        
        m_mutex.unlock();
        
        return true;
    }
    
    T pop()
    {
        m_mutex.lock();
        
            // if there is no item then we wait until there is one
            while( m_queue_size == 0 )
            {
                pthread_cond_wait(&m_cv_pop, m_mutex.native_handle());
            }
            
            // if we reach here then there is an item, get it
            T data = m_queue.front();
            m_queue.pop();
            --m_queue_size;
            
            // wake up pushing thread
            if( m_queue_size < m_max_size )
            {
                pthread_cond_signal( &m_cv_push );
            }
        
        m_mutex.unlock();
        
        return data;
    }
    
    bool try_pop(T &data, const size_t duration_us)
    {
        m_mutex.lock();
        
            timespec limit_time = add_us_to_current_time( duration_us );
        
            // if it's impossible to get an element until the duration is over, return false
            if( m_queue_size == 0 )
            {
                if( pthread_cond_timedwait(&m_cv_pop, m_mutex.native_handle(), &limit_time) == ETIMEDOUT )
                {
                    m_mutex.unlock();
                    return false;
                }
            }
            
            // if we reach here then there is an item, get it
            data = m_queue.front();
            m_queue.pop();
            --m_queue_size;
            
            // wake up pushing thread
            if( m_queue_size < m_max_size )
            {
                pthread_cond_signal( &m_cv_push );
            }
        
        m_mutex.unlock();
        
        return true;
    }
};
