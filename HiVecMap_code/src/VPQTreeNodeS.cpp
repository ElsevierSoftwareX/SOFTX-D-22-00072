#include "VPQTreeNodeS.hpp"
#include "VPQTreeNode.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define L 20037508.3427892


// constructor
VPQTreeNodeS::VPQTreeNodeS(short _level, VPQTreeNodeS * _parent)
{
    this->level = _level;
    this->parent = _parent;
    this->upLeftNode = nullptr;
    this->upRightNode = nullptr;
    this->bottomLeftNode = nullptr;
    this->bottomRightNode = nullptr;
    this->polygon_type = 's';
}
// deconstructor
VPQTreeNodeS::~VPQTreeNodeS()
{
    if (this->level == tqTree_level){
        return;
    }
    parent = nullptr;
}



// read VPQ-tree index file
void VPQTreeNodeS::readIndexFile(std::string index_path){
    struct timeval	t1, t2;
    gettimeofday(&t1, NULL);

    short level, quadType;
    VPQTreeNodeS *seek_node = this;
    short level_tmp = 0;

    std::ifstream inFile(index_path, std::ios::in | std::ios::binary);
    inFile.seekg(0, std::ios::beg);
    long int start = inFile.tellg();
    inFile.seekg(0, std::ios::end);
    long int end = inFile.tellg();
    long int total_size = end - start;
    int f = open((char *)index_path.c_str(), O_RDWR);
    nodeInfo *node = (nodeInfo *) mmap(0, total_size, PROT_READ|PROT_WRITE, MAP_SHARED, f, 0);

    for (int i = 0; i < total_size / sizeof(nodeInfo); i++){
        level = node[i].level;
        quadType = node[i].quadType;
        // build VPQTree
        if (quadType == 1){
            if (level <= level_tmp){
                for (int j = 0; j < level_tmp - level + 1; j++){
                    seek_node = seek_node->parent;
                }
            }
            seek_node->upLeftNode = new VPQTreeNodeS(level, seek_node);
            seek_node = seek_node->upLeftNode;
            level_tmp = level;
        }
        else if (quadType == 2){
            if (level <= level_tmp){
                for (int j = 0; j < level_tmp - level + 1; j++){
                    seek_node = seek_node->parent;
                }
            }
            seek_node->upRightNode = new VPQTreeNodeS(level, seek_node);
            seek_node = seek_node->upRightNode;
            level_tmp = level;
        }
        else if (quadType == 3){
            if (level <= level_tmp){
                for (int j = 0; j < level_tmp - level + 1; j++){
                    seek_node = seek_node->parent;
                }
            }
            seek_node->bottomLeftNode = new VPQTreeNodeS(level, seek_node);
            seek_node = seek_node->bottomLeftNode;
            level_tmp = level;
        }
        else if (quadType == 4){
            if (level <= level_tmp){
                for (int j = 0; j < level_tmp - level + 1; j++){
                    seek_node = seek_node->parent;
                }
            }
            seek_node->bottomRightNode = new VPQTreeNodeS(level, seek_node);
            seek_node = seek_node->bottomRightNode;
            level_tmp = level;
        }

    }

    gettimeofday(&t2, NULL);
    float time_use = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    std::cout << "time of read index file and build VPQTree is: " << time_use <<  " s" << std::endl;
    close(f);
    munmap((void *) node, total_size);
}


void VPQTreeNodeS::readIndexFile_polygon(std::string index_path){
    struct timeval	t1, t2;
    gettimeofday(&t1, NULL);

    short level, quadType;
    char polygon_type;
    VPQTreeNodeS *seek_node = this;
    short level_tmp = 0;

    std::ifstream inFile(index_path, std::ios::in | std::ios::binary);
    inFile.seekg(0, std::ios::beg);
    long int start = inFile.tellg();
    inFile.seekg(0, std::ios::end);
    long int end = inFile.tellg();
    long int total_size = end - start;
    int f = open((char *)index_path.c_str(), O_RDWR);
    nodeInfoPolygon *node_polygon = (nodeInfoPolygon *) mmap(0, total_size, PROT_READ|PROT_WRITE, MAP_SHARED, f, 0);

    for (int i = 0; i < total_size / sizeof(nodeInfoPolygon); i++){
        level = node_polygon[i].level;
        quadType = node_polygon[i].quadType;
        polygon_type = node_polygon[i].polygon_type;
        
        // build VPQTree
        if (quadType == 1){
            if (level <= level_tmp){
                for (int j = 0; j < level_tmp - level + 1; j++){
                    seek_node = seek_node->parent;
                }
            }
            seek_node->upLeftNode = new VPQTreeNodeS(level, seek_node);
            if (polygon_type != 's'){
                seek_node->upLeftNode->polygon_type = polygon_type;
            }
            seek_node = seek_node->upLeftNode;
            level_tmp = level;
        }
        else if (quadType == 2){
            if (level <= level_tmp){
                for (int j = 0; j < level_tmp - level + 1; j++){
                    seek_node = seek_node->parent;
                }
            }
            seek_node->upRightNode = new VPQTreeNodeS(level, seek_node);
            if (polygon_type != 's'){
                seek_node->upRightNode->polygon_type = polygon_type;
            }
            seek_node = seek_node->upRightNode;
            level_tmp = level;
        }
        else if (quadType == 3){
            if (level <= level_tmp){
                for (int j = 0; j < level_tmp - level + 1; j++){
                    seek_node = seek_node->parent;
                }
            }
            seek_node->bottomLeftNode = new VPQTreeNodeS(level, seek_node);
            if (polygon_type != 's'){
                seek_node->bottomLeftNode->polygon_type = polygon_type;
            }
            seek_node = seek_node->bottomLeftNode;
            level_tmp = level;
        }
        else if (quadType == 4){
            if (level <= level_tmp){
                for (int j = 0; j < level_tmp - level + 1; j++){
                    seek_node = seek_node->parent;
                }
            }
            seek_node->bottomRightNode = new VPQTreeNodeS(level, seek_node);
            if (polygon_type != 's'){
                seek_node->bottomRightNode->polygon_type = polygon_type;
            }
            seek_node = seek_node->bottomRightNode;
            level_tmp = level;
        }

    }

    gettimeofday(&t2, NULL);
    float time_use = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    std::cout << "time of read index file and build VPQTree is: " << time_use <<  " s" << std::endl;
    close(f);
    munmap((void *) node_polygon, total_size);
}













