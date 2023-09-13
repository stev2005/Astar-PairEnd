#pragma once

#include"../header.h"
#include<bits/stdc++.h>
#include"../trie.h"

using namespace std;

struct Node{
    int rpos;
    Trie *u;

    Node(){}
    Node(int _rpos){
        rpos = _rpos;
        u = nullptr;
    }
    Node(Trie *_u){
        rpos = -1;
        u = _u;
    }

    bool is_in_trie(){
        return (u!=nullptr)?true:false;
    }

    bool operator==(const Node &other)const{
        return (rpos == other.rpos && u == other.u)?true:false;
    }

    bool operator<(const Node &other)const{
        if (rpos != other.rpos)return rpos < other.rpos;
        return u < other.u;
    }
};

struct MatchingKmers{///fast and convinient way to pass a lot of data structures as parameters to functions
    vector <int> seeds, seeds1, seeds2;
    /*seeds: does (seed[i]>=0) or doesn't(seed[i]==-1) the ith seed match a kmer;
    1 for the first alignment, 2 for the second alignment*/
    vector <int> last;///last: the end position of a last occurance of a dmer in the reference
    vector <int> prevpos;///prevpos: end positions of previous occurances of a dmer in the reference
    vector <Trie*> backtotrieconnection;///backtotrieconnection: pointer to trie leaf from which a given bp of the ref is accessed (for dmers)
    vector <int> lastkmer;///same definition as last but for kmers instead of dmers
    vector <int> prevposkmer; ///same definition as prevpos but for kmers instead of dmers
    vector <Trie*> backtotrieconnectionkmer;///same definition as backtotrieconnection but for kmers instead of dmers
    map<Node, bitset<64> > crumbs, crumbs1, crumbs2;
    vector<unordered_set<int> > crumbseeds1;
    vector<unordered_set<int> > crumbseeds2;
    void clearquerydata(){
        seeds.clear();
        seeds1.clear();
        seeds2.clear();
        crumbs.clear();
        crumbs1.clear();
        crumbs2.clear();
        crumbseeds1.clear();
        crumbseeds2.clear();
    }
};

struct  Statesr{
    ///State single read
    int qpos;
    Node p;
    cost_t g;///edit distance of alignment of using qpos and p
    cost_t h;///value of heuristic function of using qpos and p
    cost_t stepcost;///what to add to the already achived cost of the previous of state <qpos, p>
    Statesr(){}
    Statesr(int _qpos, Node _p){
        qpos = _qpos;
        p = _p;
        g = 0;
        h = 0;
        stepcost = 0;
    }
    Statesr(int _qpos, Node _p, cost_t _g, cost_t _h, cost_t _stepcost){
        qpos = _qpos;
        p = _p;
        g = _g;
        h = _h;
        stepcost = _stepcost;
    }
    bool operator<(const Statesr &other)const{
        return g + h > other.g + other.h;
    }
    bool print(){
        cout << "Statesr: "<< qpos<< " "; 
        if (p.is_in_trie())
            cout << p.u ;
        else cout << p.rpos;
        cout<< " " << g << " " << h << endl;
        return true;
    }
};

bool is_available_to_crumb(vector<unordered_set<int> > & crumbseeds, int num, int pos){
    if (crumbseeds[num].find(pos) != crumbseeds[num].end())
        return true;
    else return false;
}

void getcrumbs(const string &ref, const int d, const int k, map <Node, bitset<64> > &crumbs,
                const vector<int> &seeds, const vector<Trie*> &backtotrieconnection, const vector<int> &lastkmer,
                const vector<int> &prevposkmer, int alignment, vector<unordered_set<int> > & crumbseeds){
    /*alignment: what type of alignment is used
    valuews to receive:
        - 0 when single read has its crumbs set on the Gr+
        - 1 when first read of pair-end has its crumbs set on the Gr+
        - 2 when second read of pair-end has its crumbs set on the Gr+
        */
    set<Trie*> trienodescrumbed;
    
    for (int i = 0; i < seeds.size(); ++i){
        int cntappofseedinref = 0;
        if (seeds[i] >= 0){
            int seedpos = i * k; ///begining of a seed in the query;
            int crumbtriedist = seedpos - nindel - d;
            for (int j = lastkmer [seeds[i]]; j != -1; j = prevposkmer[j],  cntappofseedinref++){
                if (alignment == 0 || is_available_to_crumb(crumbseeds, i, j)){
                    int seedstartinref = j - k + 1;///begining of a seed in the reference
                    for (int back = 0; back < seedpos + nindel; ++back){
                        int crumbpos = seedstartinref - back;
                        if (crumbpos >= 0){
                            crumbs[Node(crumbpos)][i] = true;
                            if (seedstartinref - crumbpos > crumbtriedist){
                                Trie* crumbtrie = backtotrieconnection[crumbpos];
                                while (crumbtrie != nullptr){
                                    crumbs[Node(crumbtrie)][i] = true;
                                    crumbtrie = crumbtrie->parent;
                                    trienodescrumbed.insert(crumbtrie);
                                }
                            }
                        }
                    }
                }
            }
        }
        cout << "Seed number: " << i << " has " << cntappofseedinref << " appereances in the reference\n";
    }
    cout << "Number of trie nodes with at least one crumb: " << trienodescrumbed.size() << endl;
}

cost_t seed_heuristic(Statesr cur, int k, vector<int> &seeds, map<Node, bitset<64> > &crumbs){
    int h = 0;
    for (int i = (cur.qpos % k) ? cur.qpos / k + 1: cur.qpos / k; i < seeds.size(); ++i){
        bool check = crumbs[cur.p][i];
        if (check == false) h++;
    }
    //cout <<"Heuristic for the state: "<< cur.qpos << " " << cur.p.u << " "<<cur.p.rpos << " " << h << endl; 
    return h;
}

cost_t heuristic(Statesr cur, int k, MatchingKmers &info, char *heuristic_method, int alignment){
    if (strcmp(heuristic_method, "dijkstra_heuristic") == 0)return 0;
    if (strcmp(heuristic_method, "seed_heuristic") == 0){
        if (alignment == 0)
            return seed_heuristic(cur, k, info.seeds, info.crumbs);
        else if (alignment == 1)
            return seed_heuristic(cur, k, info.seeds1, info.crumbs1);
        else if (alignment == 2) return seed_heuristic(cur, k, info.seeds2, info.crumbs2);
        assert(false);
    }
    assert(false);
}

Statesr CreateStatesr(Statesr cur, int k, MatchingKmers &info, char *heuristic_method, cost_t stepcost, int alignment){
    return Statesr(cur.qpos, cur.p, 0, heuristic(cur, k, info, heuristic_method, alignment), stepcost);
}

vector <Statesr> NextStatesr(Statesr cur, char curqbp, const string &ref, int k, MatchingKmers & info, char *heuristic_method, int alignment){
    vector <Statesr> next;
    if (cur.p.is_in_trie()){
        if (cur.p.u->is_leaf()){
            vector <int> last = info.last;
            vector <int> prevpos = info.prevpos;
            for (int i = last[cur.p.u->num]; i != -1; i = prevpos[i]){
                next.push_back(CreateStatesr(Statesr(cur.qpos, Node(i+1)), k, info, heuristic_method, 0, alignment));
            }
        }
        else{
            next.push_back(CreateStatesr(Statesr(cur.qpos+1, cur.p), k, info, heuristic_method, 1, alignment));
            for (int i = 0; i < 4; ++i)
                if (cur.p.u->child[i] != nullptr){
                    if (base[i] == curqbp)
                        next.push_back(CreateStatesr(Statesr(cur.qpos+1, Node(cur.p.u->child[i])), k, info, heuristic_method, 0, alignment));
                    else next.push_back(CreateStatesr(Statesr(cur.qpos+1, Node(cur.p.u->child[i])), k, info, heuristic_method, 1, alignment));
                    next.push_back(CreateStatesr(Statesr(cur.qpos, Node(cur.p.u->child[i])), k, info, heuristic_method, 1, alignment));
                }
        }
    }
    else{
        if (cur.p.rpos < ref.size()){
            if (ref[cur.p.rpos] == curqbp)
                next.push_back(CreateStatesr(Statesr(cur.qpos+1, Node(cur.p.rpos+1)), k, info, heuristic_method, 0, alignment));
            else next.push_back(CreateStatesr(Statesr(cur.qpos+1, Node(cur.p.rpos+1)), k, info, heuristic_method, 1, alignment));
            next.push_back(CreateStatesr(Statesr(cur.qpos, Node(cur.p.rpos+1)), k, info, heuristic_method, 1, alignment));
        }
        next.push_back(CreateStatesr(Statesr(cur.qpos+1, cur.p), k, info, heuristic_method, 1, alignment));
    }
    return next;
}

bool toexplore(map<pair<int, Node>, int> &expandedstates, Statesr &cur){
    auto it = expandedstates.find({cur.qpos, cur.p});
    if (it == expandedstates.end()){
        expandedstates[{cur.qpos, cur.p}] = cur.g;
        return true;
    }
    else if (cur.g < it->second){
        it -> second = cur.g;
        return true;
    }
    return false;
}

bool is_greedy_available(Statesr cur, string &query, string &ref){
    if (cur.p.is_in_trie())return false;
    if (cur.p.rpos < ref.size() && query[cur.qpos] == ref[cur.p.rpos])
        return true;
    return false;
}

cost_t astar_single_read_alignment(string &query, string &ref, int d, int k, Trie *root, MatchingKmers &info, char *heuristic_method, char *showcntexplstates, char *triestart, int alignment){
    int n = query.size();
    int m = ref.size();
    priority_queue<Statesr> q;
    map<pair<int, Node>, cost_t> expandedstates;
    int cntexpansions = 0;
    int cntexpansionsTrie = 0;
    int cntexpansionsref = 0;
    int cntTrienodeswithoutcrumbs = 0;
    int cntreexpandedTrienodes = 0;
    int cntreexpandedrefnodes = 0;
    Statesr cur;
    if (strcmp(triestart, "Yes") == 0){
        cur = CreateStatesr(Statesr(0, Node(root)), k, info, heuristic_method, 0, alignment);
        cout << "Missing crumbs in the root of the trie tree: "<< cur.h << endl;
        q.push(cur);
        for (int i = m - d + 1; i <= m; ++i){
            cur = CreateStatesr(Statesr(0, Node(i)), k, info, heuristic_method, 0, alignment);
            q.push(cur);
        }
    }
    else{
        for (int i = 0; i <= m; ++i){
            cur = CreateStatesr(Statesr(0, Node(i)), k, info, heuristic_method, 0, alignment);
            q.push(cur);
        }
    }
    while(!q.empty()){
        cur = q.top();
        q.pop();
        cntexpansions++;
        if (cur.p.is_in_trie()){
            cntexpansionsTrie++;
            if (info.crumbs[cur.p].count() == 0)
                cntTrienodeswithoutcrumbs++;
            auto it = expandedstates.find({cur.qpos, cur.p});
            if (it != expandedstates.end())
                cntreexpandedTrienodes++;
        }
        else{
            cntexpansionsref++;
            auto it = expandedstates.find({cur.qpos, cur.p});
            if (it != expandedstates.end())
                cntreexpandedrefnodes++;
        }
        //cout << "cntexpansions == " << cntexpansions << " qpos == " << cur.qpos << " g == " << cur.g << " h == " << cur.h << " f == " << cur.g + cur.h << "\n";
        if (cur.qpos == n)
            break;
        if (toexplore(expandedstates, cur)){
            if (is_greedy_available(cur, query, ref)){
                Statesr topush = CreateStatesr(Statesr(cur.qpos+1, Node(cur.p.rpos+1)), k, info, heuristic_method, 0, alignment);
                topush.g += cur.g;
                q.push(topush);
            }
            else{
                vector <Statesr> next = NextStatesr(cur, query[cur.qpos], ref, k, info, heuristic_method, alignment);
                for (auto i:next){
                    i.g = cur.g + i.stepcost;
                    q.push(i);
                }
            }
        }
    }
    if (strcmp(showcntexplstates, "Yes") == 0){
        cout << "Expanded states: " << cntexpansions << "\n";
        cout << "Expanded trie states: " << cntexpansionsTrie << "\n";
        cout << "Expanded trie states (%): " << (double) cntexpansionsTrie / (double) cntexpansions * (double) 100 << "%\n";
        cout << "Expanded trie states without any crumb: " << cntTrienodeswithoutcrumbs << endl;
        cout << "Expanded trie states without any crumb (% Trie expansions): " << (double) cntTrienodeswithoutcrumbs / (double) cntexpansionsTrie * (double) 100 << "%\n";
        cout << "Expanded trie states reexpanded: " << cntreexpandedTrienodes << endl;
        cout << "Expanded trie states reexpanded (% Trie expansions): " << (double) cntreexpandedTrienodes / (double) cntexpansionsTrie * (double) 100 << "%\n";
        cout << "Expanded ref states: " << cntexpansionsref << "\n";
        cout << "Expanded ref states (%): " << (double) cntexpansionsref / (double) cntexpansions * (double) 100 << "%\n";
        cout << "Expanded ref states reexpanded (% ref expansions): " << (double) cntreexpandedrefnodes / (double) cntexpansionsref * (double) 100 << "%\n";
    }
    cout << "Band: " << (double) cntexpansions / (double) n << "\n";
    cout << "End reference position of optimal alignment: " << cur.p.rpos << "\n";
    return cur.g;
}