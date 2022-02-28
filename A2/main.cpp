#include <string>
#include <mpi.h>
#include <assert.h>
#include "randomizer.hpp"
#include <vector>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <algorithm>
#include <utility>
#include <unordered_map>
#include <stdio.h>

bool comp(std::pair<uint32_t, uint32_t> a, std::pair<uint32_t, uint32_t> b){
    if(a.second==b.second){
        return a.first<b.first;
    }
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

    auto begin = std::chrono::high_resolution_clock::now();
    //Only one randomizer object should be used per MPI rank, and all should have same seed
    Randomizer random_generator(seed, num_nodes, restart_prob);
    int rank, size;
    uint32_t from, to, i;
    std::vector<uint32_t> adjacents[num_nodes];
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
    std::vector<std::pair<uint32_t,uint32_t>> temp;
    std::unordered_map<uint32_t, uint32_t> score;
    char * a;
    char null[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int start = (num_nodes/size)*rank;
    int end = (num_nodes/size)*(rank+1);
    if(rank == size-1){
        end = num_nodes;
    }
    std::cout<<"rank = "<<rank<<"range = "<<start<<','<<end<<'\n';
    FILE * fp;
    fp = fopen("output.dat","w+");
    fseek(fp, start*(1+num_rec*2)*4, SEEK_SET);
    for (int node=start; node<end; node++){
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
            std::pair<uint32_t, uint32_t> p;
            p.first = iter->first;
            p.second = iter->second;
            //cout<<p.first<<','<<p.second<<'\n';
            temp.push_back(p);
        }
        sort(temp.begin(), temp.end(),comp);
        uint32_t outDegree = adjacents[node].size();
        a = (char *)&outDegree;
        //std::cout<<"\nnode "<< node << " outdegree = " << outDegree<<'\n';
        //wf.fseek()
        for(int j=0; j<4;j++){
            fwrite(a+3-j,1,1,fp);
        }
        int temp_size=temp.size();
        for(int i=0; i<num_rec; i++){
            if(i<temp_size){
                a  = (char *)&temp[i].first;
                //std::cout<<temp[i].first<<','<<temp[i].second<<'|';
                for(int j=0;j<4;j++){
                    fwrite(a+3-j,1,1,fp);
                }
                a = (char *)&temp[i].second;
                for(int j=0;j<4;j++){
                    fwrite(a+3-j,1,1,fp);
                }
            }else{
                fwrite(null,1, 8,fp);
            }
        }
    }
    /*for(int i=0;i<num_nodes;i++){
        cout<<score[0][i]<<'\n';
    }*/

    std::cout<<"WRITE COMPLETE"<<'\n';
    auto endTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - begin);
    double duration = (1e-9 * elapsed.count());
    std::cout<<"rank "<<rank<<"time = "<<duration<<'\n';
    // Extracting Rank and Processor Count
    //print_random(rank, num_nodes, random_generator);
    
    MPI_Finalize();
}
