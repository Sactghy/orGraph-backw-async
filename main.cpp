#include <iostream>
#include <thread>
#include <exception>
#include <vector>
#include <algorithm>
#include <mutex>

std::mutex m;

class Target
{
    public:

        size_t id{}, *imx = nullptr, del{};

        std::thread *t = nullptr;

        bool ready{false};

        Target( size_t _id ) : id {_id} { }

        void task() { if ( !ready ) { ready = true;

            auto th = [ & ] ( ) { m.lock(); std::cout << " id : " << id << " || "; m.unlock(); };

            t = new std::thread ( th );

            } else { t->join(); std::cout << " id : " << id << " << "; delete t;  }

        }

        void clearmem() { delete[] imx; }

};

class BuildGraph
{
        struct Edge { Target from, to; };

        struct iTg { void operator()( Target &t, size_t maxV, int &endcheck, int &tcnt, std::vector<Target> &targets )
            
            {
                if ( !tcnt ) { endcheck = 0; tcnt++; }

                bool yn{false};

                for ( size_t n = 0; n < maxV; n++ ) {

                    if ( t.imx[n] == 1 ) { yn = true; 

                    if ( endcheck == 2 ) endcheck = 3;

                    tcnt++;

                    if ( tcnt > maxV ) throw std::exception();

                    this->operator()( targets[n], maxV, endcheck, tcnt, targets );

                    if ( endcheck == 1 || endcheck == 4 || endcheck == 5 ) { t.imx[n]++; endcheck = 2; }

                    } } if ( !yn && !endcheck && tcnt > 1 ) { tcnt++; endcheck = 5; }

                if ( endcheck == 2 ) endcheck = 4;

          } };
        
        std::vector<Edge> p;

    public :

        bool ready{false};

        size_t num_threads{1}, maxV{};

        std::vector<Target*> current;

        std::vector<Target> targets;

        BuildGraph( std::initializer_list<Edge> _p, size_t _t) : p {_p}, num_threads{_t} { }

        void init( )
        {
           auto sortA = [] ( Edge a, Edge b ) { return a.from.id < b.from.id; };
           std::sort( p.begin(), p.end(), sortA );


           for ( auto& a : p ) { Target a1 = a.from, a2 = a.to;
                                 bool b1{false}, b2 {false};

            for ( auto& t : targets ) {

                if ( t.id == a1.id ) b1 = true;
                if ( t.id == a2.id ) b2 = true; }

            if ( !b1 ) targets.push_back(a1);
            if ( !b2 ) targets.push_back(a2); }


           auto sortB = [] ( Target a, Target b ) { return a.id < b.id; };
           std::sort( targets.begin(), targets.end(), sortB );


           maxV = targets.back().id + 1;

           for ( auto& t : targets )  { t.imx = new size_t[maxV]{0};

             for ( auto& a : p ) if ( a.from.id == t.id ) t.imx[a.to.id] = 1;

             if ( t.id < 10 ) std::cout << "0"; std::cout << t.id << " ::: ";

             for ( size_t n = 0; n < maxV; n++ ) std::cout << t.imx[n] << " | "; std::cout << std::endl;

           } std::cout << std::endl;


           for ( size_t i = 0; i < maxV; i++ ) { int endcheck{1};

             while ( endcheck ) { iTg itTargets; int tcnt{};

               itTargets.operator()( targets[i], maxV, endcheck, tcnt, targets ); } }

        }

        ~BuildGraph() { for ( auto& t : targets ) t.clearmem(); }

};


int main()
{
    BuildGraph g { { {6,12},{9,11},{0,2},{1,2},{1,3},{10,13},{10,14},{13,15},{14,15},{15,19},
                     {2,4},{2,5},{2,6},{1,7},{3,8},{3,5},{3,9},{5,10},{7,16},{16,17},{17,18},
                     {20,21},{21,22},{22,23},{22,24},{23,25},{24,25},{25,26},{26,27},{27,20} },
                     std::thread::hardware_concurrency() };

        try { g.init(); } catch ( std::exception )

        { std::cout << "Loop exception!" << std::endl; return 0; } std::cout << std::endl;


    while ( !g.ready ) {


        for ( auto& t : g.targets ) { bool isready{true};

        for (size_t n = 0; n < g.maxV; n++) if ( t.imx[n] >= 1 ) isready = false;

        if ( isready && !t.ready ) g.current.push_back(&t); }

        if ( g.current.empty() ) { g.ready = true; } if ( g.ready ) break;


        for ( auto& c : g.current ) for ( auto& t : g.targets ) t.imx[c->id] = 0;


        size_t csize = g.current.size(), i = ( csize < g.num_threads ) ? csize : g.num_threads;


        while ( !g.current.empty() ) {

        size_t m = ( static_cast<int>(csize) - static_cast<int>(i) >= 0 ) ? csize - i : 0;

        for ( size_t p = csize; p > m; p-- ) g.current[p-1]->task();

        std::this_thread::sleep_for(std::chrono::milliseconds(123)); std::cout << std::endl;

        for ( size_t p = csize; p > m; p-- ) { g.current[p-1]->task(); g.current.pop_back(); }

        csize -= i; std::cout << std::endl; }

        std::cout << "-----" << std::endl;

    }

    return 0;
}
