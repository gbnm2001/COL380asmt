#include <string>
#include <mpi.h>
#include <assert.h>
#include "randomizer.hpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <algorithm>
#include <utility>
#include <unordered_map>
using namespace std;


bool comp(pair<uint32_t, uint32_t> a, pair<uint32_t, uint32_t> b){
    return a.second > b.second;
}

int main(int argc, char* argv[]){
    assert(argc > 8);
    std::string graph_file = argv[1];
    int num_nodes = std::stoi(argv[2]);
    int num_edges = std::stoi(argv[3]);
    float restart_prob = std::stof(argv[4]);
    int num_steps = std::stoi(argv[5]);
    int num_walks = std::stoi(argv[6]);
    int num_rec = std::stoi(argv[7]);
    int seed = std::stoi(argv[8]);
    int limit = 10;
    std::fstream fs(graph_file, std::ios::in | std::ios::binary);

    std::cout<<"ARRAY INITIALISED\n";
    //Only one randomizer object should be used per MPI rank, and all should have same seed
    Randomizer random_generator(seed, num_nodes, restart_prob);
    int rank, size;
    uint32_t from, to, i;
    vector<uint32_t> adjacents[num_nodes];
    for (i=0; i<num_edges; i++){
        for(int i=0;i<4;i++){
            fs.read((char*)&from+3-i, 1);
        }
        for(int i=0; i<4;i++){
            fs.read((char*)&to+3-i, 1);
        }
        //cout<<from<<"->"<<to<<'\n';
        adjacents[from].push_back(to);
    }
    std::cout<<"FILE READ ADJ LIST INITIALISED\n";
    int cur_node, edges;
    vector<pair<uint32_t,uint32_t>> temp;
    std::unordered_map<uint32_t, uint32_t> score;
    std::ofstream wf("output.dat",std::ios::out | std::ios::binary);
    char * a;
    char null[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    
    for (int node=0; node<50; node++){
        score.clear();
        for(int walk=0; walk<num_walks; walk++){
            cur_node = node;
            for(int step=0; step<num_steps; step++){
                edges = adjacents[cur_node].size();
                if(edges){
                    int next_step = random_generator.get_random_value(node);
                    if(next_step == -1){
                        cur_node = node;
                    }else{
                        uint32_t adj = adjacents[cur_node][next_step%edges];
                        if(score.find(adj) == score.end()){
                            score[adj]=1;
                        }else{
                            score[adj]++;
                        }
                        cur_node = adj;
                    }
                }else{
                    cur_node = node;
                }
            }
        }
        temp.clear();
        std::unordered_map<uint32_t, uint32_t>::iterator iter;
        for (iter = score.begin(); iter!=score.end(); iter++){
            pair<uint32_t, uint32_t> p;
            p.first = iter->first;
            p.second = iter->second;
            //cout<<p.first<<','<<p.second<<'\n';
            temp.push_back(p);
        }
        sort(temp.begin(), temp.end(),comp);
        uint32_t outDegree = adjacents[node].size();
        a = (char *)&outDegree;
        std::cout<<"\nnode "<< node << " outdegree = " << outDegree<<'\n';
        for(int j=0; j<4;j++){
            wf.write(a+3-j,1);
        }
        for(int i=0; i<num_rec; i++){
            if(i<temp.size()){
                a  = (char *)&temp[i].first;
                std::cout<<temp[i].first<<','<<temp[i].second<<'|';
                for(int j=0;j<4;j++){
                    wf.write(a+3-j, 1);
                }
                a = (char *)&temp[i].second;
                for(int j=0;j<4;j++){
                    wf.write(a+3-j,1);
                }
            }else{
                wf.write(null, 8);
            }
        }
    }
    std::cout<<"CALCULATED\n";
    /*for(int i=0;i<num_nodes;i++){
        cout<<score[0][i]<<'\n';
    }*/
    wf.close();

    std::cout<<"WRITE COMPLETE"<<'\n';
    //Starting MPI pipeline
    /*MPI_Init(NULL, NULL);
    
    // Extracting Rank and Processor Count
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    print_random(rank, num_nodes, random_generator);
    
    MPI_Finalize();*/
}
