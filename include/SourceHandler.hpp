#pragma once

#include <pthread.h>


#include "noncopyable.hpp"
#include "ThreadSafeQueue.hpp"
#include "atomic_bool.hpp"

#include "ModuleSource.hpp"


template<class T> class SourceHandler;
template<class T> class SinkHandler;
template<class T> class FilterHandler;


template<class T>
class SourceHandler : private noncopyable
{
    private:
        pthread_t           m_thread;
        ModuleSource<T>    *m_module;
        ThreadSafeQueue<T>  m_out_queue;
        atomic_bool         m_stop_thread;
        
        
        void threadMain()
        {
            while( ! m_stop_thread )
            {
                T data;
                
                if( ! m_module->process(data) )
                {
                    break;
                }
                
                while( ! m_out_queue.try_push(data, 1000) )
                {
                    if( m_stop_thread ) break;
                }
            }
        }
        
        static void* threadMainLauncher(void* p)
        {
            static_cast<SourceHandler*>(p)->threadMain();
            return NULL;
        }
        
    public:
        SourceHandler(ModuleSource<T> *module) : m_module(module), m_out_queue(10), m_stop_thread(false)
        {}
        
        ~SourceHandler()
        {
            stop();
        }
        
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
            pthread_create(&m_thread, NULL, &SourceHandler::threadMainLauncher, this);
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
