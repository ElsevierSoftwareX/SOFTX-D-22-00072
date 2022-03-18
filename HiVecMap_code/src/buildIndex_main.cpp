#include "hicore/core.hpp"
#include "hicore/core_creator.hpp"
#include "hicore/core_iterator.hpp"
#include "hicore/shp.hpp"
#include "VPQTreeNode.hpp"
#include "buildVPQTree.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <sys/stat.h>
#include <stdio.h>

using namespace std;
using namespace HiGIS::IO;
using namespace HiGIS::Core;

#define L	20037508.3427892



string split(string shp_fileName){
    string shp_name_split;
    stringstream shpName_stream(shp_fileName);
    while (getline(shpName_stream, shp_name_split, '/')){
    }
    return shp_name_split;
}

int getShpLen(string shp_fileName, string shp_name){
    string shp_name_split;
    stringstream shp_name_stream(shp_name);
    int len = 0;
    while (getline(shp_name_stream, shp_name_split, '|')){
        len++;
    }
    return len;
}

void getShpPath(string shp_fileName, string shp_name, int len, string shp_path[]){
    string shp_name_split;
    stringstream shp_name_stream(shp_name);
    int i = 0;
    while (getline(shp_name_stream, shp_name_split, '|')){
        shp_path[i] = shp_fileName + "/" + shp_name_split;
        i++;
    }
}



int main(int argc, char **argv){
    // get parameters
    string shp_fileName = argv[1]; // file path of shapefile
    string shp_name = argv[2]; // specific name of shapefile
    string output_path = argv[3];   // path of index file

    // get absolute path of shapefile
    int shp_len = getShpLen(shp_fileName, shp_name);
    string shp_path[shp_len];
    getShpPath(shp_fileName, shp_name, shp_len, shp_path);

    // read shapefile
    Shapefile shp;
    GeoData data;
    try{
        data = shp.Read(shp_path[0]);
    }
    catch(...){
        cout << "read shpFile failed" << endl;
    }
    
    // set the max tile level 
    tile_level = 9;
    tqTree_level = tile_level + 9;
    string index_path = output_path + split(shp_fileName); 
    struct stat fileStat;

    // point
    if (data.feature_type == 1){
        // have index file
        if (stat(index_path.c_str(), &fileStat) == 0){
            cout << "---- GeoType: point ---- index file exits ! ----" << endl;
        }
        // don't have index file, build VPQ-tree and save in file
        else{
            VPQTreeNode *pointVPQTree = buildPointVPQTree(shp_path, shp_len, index_path);
            // free malloc
            delete pointVPQTree;
            pointVPQTree = NULL;
            }
    }
    // line
    else if (data.feature_type == 5){
        // have index file
        if (stat(index_path.c_str(), &fileStat) == 0){
            cout << "---- GeoType: line ---- index file exits ! ----" << endl;
        }
        // don't have index file, build VPQ-tree and save in file
        else{
            VPQTreeNode *lineVPQTree = buildLineVPQTree(shp_path, shp_len, index_path);
            // free malloc
            delete lineVPQTree;
            lineVPQTree = NULL;
        }
    }
    // polygon
    else if (data.feature_type == 6){
        // have index file
        if (stat(index_path.c_str(), &fileStat) == 0){
            cout << "---- GeoType: polygon ---- index file exits ! ----" << endl;
        }
        // don't have index file, build VPQ-tree and save in file
        else{
            VPQTreeNode *polygonVPQTree = buildPolygonVPQTree(shp_path, shp_len, index_path);
            // free malloc
            delete polygonVPQTree;
            polygonVPQTree = NULL;
        }
    }
    return 0;
}







