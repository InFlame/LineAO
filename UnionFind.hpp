#pragma once

#include <algorithm>
#include <numeric>
#include <vector>

namespace fantom
{

    class UnionFind
    {
    public:
        using Node = size_t;

        UnionFind( size_t numberOfNodes );

        /// Return the connected component of \c node.
        size_t find( Node node ) const;

        /// If \c node1 and \c node2 belong to different connected components, merge these components, i.e.,
        /// replace them by their union.
        /// Note: this is NOT thread safe.
        void unite( Node node1, Node node2 );


    private:
        // Connected components are stored as a tree that is embedded in a linear array.
        // Every node stores its parent id, root nodes point to themselves.
        mutable std::vector< size_t > mParent;

        // Rank of a component is the height of its subtree.
        // This is used for balancing of the component tree (rank optimization).
        std::vector< size_t > mRank;
    };

    inline UnionFind::UnionFind( size_t numberOfNodes )
        : mParent( numberOfNodes )
        , mRank( numberOfNodes )
    {
        std::fill( mRank.begin(), mRank.end(), 0 );
        std::iota( mParent.begin(), mParent.end(), 0 );
    }

    inline size_t UnionFind::find( Node node ) const
    {
        if( mParent[node] != node )
        {
            // Perform path compression
            // A shortcut to the overall parent is set during path traversal.
            // This speeds up search times.
            // The compression is also thread safe, because different threads would simultaneously
            // write the same value in case of a conflict, so no phantom writes or race conditions occur.
            return mParent[node] = find( mParent[node] );
        }
        else
        {
            return node;
        }
    }

    inline void UnionFind::unite( Node node1, Node node2 )
    {
        auto x = find( node1 );
        auto y = find( node2 );

        if( x == y )
        {
            return;
        }

        // balancing: smaller tree is attached to larger tree
        if( mRank[x] < mRank[y] )
        {
            mParent[x] = y;
        }
        else if( mRank[x] > mRank[y] )
        {
            mParent[y] = x;
        }
        else
        {
            mParent[y] = x;
            mRank[x]++;
        }
    }
}
