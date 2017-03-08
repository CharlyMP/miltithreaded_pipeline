#pragma once

#include <pthread.h>


#include "noncopyable.hpp"
#include "ThreadSafeQueue.hpp"
#include "atomic_bool.hpp"

#include "ModuleSink.hpp"


template<class T> class SourceHandler;
template<class T> class SinkHandler;
template<class T> class FilterHandler;


template<class T>
class SinkHandler : private noncopyable
{
    private:
        friend SourceHandler<T>;
        friend FilterHandler<T>;
    
        pthread_t           m_thread;
        ModuleSink<T>      *m_module;
        ThreadSafeQueue<T> *m_in_queue;
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
                
                if( m_stop_thread  || (! m_module->process(data)) )
                {
                    break;
                }
            }
        }
        
        static void* threadMainLauncher(void* p)
        {
            static_cast<SinkHandler*>(p)->threadMain();
            return NULL;
        }
        
    public:
        SinkHandler(ModuleSink<T> *module) : m_module(module), m_in_queue(NULL), m_stop_thread(false)
        {}
        
        bool run()
        {
            if( m_in_queue == NULL ) return false;
            pthread_create(&m_thread, NULL, &SinkHandler::threadMainLauncher, this);
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
