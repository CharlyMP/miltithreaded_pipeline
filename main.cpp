#include <iostream>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

#include "include/ThreadSafeQueue.hpp"
#include "include/atomic_bool.hpp"


#include "include/ModuleSource.hpp"
#include "include/ModuleSink.hpp"
#include "include/ModuleFilter.hpp"

#include "include/SourceHandler.hpp"
#include "include/SinkHandler.hpp"
#include "include/FilterHandler.hpp"


class Feeder : public ModuleSource<int>
{
    public:
        Feeder()
        {
            srand( time(NULL) );
        }

        virtual bool process(int &out_data)
        {
            out_data = rand();
            std::cout << "Feeder " << out_data << std::endl;
            return true;
        }
};

class Writer : public ModuleSink<int>
{
    public:
        Writer()
        {}
        
        virtual bool process(const int &in_data)
        {
            std::cout << "Writer " << in_data << std::endl;
            return true;
        }
};

class Incrementer : public ModuleFilter<int>
{
    public:
        Incrementer()
        {}
        
        virtual bool process(const int &in_data, int &out_data)
        {
            out_data = in_data + 1;
            usleep( 100000 );
            return true;
        }
};


int main(void)
{
    ModuleSource<int> *feeder = new Feeder();
    ModuleFilter<int> *incrementer = new Incrementer();
    ModuleSink<int>   *writer = new Writer();
    
    
    SourceHandler<int> feederHdl( feeder );
    FilterHandler<int> incrementerHdl( incrementer );
    SinkHandler<int>   writerHdl( writer );
    
    
    feederHdl.sink( incrementerHdl );
    incrementerHdl.sink( writerHdl );

    feederHdl.run();
    incrementerHdl.run();
    writerHdl.run();
    
    usleep(1000000);
    
    feederHdl.stop();
    writerHdl.stop();
    
    
    delete incrementer;
    delete writer;
    delete feeder;

    return 0;
}
