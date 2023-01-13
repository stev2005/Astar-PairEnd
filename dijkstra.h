#ifndef DIJKSTRA_H
#define DIJKSTRA_H
#include <bits/stdc++.h>
//#include "DNA.h"
using namespace std;

struct Node{
    int querypos, refpos, cost;
    Node(){}
    Node(int _querypos, int _refpos, int _cost){
        querypos=_querypos;
        refpos=_refpos;
        cost=_cost;
    }
    void output (){
        cout << "querypos==" << querypos << " refpos==" << refpos << " cost==" << cost << "\n" ;
    }
    bool operator< (const Node& cell) const{
        return cost>cell.cost;
    }
};

int minimum_edit_distance_dijkstra(string &query,string &Ref){
    set < pair < int , int > > visited ;
    int n, m;
    n=query.size();
    m=Ref.size();
    priority_queue<Node>q;
    for (int i=0; i<=m; ++i)
        q.push(Node(0,i,0));
    Node w,nb;
    pair < int , int > curcell;
    char c1,c2;
    while (!q.empty()){
        w=q.top();
        q.pop();
        curcell = { w.querypos , w . refpos } ;
        if ( visited . find ( curcell ) == visited . end () )
            visited . insert ( curcell ) ;
        else continue;
        if (w.querypos==n)break;
        if (w.refpos<m){
            nb=Node(w.querypos,w.refpos+1,w.cost+1);
            q.push(nb);
        }
        if (w.querypos<n&&w.refpos<m){
            c1 = Ref [ w.refpos ] ;
            c2 = query [ w.querypos ] ;
            nb=Node(w.querypos+1,w.refpos+1,w.cost);
            if(c1!=c2)++nb.cost;
            q.push(nb);
        }
        if (w.querypos<n){
            nb=Node(w.querypos+1,w.refpos,w.cost+1);
            q.push(nb);
        }
    }
    while ( ! q . empty () )
        q . pop () ;
    return w.cost;
}

#endif // DIJKSTRA_h