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

        size_t id{}, *imx = nullptr;

        std::thread *t = nullptr;

        bool ready{false};

        Target( size_t _id ) : id {_id} { }

        void task() { if ( !ready ) { ready = true;

            auto th = [ & ] ( ) { m.lock(); std::cout << " id : " << id << " || "; m.unlock(); };

            t = new std::thread ( th );

            } else { t->join(); m.lock(); std::cout << " id : " << id << " << "; m.unlock(); delete t;  }

        }

        void clearmem() { delete[] imx; }

};

class BuildGraph
{
        std::vector<std::tuple<Target,Target>> p;

    public :

        size_t num_threads;

        std::vector<std::vector<Target>> routes;
        std::vector<Target*> current;
        std::vector<Target> targets;

        BuildGraph( std::initializer_list<std::tuple<Target,Target>> _p, size_t _t) : p {_p}, num_threads{_t} { }

        void init( )
        {
           struct { bool operator()( std::tuple<Target,Target> a,
                                     std::tuple<Target,Target> b ) const
                          { return ( std::get<0>(a).id < std::get<0>(b).id ); } } sortA;

           std::sort( p.begin(), p.end(), sortA );

           for ( auto& a : p ) { Target a1 = std::get<0>(a), a2 = std::get<1>(a); bool b1{false}, b2 {false};
            for ( auto& t : targets ) { if ( t.id == a1.id ) b1 = true;  if ( t.id == a2.id ) b2 = true; }
            if ( !b1 ) targets.push_back(a1); if ( !b2 ) targets.push_back(a2); }

           struct { bool operator()( Target a, Target b ) const { return ( a.id < b.id ); } } sortB;

           std::sort( targets.begin(), targets.end(), sortB );

           size_t maxV = targets.back().id+1;
           for ( auto& t : targets )  { t.imx = new size_t[maxV]{0};
             for ( auto& a : p ) if ( std::get<0>(a).id == t.id ) t.imx[std::get<1>(a).id] = 1;
             if ( t.id < 10 ) std::cout << "0"; std::cout << t.id << " ::: "; for ( size_t n = 0; n < maxV; n++ ) { if ( t.imx[n] != 1 && t.imx[n] != 0 ) t.imx[n] = 0; std::cout << t.imx[n] << " | "; } std::cout << std::endl;
           } std::cout << std::endl;


           struct iTg { std::vector<Target> *r = nullptr;
                        int vcnt{0}, del{0};

               void operator()( Target &t,
                                size_t maxV,
                                int &endcheck,
                                std::vector<Target> &targets,
                                std::vector<std::vector<Target>> &routes )
             {
                   if ( !r ) { endcheck = 0; r = new std::vector<Target>; r->push_back( t ); }

                   bool yn{false}, yynn{false};

                   for ( size_t n = 0; n < maxV; n++ ) {

                       if ( t.imx[n] == 1 ) { yn = true; vcnt++;

                       if ( endcheck == 2 ) endcheck = 3;

                       r->push_back( targets[n] );

                       if ( r->size() > maxV ) throw std::exception();

                       this->operator()( targets[n], maxV, endcheck, targets, routes );

                       if ( del == 1 ) del++;

                       if ( endcheck == 1 || endcheck == 4 || endcheck == 5 ) { t.imx[n]++; endcheck = 2; }

                       } else if ( t.imx[n] > 1 ) yynn = 1;

                   } if ( !yn && !endcheck && vcnt ) { routes.push_back(*r); endcheck = 5; }

                   if ( endcheck == 2 ) endcheck = 4;

                   if ( !yn && yynn ) del = 1;

             } };


           for ( size_t i = 0; i < maxV; i++ ) { int endcheck{1};

             while ( endcheck ) { iTg itTargets;

               itTargets.operator()( targets[i], maxV, endcheck, targets, routes );

             if ( itTargets.del == 2 ) routes.pop_back(); } }



           for ( auto& r : routes )
           { for ( auto& rr1 : r ) std::cout << rr1.id << " ";
             std::cout << " |: " << r.size() << std::endl; }

        }

        ~BuildGraph() { for ( auto& t : targets ) t.clearmem(); }

};


int main()
{
    BuildGraph g { { {6,12},{9,11},{0,2},{1,2},{1,3},{10,13},{13,15},{10,14},{15,16},{14,17},
                     {2,4},{2,5},{2,6},{1,7},{3,8},{3,5},{3,9},{5,10},{7,18},{18,19},{19,17} },
                     std::thread::hardware_concurrency() };

        try { g.init(); } catch ( std::exception )

        { std::cout << "Not ok" << std::endl; return 0; } std::cout << std::endl;


    while ( !g.routes.empty() ) { int c1{};

    for ( auto& r : g.routes ) { if ( !r.empty() ) { Target *t = &r.back(); bool iscc{};

        int c2{}; for ( auto& g : g.routes ) { if ( c1 != c2 )

             for ( auto& c : g ) { if ( c.id == t->id ) iscc = true; } c2++; }

        if ( !iscc ) g.current.push_back( t ); r.pop_back(); } c1++; }


    size_t csize = g.current.size(), i = ( csize < g.num_threads ) ? csize : g.num_threads;


    while ( !g.current.empty() ) {

    size_t m = ( static_cast<int>(csize) - static_cast<int>(i) >= 0 ) ? csize - i : 0;

    for ( size_t p = csize; p > m; p-- ) g.current[p-1]->task();

    std::this_thread::sleep_for(std::chrono::milliseconds(123)); std::cout << std::endl;

    for ( size_t p = csize; p > m; p-- ) { g.current[p-1]->task(); g.current.pop_back();}

    csize -= i; std::cout << std::endl; /* std::cout << std::endl << m << " || " << g.current.size() << std::endl; */ }

    for ( std::vector<std::vector<Target>>::iterator it1 = g.routes.begin(); it1 != g.routes.end(); )
    { if ( it1.operator*().empty() ) g.routes.erase(it1); else it1++; }

    std::cout << " - - - - - - - - - - " << std::endl; }

    return 0;
}
