#include <set>
#include <list>
#include <vector>
#include <random>
#include <cassert>
#include <queue>

struct Random
{
    using State = int;
    using Label = int;

    std::vector< std::vector< int > > _succs;

    Random( int vertices, int edges, unsigned seed = 0 )
    {
        std::mt19937 rand{ seed };
        std::uniform_int_distribution< int > dist( 1, vertices );
        _succs.resize( vertices + 1 );

		connected[1] = true;
        /* first connect everything */
        for(int con = 2; con <= vertices; ++con) ) {
			std::uniform_int_distribution< int > dist2( 1, con - 1 );
            int next = dist2( rand );
			
            _succs[ con ].push_back( next );
            -- edges;
        }

        /* add more edges at random */
        while ( edges > 0 ) {
        next:
            int from = dist( rand );
            int to = dist( rand );
            for ( auto c : _succs[ from ] )
                if ( to == c )
                    goto next;
            _succs[ from ].push_back( to );
            -- edges;
        }
    }

    template< typename Y >
    void edges( int from, Y yield )
    {
        for ( auto t : _succs[ from ] )
            yield( t );
    }

    template< typename Y >
    void initials( Y yield )
    {
        yield( 1 );
    }
};

template< typename G >
std::pair< int, int > bfs( G &graph )
{
    std::queue< int > open;
    std::vector< bool > closed;
	closed.resize(1024);
	
    graph.initials( [&]( auto v ) { open.push( v ); } );
    int edges = -1, vertices = 0;

    while ( !open.empty() )
    {
        auto v = open.front();
        open.pop();

        ++ edges;
        if ( v < closed.size() && closed.at( v ) )
            continue;

		while(v >= closed.size()) closed.resize(closed.size()*2);
        closed.at( v ) = true;
        ++ vertices;
        graph.edges( v, [&]( auto v ) { open.push( v ); } );
    }

    return std::make_pair( vertices, edges );
}

int main()
{
    int vertices = 5000, edges = 80000;
    Random r( vertices, edges );
    auto b = bfs( r );
    assert( b.first == vertices );
    assert( b.second == edges );
    return 0;
}
