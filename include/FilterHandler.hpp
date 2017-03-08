#pragma once

#include <pthread.h>


#include "noncopyable.hpp"
#include "ThreadSafeQueue.hpp"
#include "atomic_bool.hpp"

#include "ModuleFilter.hpp"


template<class T> class SourceHandler;
template<class T> class SinkHandler;
template<class T> class FilterHandler;


template<class T>
class FilterHandler : private noncopyable
{
    private:
        friend SourceHandler<T>;
    
        pthread_t           m_thread;
        ModuleFilter<T>    *m_module;
        ThreadSafeQueue<T> *m_in_queue;
        ThreadSafeQueue<T>  m_out_queue;
        atomic_bool         m_stop_thread;
        
        void threadMain()
        {
            while( ! m_stop_thread )
            {
                T data;
                while( ! m_in_queue->try_pop(data, 1000) )
                {
                    if( m_stop_thread ) break;
                }
            
                T res;
                if( (! m_stop_thread)  &&  (! m_module->process(data, res)) )
                {
                    break;
                }
                
                while( ! m_out_queue.try_push(res, 1000) )
                {
                    if( m_stop_thread ) break;
                }
            }
        }
        
        static void* threadMainLauncher(void* p)
        {
            static_cast<FilterHandler*>(p)->threadMain();
            return NULL;
        }
        
    public:
        FilterHandler(ModuleFilter<T> *module) : m_module(module), m_in_queue(NULL), m_out_queue(10), m_stop_thread(false)
        {}
        
        bool sink(SinkHandler<T> &sinkhdl)
        {
            sinkhdl.m_in_queue = &m_out_queue;
            return true;
        }
        
        bool sink(FilterHandler<T> &sinkhdl)
        {
            sinkhdl.m_in_queue = &m_out_queue;
            return true;
        }
        
        bool run()
        {
            if( m_in_queue == NULL ) return false;
            pthread_create(&m_thread, NULL, &FilterHandler::threadMainLauncher, this);
            return true;
        }
        
        bool stop()
        {
            m_stop_thread = true;
            if( pthread_join(m_thread, NULL) )
            {
                return false;
            }
            return true;
        }
};
