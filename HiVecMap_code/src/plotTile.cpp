#include "plotTile.hpp"
#include "png.h"
#include <math.h>
#include "VPQTreeNodeS.hpp"
#include <iostream>
#include <vector>
#include <time.h>
#include <mpi.h>
#include <omp.h> 
#include <sys/time.h>
#include "string.h"
#include <cstdlib>
#include <algorithm>

using namespace std;

#define L	20037508.3427892
#define TILE_SIZE 256



bool nodeIfExist(int z, float tile_box[], float pix_x, float pix_y, VPQTreeNodeS *MBNode){
    int level = 8;
    float min_x = tile_box[0];
    float max_x = tile_box[1];
    float min_y = tile_box[2];
    float max_y = tile_box[3];
    float mid_x, mid_y;
    int divided_num = 0;

    while (divided_num < level){
        mid_x = (min_x + max_x) / 2;
        mid_y = (min_y + max_y) / 2;
        // when point in upLeftNode
        if (pix_x < mid_x && pix_y >= mid_y){
            if (MBNode->upLeftNode == nullptr){
                return false;
            }
            MBNode = MBNode->upLeftNode;
            max_x = mid_x;
            min_y = mid_y;
        }
        // when point in upRightNode
        else if (pix_x >= mid_x && pix_y >= mid_y){
            if (MBNode->upRightNode == nullptr){
                return false;
            } 
            MBNode = MBNode->upRightNode;
            min_x = mid_x;
            min_y = mid_y;
        }
        // when point in bottomLeftNode
        else if (pix_x < mid_x && pix_y < mid_y){
            if (MBNode->bottomLeftNode == nullptr){
                return false;
            }
            MBNode = MBNode->bottomLeftNode;
            max_x = mid_x;
            max_y = mid_y;
        }
        // when point in bottomRightNode
        else if (pix_x >= mid_x && pix_y < mid_y){
            if (MBNode->bottomRightNode == nullptr){
                return false;
            }
            MBNode = MBNode->bottomRightNode;
            min_x = mid_x;
            max_y = mid_y;
        }else {
            return false;
        }
        divided_num ++;
    }
    return true;
}


char nodeIfExist_polygon(int z, float tile_box[], float pix_x, float pix_y, VPQTreeNodeS *seek){
    float min_x = tile_box[0];
    float max_x = tile_box[1];
    float min_y = tile_box[2];
    float max_y = tile_box[3];
    double mid_x, mid_y;

    int level = 8 + z;
    int divided_num = 0;
    char node_type = 'n';

    while (divided_num < level){
        if (seek->polygon_type == 'b'){
            node_type = 'b';
        }
        mid_x = (min_x + max_x) / 2;
        mid_y = (min_y + max_y) / 2;
        // when point in upLeftNode
        if (pix_x < mid_x && pix_y >= mid_y){
            if (seek->upLeftNode == nullptr){
                return node_type;
            }
            seek = seek->upLeftNode;
            max_x = mid_x;
            min_y = mid_y;
        }
        // when point in upRightNode
        else if (pix_x >= mid_x && pix_y >= mid_y){
            if (seek->upRightNode == nullptr){
                return node_type;
            } 
            seek = seek->upRightNode;
            min_x = mid_x;
            min_y = mid_y;
        }
        // when point in bottomLeftNode
        else if (pix_x < mid_x && pix_y < mid_y){
            if (seek->bottomLeftNode == nullptr){
                return node_type;
            }
            seek = seek->bottomLeftNode;
            max_x = mid_x;
            max_y = mid_y;
        }
        // when point in bottomRightNode
        else if (pix_x >= mid_x && pix_y < mid_y){
            if (seek->bottomRightNode == nullptr){
                return node_type;
            }
            seek = seek->bottomRightNode;
            min_x = mid_x;
            max_y = mid_y;
        }
        divided_num++;
    }
    node_type = seek->polygon_type;
    return node_type;
}


// find the node of one tile in VPQTree by (z, x, y)
VPQTreeNodeS * locateTileNode(int z, int x, int y, VPQTreeNodeS *seek){
    float tile_Rz = L / pow(2, z-1);
    float tile_x = (x + 0.5) * tile_Rz - L;
    float tile_y = L - (y + 0.5) * tile_Rz;
    float min_x = -L;
    float max_x = L;
    float min_y = -L;
    float max_y = L;
    float mid_x, mid_y;
    int divided_num = 0;

    while (divided_num < z){
        mid_x = (min_x + max_x) / 2;
        mid_y = (min_y + max_y) / 2;
        // tile_center_pt in upLeftNode
        if (tile_x < mid_x && tile_y >= mid_y){
            if (seek->upLeftNode == nullptr){
                seek = nullptr;
                break;
            }
            seek = seek->upLeftNode;
            max_x = mid_x;
            min_y = mid_y;
        }
        // tile_center_pt in upRightNode
        else if (tile_x >= mid_x && tile_y >= mid_y){
            if (seek->upRightNode == nullptr){
                seek = nullptr;
                break;
            } 
            seek = seek->upRightNode;
            min_x = mid_x;
            min_y = mid_y;
        }
        // tile_center_pt in bottomLeftNode
        else if (tile_x < mid_x && tile_y < mid_y){
            if (seek->bottomLeftNode == nullptr){
                seek = nullptr;
                break;
            }
            seek = seek->bottomLeftNode;
            max_x = mid_x;
            max_y = mid_y;
        }
        // tile_center_pt in bottomRightNode
        else if (tile_x >= mid_x && tile_y < mid_y){
            if (seek->bottomRightNode == nullptr){
                seek = nullptr;
                break;
            }
            seek = seek->bottomRightNode;
            min_x = mid_x;
            max_y = mid_y;
        }
        divided_num ++;
    }
    return seek;
}

// find the node of each pix in VPQTree by pix_center_pt
VPQTreeNodeS * locatePixNode(int z, float tile_box[], float pix_x, float pix_y, VPQTreeNodeS *seek, VPQTreeNodeS *rootNode){
    float min_x = tile_box[0];
    float max_x = tile_box[1];
    float min_y = tile_box[2];
    float max_y = tile_box[3];
    float mid_x, mid_y;

    // 判断点是否在全球范围内
    if (abs(pix_x) <= L && abs(pix_y) <= L){
        // 当点不在结点范围内，向上寻找包含点的结点
        if (pix_x < min_x || pix_x > max_x || pix_y < min_y || pix_y > max_y){
            while(1){
                seek = rootNode;
                min_x = -L;
                max_x = L;
                min_y = -L;
                max_y = L;
                break;
            }
        }
    }
    else{
        seek = nullptr;
        return seek;
    }

    int level = 8 + z - seek->level;
    int divided_num = 0;

    while (divided_num < level){
        mid_x = (min_x + max_x) / 2;
        mid_y = (min_y + max_y) / 2;
        // when point in upLeftNode
        if (pix_x < mid_x && pix_y >= mid_y){
            if (seek->upLeftNode == nullptr){
                seek = nullptr;
                break;
            }
            seek = seek->upLeftNode;
            max_x = mid_x;
            min_y = mid_y;
        }
        // when point in upRightNode
        else if (pix_x >= mid_x && pix_y >= mid_y){
            if (seek->upRightNode == nullptr){
                seek = nullptr;
                break;
            } 
            seek = seek->upRightNode;
            min_x = mid_x;
            min_y = mid_y;
        }
        // when point in bottomLeftNode
        else if (pix_x < mid_x && pix_y < mid_y){
            if (seek->bottomLeftNode == nullptr){
                seek = nullptr;
                break;
            }
            seek = seek->bottomLeftNode;
            max_x = mid_x;
            max_y = mid_y;
        }
        // when point in bottomRightNode
        else if (pix_x >= mid_x && pix_y < mid_y){
            if (seek->bottomRightNode == nullptr){
                seek = nullptr;
                break;
            }
            seek = seek->bottomRightNode;
            min_x = mid_x;
            max_y = mid_y;
        }
        divided_num++;
    }
    return seek;
}

float *getNearestCNode(VPQTreeNodeS *seek, float pix_x, float pix_y, float near_x, float near_y, float Rz){
    float dist = 0;
    float node_x, node_y;
    if (seek->upLeftNode != nullptr){
        float node_x1 = near_x - Rz;
        float node_y1 = near_y + Rz;
        float dist1 = sqrt(pow(node_x1 - pix_x, 2) + pow(node_y1 - pix_y, 2));
        if (dist == 0){
            dist = dist1;
            node_x = node_x1;
            node_y = node_y1;
        }
    }
    if (seek->upRightNode != nullptr){
        float node_x2 = near_x + Rz;
        float node_y2 = near_y + Rz;
        float dist2 = sqrt(pow(node_x2 - pix_x, 2) + pow(node_y2 - pix_y, 2));
        if (dist == 0 || dist2 < dist){
            dist = dist2;
            node_x = node_x2;
            node_y = node_y2;
        }
    }
    if (seek->bottomLeftNode != nullptr){
        float node_x3 = near_x - Rz;
        float node_y3 = near_y - Rz;
        float dist3 = sqrt(pow(node_x3 - pix_x, 2) + pow(node_y3 - pix_y, 2));
        if (dist == 0 || dist3 < dist){
            dist = dist3;
            node_x = node_x3;
            node_y = node_y3;
        }
    }
    if (seek->bottomRightNode != nullptr){
        float node_x4 = near_x + Rz;
        float node_y4 = near_y - Rz;
        float dist4 = sqrt(pow(node_x4 - pix_x, 2) + pow(node_y4 - pix_y, 2));
        if (dist == 0 || dist4 < dist){
            dist = dist4;
            node_x = node_x4;
            node_y = node_y4;
        }
    }
    float *nearInfo = new float[3];
    nearInfo[0] = dist;
    nearInfo[1] = node_x;
    nearInfo[2] = node_y;
    return nearInfo;
}

float *getNearestCNode_polygon(VPQTreeNodeS *seek, float pix_x, float pix_y, float near_x, float near_y, float Rz){
    float dist = 0;
    float node_x, node_y;
    if (seek->upLeftNode != nullptr && seek->upLeftNode->polygon_type == 's'){
        float node_x1 = near_x - Rz;
        float node_y1 = near_y + Rz;
        float dist1 = sqrt(pow(node_x1 - pix_x, 2) + pow(node_y1 - pix_y, 2));
        if (dist == 0){
            dist = dist1;
            node_x = node_x1;
            node_y = node_y1;
        }
    }
    if (seek->upRightNode != nullptr && seek->upRightNode->polygon_type == 's'){
        float node_x2 = near_x + Rz;
        float node_y2 = near_y + Rz;
        float dist2 = sqrt(pow(node_x2 - pix_x, 2) + pow(node_y2 - pix_y, 2));
        if (dist == 0 || dist2 < dist){
            dist = dist2;
            node_x = node_x2;
            node_y = node_y2;
        }
    }
    if (seek->bottomLeftNode != nullptr && seek->bottomLeftNode->polygon_type == 's'){
        float node_x3 = near_x - Rz;
        float node_y3 = near_y - Rz;
        float dist3 = sqrt(pow(node_x3 - pix_x, 2) + pow(node_y3 - pix_y, 2));
        if (dist == 0 || dist3 < dist){
            dist = dist3;
            node_x = node_x3;
            node_y = node_y3;
        }
    }
    if (seek->bottomRightNode != nullptr && seek->bottomRightNode->polygon_type == 's'){
        float node_x4 = near_x + Rz;
        float node_y4 = near_y - Rz;
        float dist4 = sqrt(pow(node_x4 - pix_x, 2) + pow(node_y4 - pix_y, 2));
        if (dist == 0 || dist4 < dist){
            dist = dist4;
            node_x = node_x4;
            node_y = node_y4;
        }
    }
    float *nearInfo = new float[3];
    nearInfo[0] = dist;
    nearInfo[1] = node_x;
    nearInfo[2] = node_y;
    return nearInfo;
}



void pointUpResolutionSample(int z, int x, int y, VPQTreeNodeS *tile_node, VPQTreeNodeS *pointVPQTree, char *tile_area){
    short multiple = 1;
    float Rz = L / (128 << z);
    float R = multiple * Rz; 
    float R1 = R - sqrt(2) * Rz / 4.0;
    float R2 = R + sqrt(2) * Rz / 4.0;
    float Rz4 = 0.25 * Rz;
    int level = int(multiple + sqrt(2) / 4.0 - 0.5) + 1;
    // compute bounding of tile (z,x,y)
    float tile_Rz = L / pow(2, z-1);
    float min_x = x * tile_Rz - L;
    float max_x = (x + 1) * tile_Rz - L;
    float min_y = L - (y + 1) * tile_Rz;
    float max_y = L - y * tile_Rz;
    float tile_box[] = {min_x, max_x, min_y, max_y};
    // compute each pix
    # pragma omp parallel for num_threads(4)
    for (int i = 0; i < TILE_SIZE; i++){
        for (int j = 0; j < TILE_SIZE; j++){
            int tile_index = i * 256 + j;
            tile_area[tile_index] = 0;
            // get center x and y coords of each pix
            float pix_x = (256 * x + j + 0.5) * Rz - L;
            float pix_y = L - (256 * y + i + 0.5) * Rz;

            // 1. Determine whether the node corresponding to the pixel exists
            if (nodeIfExist(z, tile_box, pix_x, pix_y, tile_node)){
                tile_area[tile_index] += 4;
                continue;
            }

            short pix_value = 0;
            // Peripheral level
            for (int l = 1; l <= level; l++){
                // Get nodes on the hierarchy
                for (int m = -l; m <= l; m++){
                    for (int n = -l; n <= l; n++){
                        if (m == -l || m == l || n == -l || n == l){
                            
                            VPQTreeNodeS *node_floor = locatePixNode(z, tile_box, pix_x + m * Rz, pix_y + n * Rz, tile_node, pointVPQTree);
                            if (node_floor != nullptr){
                        
                                if (Rz * sqrt(m * m + n * n) < R1){
                                    tile_area[tile_index] = 4;
                                    break;
                                }
                                else if (Rz * sqrt(m * m + n * n) < R2){ 
                                    if (sqrt(pow(Rz4 - m * Rz, 2) + pow(Rz4 - n * Rz, 2)) < R){
                                        pix_value += 1;
                                    }
                                    if (sqrt(pow(- Rz4 - m * Rz, 2) + pow(- Rz4 - n * Rz, 2)) < R){
                                        pix_value += 1;
                                    }
                                    if (sqrt(pow(Rz4 - m * Rz, 2) + pow(- Rz4 - n * Rz, 2)) < R){
                                        pix_value += 1;
                                    }  
                                    if (sqrt(pow(- Rz4 - m * Rz, 2) + pow(Rz4 - n * Rz, 2)) < R){
                                        pix_value += 1; 
                                    }

                                    if (pix_value == 4){
                                        tile_area[tile_index] = 4;
                                        break;
                                    }
                                    else if (pix_value > tile_area[tile_index]){
                                        tile_area[tile_index] = pix_value;
                                    }
                                    pix_value = 0; 
                                }

                            }
                        }
                        
                    }
                    if (tile_area[tile_index] == 4)
                        break;
                }
                if (tile_area[tile_index] == 4)
                    break;
            }

        }
    }
}


void lineUpResolutionSample(int z, int x, int y, VPQTreeNodeS *tile_node, VPQTreeNodeS *lineVPQTree, char *tile_area){
    short multiple = 1;
    float Rz = L / (128 << z);
    float R = multiple * Rz; 
    float R1 = R - sqrt(2) * Rz / 4.0;
    float R2 = R + sqrt(2) * Rz / 4.0;
    float Rz4 = 0.25 * Rz;
    int level = int(multiple + sqrt(2) / 4.0 - 0.5) + 1;
    // compute bounding of tile (z,x,y)
    float tile_Rz = L / pow(2, z-1);
    float min_x = x * tile_Rz - L;
    float max_x = (x + 1) * tile_Rz - L;
    float min_y = L - (y + 1) * tile_Rz;
    float max_y = L - y * tile_Rz;
    float tile_box[] = {min_x, max_x, min_y, max_y};

    // compute each pix
    # pragma omp parallel for num_threads(4)
    for (int i = 0; i < TILE_SIZE; i++){
        for (int j = 0; j < TILE_SIZE; j++){
            int tile_index = i * 256 + j;
            tile_area[tile_index] = 0;
            // get center x and y coords of each pix
            float pix_x = (256 * x + j + 0.5) * Rz - L;
            float pix_y = L - (256 * y + i + 0.5) * Rz;

            // 1. Determine whether the node corresponding to the pixel exists
            if (nodeIfExist(z, tile_box, pix_x, pix_y, tile_node)){
                tile_area[tile_index] = 4;
                continue;
            }
            
            short pix_value = 0;
            // Peripheral level
            for (int l = 1; l <= level; l++){
                // Gets nodes on the hierarchy
                for (int m = -l; m <= l; m++){
                    for (int n = -l; n <= l; n++){
                        if (abs(m) == l || abs(n) == l){
                            VPQTreeNodeS *node_floor = locatePixNode(z, tile_box, pix_x + m * Rz, pix_y + n * Rz, tile_node, lineVPQTree);
                            if (node_floor != nullptr){
                                float *nearInfo = getNearestCNode(node_floor, pix_x, pix_y, pix_x + m * Rz, pix_y + n * Rz, Rz / 4);
                                float dist = nearInfo[0];
                                if (dist < R1){
                                    tile_area[tile_index] = 4;
                                    break;
                                }
                                else if (dist < R2){  
                                    float node_x = nearInfo[1];
                                    float node_y = nearInfo[2];
                                    if (sqrt(pow(pix_x + Rz4 - node_x, 2) + pow(pix_y + Rz4 - node_y, 2)) < R){
                                        pix_value += 1;
                                    }
                                    if (sqrt(pow(pix_x - Rz4 - node_x, 2) + pow(pix_y - Rz4 - node_y, 2)) < R){
                                        pix_value += 1;
                                    }
                                    if (sqrt(pow(pix_x + Rz4 - node_x, 2) + pow(pix_y - Rz4 - node_y, 2)) < R){
                                        pix_value += 1;
                                    }  
                                    if (sqrt(pow(pix_x - Rz4 - node_x, 2) + pow(pix_y + Rz4 - node_y, 2)) < R){
                                        pix_value += 1; 
                                    }

                                    if (pix_value == 4){
                                        tile_area[tile_index] = 4;
                                        break;
                                    }
                                    else if (pix_value > tile_area[tile_index]){
                                        tile_area[tile_index] = pix_value;
                                    }
                                    pix_value = 0;
                                    
                                }
                            }
                        }
                        
                    }
                    if (tile_area[tile_index] == 4)
                        break;
                }
                if (tile_area[tile_index] == 4)
                    break;
            }

        }
    }
}


void polygonUpResolutionSample(int z, int x, int y, VPQTreeNodeS *polygonVPQTree, char *tile_area){
    short multiple = 1;
    float Rz = L / (128 << z);
    float R = multiple * Rz; 
    float R1 = R - sqrt(2) * Rz / 4.0;
    float R2 = R + sqrt(2) * Rz / 4.0;
    float Rz4 = 0.25 * Rz;
    short level = int(multiple + sqrt(2) / 4.0 - 0.5) + 1;
    float min_x = - L;
    float max_x = L;
    float min_y = - L;
    float max_y = L;
    float tile_box[] = {min_x, max_x, min_y, max_y};

    // compute each pix
    # pragma omp parallel for num_threads(4)
    for (int i = 0; i < TILE_SIZE; i++){
        for (int j = 0; j < TILE_SIZE; j++){
            int tile_index = i * 256 + j;
            tile_area[tile_index] = 0;
            // get center x and y coords of each pix
            float pix_x = (256 * x + j + 0.5) * Rz - L;
            float pix_y = L - (256 * y + i + 0.5) * Rz;

            // // 1. Determine whether the node corresponding to the pixel exists
            char node_type = nodeIfExist_polygon(z, tile_box, pix_x, pix_y, polygonVPQTree);
            if (node_type == 'b'){
                tile_area[tile_index] = -1;
            }
            else if (node_type == 's'){
                tile_area[tile_index] = 4;
                continue;
            }
            
            short pix_value = 0;
            // Peripheral level
            for (int l = 1; l <= level; l++){
                // Gets nodes on the hierarchy
                for (int m = -l; m <= l; m++){
                    for (int n = -l; n <= l; n++){
                        if (abs(m) == l || abs(n) == l){
                            
                            VPQTreeNodeS *node_floor = locatePixNode(z, tile_box, pix_x + m * Rz, pix_y + n * Rz, polygonVPQTree, polygonVPQTree);
                            if (node_floor != nullptr && node_floor->polygon_type == 's'){
                                float *nearInfo = getNearestCNode_polygon(node_floor, pix_x, pix_y, pix_x + m * Rz, pix_y + n * Rz, Rz / 4);
                                float dist = nearInfo[0];
                                
                                if (dist < R1){
                                    tile_area[tile_index] = 4;
                                    break;
                                }
                                else if (dist < R2){  
                                    float node_x = nearInfo[1];
                                    float node_y = nearInfo[2];
                                    if (sqrt(pow(pix_x + Rz4 - node_x, 2) + pow(pix_y + Rz4 - node_y, 2)) < R){
                                        pix_value += 1;
                                    }
                                    if (sqrt(pow(pix_x - Rz4 - node_x, 2) + pow(pix_y - Rz4 - node_y, 2)) < R){
                                        pix_value += 1;
                                    }
                                    if (sqrt(pow(pix_x + Rz4 - node_x, 2) + pow(pix_y - Rz4 - node_y, 2)) < R){
                                        pix_value += 1;
                                    }  
                                    if (sqrt(pow(pix_x - Rz4 - node_x, 2) + pow(pix_y + Rz4 - node_y, 2)) < R){
                                        pix_value += 1; 
                                    }

                                    if (pix_value == 4){
                                        tile_area[tile_index] = 4;
                                        break;
                                    }
                                    else if (pix_value > tile_area[tile_index]){
                                        tile_area[tile_index] = pix_value;
                                    }
                                    pix_value = 0;
                                    
                                }
                                

                            }


                        }
                        
                    }
                    if (tile_area[tile_index] == 4)
                        break;
                }
                if (tile_area[tile_index] == 4)
                    break;
            }

        }
    }
}




