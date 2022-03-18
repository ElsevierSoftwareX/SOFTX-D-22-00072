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

short tile_level;
short tqTree_level;

// constructor
VPQTreeNode::VPQTreeNode(float _x, float _y, float _width, short _level, QuadType _quadType, VPQTreeNode * _parent)
{
    this->x = _x;
    this->y = _y;
    this->width = _width;
    this->level = _level;
    this->quadType = _quadType;
    this->parent = _parent;
    this->upLeftNode = nullptr;
    this->upRightNode = nullptr;
    this->bottomLeftNode = nullptr;
    this->bottomRightNode = nullptr;
    this->polygon_type = 's';
}
// deconstructor
VPQTreeNode::~VPQTreeNode()
{
    if (this->level == tqTree_level){
        return;
    }
    parent = nullptr;
}


bool VPQTreeNode::IsContain(float node_x, float node_y, float width, float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max) const{
    if (obj_x_min >= node_x && 
        obj_x_max <= node_x + width && 
        obj_y_min >= node_y && 
        obj_y_max <= node_y + width){
        return true;
    }
    return false;
}

bool VPQTreeNode::IsContainPoint(float node_x, float node_y, float width, float point_x, float point_y) const{
    if (point_x >= node_x && 
        point_x <= node_x + width && 
        point_y >= node_y && 
        point_y <= node_y + width){
        return true;
    }
    return false;
}

bool VPQTreeNode::IsIntersect(float node_x, float node_y, float width, float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max) const{
    if (obj_x_min < node_x + width && 
        obj_x_max > node_x && 
        obj_y_min < node_y + width && 
        obj_y_max > node_y){
        return true;
    }
    return false;
}

bool VPQTreeNode::IsCoverNode(float node_x, float node_y, float width, float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max, string obj_type) const{
    if (obj_x_min < node_x + width && 
        obj_x_max > node_x && 
        obj_y_min < node_y + width && 
        obj_y_max > node_y){

        float center_x = node_x + width / 2;
        float center_y = node_y + width / 2;
        float obj_width = obj_x_max - obj_x_min;
        float obj_height = obj_y_max - obj_y_min;
        if (obj_type == "0"){
            float a = obj_width * (center_y - obj_y_min) - (center_x - obj_x_min) * obj_height;
            if (a == 0){
                return true;
            }
            else if (a > 0){
                float b = obj_width * (node_y - obj_y_min) - (node_x + width - obj_x_min) * obj_height;
                if (a * b < 0){
                    return true;
                }
            }
            else if (a < 0){
                float b = obj_width * (node_y + width - obj_y_min) - (node_x - obj_x_min) * obj_height;
                if (a * b < 0){
                    return true;
                }
            }
        }
        else if (obj_type == "1"){
            float a = obj_width * (center_y - obj_y_min - obj_height) + (center_x - obj_x_min) * obj_height;
            if (a == 0){
                return true;
            }
            else if (a > 0){
                float b = obj_width * (node_y - obj_y_min - obj_height) + (node_x - obj_x_min) * obj_height;
                if (a * b < 0){
                    return true;
                }
            }
            else if (a < 0){
                float b = obj_width * (node_y + width - obj_y_min - obj_height) + (node_x + width - obj_x_min) * obj_height;
                if (a * b < 0){
                    return true;
                }
            }

        }
        return false;
    }
    return false;
}

bool IsInPolygon(float x, float y, float ring_x[], float ring_y[], int len){
    int lcount = 0;
    float diff_x0 = ring_x[0] - x;
    float diff_y0 = ring_y[0] - y;
    for (int i = 1; i < len; i++){
        float diff_x1 = ring_x[i] - x;
        float diff_y1 = ring_y[i] - y;
        if (((diff_y0 <= 0) && (diff_y1 > 0)) || ((diff_y0 > 0) && (diff_y1 <= 0))){
            const float intersection = (diff_x1 * diff_y0 - diff_x0 * diff_y1) / (diff_y0 - diff_y1);
            if (intersection > 0){
                lcount++;
            }
        }
        diff_x0 = diff_x1;
        diff_y0 = diff_y1;
    }
    // When lcount is cardinal, it is in the polygon object
    if (lcount % 2 == 1){
        return true;
    } 
    
    return false;
}

bool IsInMBR(float x, float y, float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max){
    if (x < obj_x_min || x > obj_x_max || y < obj_y_min || y > obj_y_max){
        return false;
    }
    return true;
}


// Insert point objects
void VPQTreeNode::InsertPointObject(float point_x, float point_y){
    if (level == tqTree_level){
        return;
    }

    // UP_LEFT
    if (IsContainPoint(x, y+width/2, width/2, point_x, point_y)){
        if (!upLeftNode){
            upLeftNode = new VPQTreeNode(x, y+width/2, width/2, level+1, UP_LEFT, this);
        }
        upLeftNode->InsertPointObject(point_x, point_y);
        return;
    }
    // UP_RIGHT
    else if (IsContainPoint(x+width/2, y+width/2, width/2, point_x, point_y)){
        if (!upRightNode){
            upRightNode = new VPQTreeNode(x+width/2, y+width/2, width/2, level+1, UP_RIGHT, this);
        }
        upRightNode->InsertPointObject(point_x, point_y);
        return;
    }
    // BOTTOM_LEFT
    else if (IsContainPoint(x, y, width/2, point_x, point_y)){
        if (!bottomLeftNode){
            bottomLeftNode = new VPQTreeNode(x, y, width/2, level+1, BOTTOM_LEFT, this);
        }
        bottomLeftNode->InsertPointObject(point_x, point_y);
        return;
    }
    // BOTTOM_RIGHT
    else if (IsContainPoint(x+width/2, y, width/2, point_x, point_y)){
        if (!bottomRightNode){
            bottomRightNode = new VPQTreeNode(x+width/2, y, width/2, level+1, BOTTOM_RIGHT, this);
        }
        bottomRightNode->InsertPointObject(point_x, point_y);
        return;
    }
}

// Inserct line objects
void VPQTreeNode::InsertLineObject(float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max, std::string obj_type){
    if (level == tqTree_level){
        return;
    }

    // UP_LEFT
    if (IsContain(x, y+width/2, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max)){
        if (!upLeftNode){
            upLeftNode = new VPQTreeNode(x, y+width/2, width/2, level+1, UP_LEFT, this);
        }
        upLeftNode->InsertLineObject(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        return;
    }
    // UP_RIGHT
    else if (IsContain(x+width/2, y+width/2, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max)){
        if (!upRightNode){
            upRightNode = new VPQTreeNode(x+width/2, y+width/2, width/2, level+1, UP_RIGHT, this);
        }
        upRightNode->InsertLineObject(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        return;
    }
    // BOTTOM_LEFT
    else if (IsContain(x, y, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max)){
        if (!bottomLeftNode){
            bottomLeftNode = new VPQTreeNode(x, y, width/2, level+1, BOTTOM_LEFT, this);
        }
        bottomLeftNode->InsertLineObject(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        return;
    }
    // BOTTOM_RIGHT
    else if (IsContain(x+width/2, y, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max)){
        if (!bottomRightNode){
            bottomRightNode = new VPQTreeNode(x+width/2, y, width/2, level+1, BOTTOM_RIGHT, this);
        }
        bottomRightNode->InsertLineObject(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        return;
    }
    // When nodes in the next layer cannot contain objects, the objects are inserted into the nodes that intersect with them
    else{
        if (IsCoverNode(x, y+width/2, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type)){
            if (!upLeftNode){
                upLeftNode = new VPQTreeNode(x, y+width/2, width/2, level+1, UP_LEFT, this);
            }
            upLeftNode->InsertLineObject(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        }
        if (IsCoverNode(x+width/2, y+width/2, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type)){
            if (!upRightNode){
                upRightNode = new VPQTreeNode(x+width/2, y+width/2, width/2, level+1, UP_RIGHT, this);
            }
            upRightNode->InsertLineObject(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        }
        if (IsCoverNode(x, y, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type)){
            if (!bottomLeftNode){
                bottomLeftNode = new VPQTreeNode(x, y, width/2, level+1, BOTTOM_LEFT, this);
            }
            bottomLeftNode->InsertLineObject(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        }
        if (IsCoverNode(x+width/2, y, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type)){
            if (!bottomRightNode){
                bottomRightNode = new VPQTreeNode(x+width/2, y, width/2, level+1, BOTTOM_RIGHT, this);
            }
            bottomRightNode->InsertLineObject(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        }
    }
    return;
}

// Insert polygon segments
void VPQTreeNode::InsertPolygonSegment(float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max, std::string obj_type){
    if (level == tqTree_level){
        return;
    }

    // UP_LEFT
    if (IsContain(x, y+width/2, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max)){
        if (!upLeftNode){
            upLeftNode = new VPQTreeNode(x, y+width/2, width/2, level+1, UP_LEFT, this);
        }
        else if (upLeftNode->polygon_type == 'm'){
            upLeftNode->polygon_type = 's';
        }
        upLeftNode->InsertPolygonSegment(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        return;
    }
    // UP_RIGHT
    else if (IsContain(x+width/2, y+width/2, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max)){
        if (!upRightNode){
            upRightNode = new VPQTreeNode(x+width/2, y+width/2, width/2, level+1, UP_RIGHT, this);
        }
        else if (upRightNode->polygon_type == 'm'){
            upRightNode->polygon_type = 's';
        }
        upRightNode->InsertPolygonSegment(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        return;
    }
    // BOTTOM_LEFT
    else if (IsContain(x, y, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max)){
        if (!bottomLeftNode){
            bottomLeftNode = new VPQTreeNode(x, y, width/2, level+1, BOTTOM_LEFT, this);
        }
        else if (bottomLeftNode->polygon_type == 'm'){
            bottomLeftNode->polygon_type = 's';
        }
        bottomLeftNode->InsertPolygonSegment(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        return;
    }
    // BOTTOM_RIGHT
    else if (IsContain(x+width/2, y, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max)){
        if (!bottomRightNode){
            bottomRightNode = new VPQTreeNode(x+width/2, y, width/2, level+1, BOTTOM_RIGHT, this);
        }
        else if (bottomRightNode->polygon_type == 'm'){
            bottomRightNode->polygon_type = 's';
        }
        bottomRightNode->InsertPolygonSegment(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        return;
    }
    // When nodes in the next layer cannot contain objects, the objects are inserted into the nodes that intersect with them
    else{
        if (IsCoverNode(x, y+width/2, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type)){
            if (!upLeftNode){
                upLeftNode = new VPQTreeNode(x, y+width/2, width/2, level+1, UP_LEFT, this);
            }
            else if (upLeftNode->polygon_type == 'm'){
                upLeftNode->polygon_type = 's';
            }
            upLeftNode->InsertPolygonSegment(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        }
        if (IsCoverNode(x+width/2, y+width/2, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type)){
            if (!upRightNode){
                upRightNode = new VPQTreeNode(x+width/2, y+width/2, width/2, level+1, UP_RIGHT, this);
            }
            else if (upRightNode->polygon_type == 'm'){
                upRightNode->polygon_type = 's';
            }
            upRightNode->InsertPolygonSegment(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        }
        if (IsCoverNode(x, y, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type)){
            if (!bottomLeftNode){
                bottomLeftNode = new VPQTreeNode(x, y, width/2, level+1, BOTTOM_LEFT, this);
            }
            else if (bottomLeftNode->polygon_type == 'm'){
                bottomLeftNode->polygon_type = 's';
            }
            bottomLeftNode->InsertPolygonSegment(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        }
        if (IsCoverNode(x+width/2, y, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type)){
            if (!bottomRightNode){
                bottomRightNode = new VPQTreeNode(x+width/2, y, width/2, level+1, BOTTOM_RIGHT, this);
            }
            else if (bottomRightNode->polygon_type == 'm'){
                bottomRightNode->polygon_type = 's';
            }
            bottomRightNode->InsertPolygonSegment(obj_x_min, obj_y_min, obj_x_max, obj_y_max, obj_type);
        }
    }
    return;
}

// Insert Polygon Box
void VPQTreeNode::InsertPolygonBox(float box_x_min, float box_y_min, float box_x_max, float box_y_max, float ring_x[], float ring_y[], int len){
    
    if (level == tqTree_level || polygon_type == 'b'){
        return;
    }


    if (IsIntersect(x, y+width/2, width/2, box_x_min, box_y_min, box_x_max, box_y_max)){
        if (!upLeftNode){
            if (IsInMBR(x+width/4, y+3*width/4, box_x_min, box_y_min, box_x_max, box_y_max)){
                if (IsInPolygon(x+width/4, y+3*width/4, ring_x, ring_y, len)){
                    upLeftNode = new VPQTreeNode(x, y+width/2, width/2, level+1, UP_LEFT, this);
                    upLeftNode->polygon_type = 'b'; 
                }
            }
        }else{
            if (upLeftNode->polygon_type == 'm'){
                if (IsInPolygon(x+width/4, y+3*width/4, ring_x, ring_y, len))
                    upLeftNode->polygon_type = 'b'; 
            }
            else{
                upLeftNode->InsertPolygonBox(box_x_min, box_y_min, box_x_max, box_y_max, ring_x, ring_y, len);
            }
        }
    }

    if (IsIntersect(x+width/2, y+width/2, width/2, box_x_min, box_y_min, box_x_max, box_y_max)){
        if (!upRightNode){
            if (IsInMBR(x+3*width/4, y+3*width/4, box_x_min, box_y_min, box_x_max, box_y_max)){
                if (IsInPolygon(x+3*width/4, y+3*width/4, ring_x, ring_y, len)){
                    upRightNode = new VPQTreeNode(x+width/2, y+width/2, width/2, level+1, UP_RIGHT, this);
                    upRightNode->polygon_type = 'b';
                }
            }
        }else{
            if (upRightNode->polygon_type == 'm'){
                if (IsInPolygon(x+3*width/4, y+3*width/4, ring_x, ring_y, len))
                    upRightNode->polygon_type = 'b';
                
            }
            else{
                upRightNode->InsertPolygonBox(box_x_min, box_y_min, box_x_max, box_y_max, ring_x, ring_y, len);
            }
        } 
    }

    if (IsIntersect(x, y, width/2, box_x_min, box_y_min, box_x_max, box_y_max)){
        if (!bottomLeftNode){
            if (IsInMBR(x+width/4, y+width/4, box_x_min, box_y_min, box_x_max, box_y_max)){
                if (IsInPolygon(x+width/4, y+width/4, ring_x, ring_y, len)){
                    bottomLeftNode = new VPQTreeNode(x, y, width/2, level+1, BOTTOM_LEFT, this);
                    bottomLeftNode->polygon_type = 'b';
                }  
            }
        }else{
            if (bottomLeftNode->polygon_type == 'm'){
                if (IsInPolygon(x+width/4, y+width/4, ring_x, ring_y, len))
                    bottomLeftNode->polygon_type = 'b';
            }
            else{
                bottomLeftNode->InsertPolygonBox(box_x_min, box_y_min, box_x_max, box_y_max, ring_x, ring_y, len);
            }
        }
        
    }

    if (IsIntersect(x+width/2, y, width/2, box_x_min, box_y_min, box_x_max, box_y_max)){
        if (!bottomRightNode){
            if (IsInMBR(x+3*width/4, y+width/4, box_x_min, box_y_min, box_x_max, box_y_max)){
                if (IsInPolygon(x+3*width/4, y+width/4, ring_x, ring_y, len)){
                    bottomRightNode = new VPQTreeNode(x+width/2, y, width/2, level+1, BOTTOM_RIGHT, this);
                    bottomRightNode->polygon_type = 'b';
                } 
            }
            
        }else{
            if (bottomRightNode->polygon_type == 'm'){
                if (IsInPolygon(x+3*width/4, y+width/4, ring_x, ring_y, len))
                    bottomRightNode->polygon_type = 'b';
            }
            else{
                bottomRightNode->InsertPolygonBox(box_x_min, box_y_min, box_x_max, box_y_max, ring_x, ring_y, len);
            }
        }
    }

    return; 
}



// Build to the VPQ-tree node of the MBR containing a single polygon object
VPQTreeNode * VPQTreeNode::buildMBNode_polygon(float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max){
    
    // UP_LEFT
    if (IsContain(x, y+width/2, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max) && (level+1 <= tqTree_level)){
        if (!upLeftNode){
            upLeftNode = new VPQTreeNode(x, y+width/2, width/2, level+1, UP_LEFT, this);
            if (polygon_type == 'b')
                upLeftNode->polygon_type = 'b';
            else
                upLeftNode->polygon_type = 'm';
        }
        VPQTreeNode *seek = upLeftNode->buildMBNode(obj_x_min, obj_y_min, obj_x_max, obj_y_max);
        return seek;
    }

    // UP_RIGHT
    else if (IsContain(x+width/2, y+width/2, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max) && (level+1 <= tqTree_level)){
        if (!upRightNode){
            upRightNode = new VPQTreeNode(x+width/2, y+width/2, width/2, level+1, UP_RIGHT, this);
            if (polygon_type == 'b')
                upRightNode->polygon_type = 'b';
            else
                upRightNode->polygon_type = 'm';
        }
        VPQTreeNode *seek = upRightNode->buildMBNode(obj_x_min, obj_y_min, obj_x_max, obj_y_max);
        return seek;
    }
    // BOTTOM_LEFT
    else if (IsContain(x, y, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max) && (level+1 <= tqTree_level)){
        if (!bottomLeftNode){
            bottomLeftNode = new VPQTreeNode(x, y, width/2, level+1, BOTTOM_LEFT, this);
            if (polygon_type == 'b')
                bottomLeftNode->polygon_type = 'b';
            else 
                bottomLeftNode->polygon_type = 'm';
        }
        VPQTreeNode *seek = bottomLeftNode->buildMBNode(obj_x_min, obj_y_min, obj_x_max, obj_y_max);
        return seek;
    }
    // BOTTOM_RIGHT
    else if (IsContain(x+width/2, y, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max) && (level+1 <= tqTree_level)){
        if (!bottomRightNode){
            bottomRightNode = new VPQTreeNode(x+width/2, y, width/2, level+1, BOTTOM_RIGHT, this);
            if (polygon_type == 'b')
                bottomRightNode->polygon_type = 'b';
            else
                bottomRightNode->polygon_type = 'm';
        }
        VPQTreeNode *seek = bottomRightNode->buildMBNode(obj_x_min, obj_y_min, obj_x_max, obj_y_max);
        return seek;
    }
    else {
        return this;
    }
}

// Build to the VPQ-tree node of the MBR containing a single point or line object
VPQTreeNode * VPQTreeNode::buildMBNode(float obj_x_min, float obj_y_min, float obj_x_max, float obj_y_max){
    // UP_LEFT
    if (IsContain(x, y+width/2, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max) && (level+1 <= tqTree_level)){
        if (!upLeftNode){
            upLeftNode = new VPQTreeNode(x, y+width/2, width/2, level+1, UP_LEFT, this);
        }
        VPQTreeNode *seek = upLeftNode->buildMBNode(obj_x_min, obj_y_min, obj_x_max, obj_y_max);
        return seek;
    }

    // UP_RIGHT
    else if (IsContain(x+width/2, y+width/2, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max) && (level+1 <= tqTree_level)){
        if (!upRightNode){
            upRightNode = new VPQTreeNode(x+width/2, y+width/2, width/2, level+1, UP_RIGHT, this);
        }
        VPQTreeNode *seek = upRightNode->buildMBNode(obj_x_min, obj_y_min, obj_x_max, obj_y_max);
        return seek;
    }
    // BOTTOM_LEFT
    else if (IsContain(x, y, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max) && (level+1 <= tqTree_level)){
        if (!bottomLeftNode){
            bottomLeftNode = new VPQTreeNode(x, y, width/2, level+1, BOTTOM_LEFT, this);
        }
        VPQTreeNode *seek = bottomLeftNode->buildMBNode(obj_x_min, obj_y_min, obj_x_max, obj_y_max);
        return seek;
    }
    // BOTTOM_RIGHT
    else if (IsContain(x+width/2, y, width/2, obj_x_min, obj_y_min, obj_x_max, obj_y_max) && (level+1 <= tqTree_level)){
        if (!bottomRightNode){
            bottomRightNode = new VPQTreeNode(x+width/2, y, width/2, level+1, BOTTOM_RIGHT, this);
        }
        VPQTreeNode *seek = bottomRightNode->buildMBNode(obj_x_min, obj_y_min, obj_x_max, obj_y_max);
        return seek;
    }
    else {
        return this;
    }
}



// Output the VPQ-tree index to external memory
void VPQTreeNode::saveToFile(std::ofstream &outFile){
    if (this->level != 0){
        nodeInfo node(this->level, this->quadType); 
        outFile.write((char*)&node, sizeof(node));
    }
    if (upLeftNode == nullptr && upRightNode == nullptr && bottomLeftNode == nullptr && bottomRightNode == nullptr){
        return;
    }

    if (upLeftNode){
        upLeftNode->saveToFile(outFile);
    }
    if (upRightNode)
    {
        upRightNode->saveToFile(outFile);
    }
    if (bottomLeftNode){
        bottomLeftNode->saveToFile(outFile);
    }
    if (bottomRightNode){
        bottomRightNode->saveToFile(outFile);
    }

}


void VPQTreeNode::saveToFile_polygon(std::ofstream &outFile){
    if (this->level != 0){
        nodeInfoPolygon node_polygon(this->level, this->quadType, this->polygon_type); 
        outFile.write((char*)&node_polygon, sizeof(node_polygon));
    }
    if (upLeftNode == nullptr && upRightNode == nullptr && bottomLeftNode == nullptr && bottomRightNode == nullptr){
        return;
    }

    if (upLeftNode){
        upLeftNode->saveToFile_polygon(outFile);
    }
    if (upRightNode)
    {
        upRightNode->saveToFile_polygon(outFile);
    }
    if (bottomLeftNode){
        bottomLeftNode->saveToFile_polygon(outFile);
    }
    if (bottomRightNode){
        bottomRightNode->saveToFile_polygon(outFile);
    }
}


