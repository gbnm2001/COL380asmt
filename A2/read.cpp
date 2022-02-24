#include <iostream>
#include <fstream>
#include <stdint.h>



int main(int argc, char *argv[])
{
    /*if (argc < 3)
    {
        std::cout << "Insufficient Arguments\n"; // <datafile> <n_threads>
        return 0;
    }*/

    std::fstream fs(argv[1], std::ios::in | std::ios::binary);
    int num_nodes = std::stoi(argv[2]);
    int num_rec = std::stoi(argv[3]);
    // Reading data
    uint32_t from, to;
    if(fs){
        std::cout<<argv[1]<<'\n';
    }

    for (int node = 0; node < num_nodes; node++){
        for(int i=0;i<4;i++){
            fs.read((char*)&from+3-i, 1);
        }
        std::cout<<"node "<<node<<", degree = "<<from<<'=';
        for (int rec=0; rec<num_rec;rec++){
            for(int i=0;i<4;i++){
                fs.read((char*)&from+3-i, 1);
            }
            std::cout<<','<<from;
            for(int i=0;i<4;i++){
                fs.read((char*)&from+3-i, 1);
            }
            std::cout<<'('<<from<<')';
        }
        std::cout<<'\n';
    }

    fs.close();

    
    return 0;
}
