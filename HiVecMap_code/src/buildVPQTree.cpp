#include "hicore/core.hpp"
#include "hicore/core_creator.hpp"
#include "hicore/core_iterator.hpp"
#include "hicore/shp.hpp"
#include "buildVPQTree.hpp"
#include "VPQTreeNode.hpp"
#include <math.h>
#include <sys/time.h>
#include <fstream>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace HiGIS::IO;
using namespace HiGIS::Core;

#define pi 3.14159265358979323
#define L 20037508.3427892
#define tolerance 0.00001


// coordinate transformation
void coordTran(double &x ,double &y){
    x = x * 20037508.34 / 180;
    y = (log(tan(((90 + y) * pi) / 360)) / (pi / 180)) * 20037508.34 / 180;
}


// build VPQTree of point
VPQTreeNode * buildPointVPQTree(std::string data_path[], int len, std::string file_path){
    struct timeval	t1, t2;
	gettimeofday(&t1, NULL);

    // create VPQTree 
    // 1. create root node ptr, dynamic allocate malloc 
    VPQTreeNode *pointVPQTree = new VPQTreeNode(-L, -L, 2 * L, 0, ROOT, nullptr);
    // 2. insert point object
    Shapefile shp;
    int data_count = 0;
    for (int i = 0; i < len; i++){
        GeoData data = shp.Read(data_path[i]);
        PointIterator ptIter(data);
        do{
            // create new object ptr, insert object to VPQ-tree
            double x = ptIter.X();
            double y = ptIter.Y();
            coordTran(x, y);
            pointVPQTree->InsertPointObject(x, y);
        }while (ptIter.NextFeature() >= 0);
        data_count += data.feature_count;
    }

    ofstream indexFile(file_path, ios::out | ios::binary);
    pointVPQTree->saveToFile(indexFile);
    
    gettimeofday(&t2, NULL);
    float time_use = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    cout << "---------------------- build index ----------------------------" << endl;
    cout << "the num of point is: " << data_count << std::endl;
    cout << "the time of build VPQ-tree is: " << time_use <<  " s" << endl;
    cout << "---------------------------------------------------------------" << endl;
    return pointVPQTree;
}



// build VPQTree of line
VPQTreeNode * buildLineVPQTree(std::string data_path[], int len, std::string file_path){
    struct timeval	t1, t2;
	gettimeofday(&t1, NULL);
    // create VPQ-tree 
    // 1. create root node ptr
    VPQTreeNode *lineVPQTree = new VPQTreeNode(-L, -L, 2 * L, 0, ROOT, nullptr);
    // 2. insert line object
    int feature_count = 0;
    int segment_count = 0;
    Shapefile shp;

    for (int k = 0; k < len; k++){
        GeoData data = shp.Read(data_path[k]);
        LineStringIterator lineIter(data);
        do{
            int nodes_num = lineIter.NodeCount();
            float line_x[nodes_num];
            float line_y[nodes_num];
            double *nodes = lineIter.Nodes();
            double x0 = lineIter.X(nodes, 0);
            double y0 = lineIter.Y(nodes, 0);
            coordTran(x0, y0);
            line_x[0] = x0;
            line_y[0] = y0;
            // get MBR of each feature
            double x_min = x0;
            double x_max = x0;
            double y_min = y0;
            double y_max = y0;
            
            for (int i = 1; i < nodes_num; i++){
                double xi = lineIter.X(nodes, i);
                double yi = lineIter.Y(nodes, i);
                coordTran(xi, yi);
                line_x[i] = xi;
                line_y[i] = yi;
                x_min = std::min(x_min, xi);
                x_max = std::max(x_max, xi);
                y_min = std::min(y_min, yi);
                y_max = std::max(y_max, yi); 
            }

            VPQTreeNode *MBNode = lineVPQTree->buildMBNode(x_min, y_min, x_max, y_max);

            // cut line to segment and insert to VPQ-tree
            for (int j = 0; j < nodes_num - 1; j++){
                double x0 = line_x[j];
                double y0 = line_y[j];
                double x1 = line_x[j + 1];
                double y1 = line_y[j + 1];
                float obj_x_min = std::min(x0, x1);
                float obj_x_max = std::max(x0, x1);
                float obj_y_min = std::min(y0, y1);
                float obj_y_max = std::max(y0, y1);
                if (((x1 >= x0) && (y1 >= y0)) or ((x1 <= x0) && (y1 <= y0))){
                    MBNode->InsertLineObject(obj_x_min, obj_y_min, obj_x_max, obj_y_max, "0");
                }
                else if (((x1 < x0) && (y1 > y0)) or ((x1 > x0) && (y1 < y0))){
                    MBNode->InsertLineObject(obj_x_min, obj_y_min, obj_x_max, obj_y_max, "1");
                }
            }
            segment_count += (nodes_num - 1);


        }while(lineIter.NextFeature() >= 0);
        feature_count += data.feature_count;
    }
    
    ofstream indexFile(file_path, ios::out | ios::binary);
    lineVPQTree->saveToFile(indexFile);

    gettimeofday(&t2, NULL);
    float time_use = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    cout << "---------------------- build index ----------------------------" << endl;
    cout << "the num of the line is: " << feature_count << endl;
    cout << "the num of the finely line is: " << segment_count << endl;
    cout << "the time of build VPQ-tree is: " << time_use <<  " s" << endl;
    cout << "---------------------------------------------------------------" << endl;
    return lineVPQTree;
}



// build VPQTree of polygon
VPQTreeNode * buildPolygonVPQTree(std::string data_path[], int len, std::string file_path){
    struct timeval	t1, t2;
	gettimeofday(&t1, NULL);

    // create VPQ-tree 
    // 1. create root node ptr, dynamic allocate malloc 
    VPQTreeNode *polygonVPQTree = new VPQTreeNode(-L, -L, 2 * L, 0, ROOT, nullptr);

    // 2. insert line object
    int feature_count = 0;
    int segment_count = 0;
    Shapefile shp;

    for (int m = 0; m < len; m++){
        GeoData data = shp.Read(data_path[m]);
        PolygonIterator polygonIter(data);
        do{
            // get the polygon part
            for (int n = 0; n < polygonIter.PartCount(); n++){
                auto poly = polygonIter.Part(n);
                // get the ring of each part
                for (int k = 0; k < polygonIter.RingCount(poly); k++){
                    auto ring = polygonIter.Ring(poly, k);
                    auto nodes = polygonIter.Nodes(ring);
                    int nodes_num = polygonIter.NodeCount(ring);
                    float ring_x[nodes_num];
                    float ring_y[nodes_num];

                    // for every node of each ring
                    double x0 = polygonIter.X(nodes, 0);
                    double y0 = polygonIter.Y(nodes, 0);
                    coordTran(x0, y0);
                    ring_x[0] = x0;
                    ring_y[0] = y0;

                    double x_min = x0;
                    double x_max = x0;
                    double y_min = y0;
                    double y_max = y0;
                    for (int i = 1; i < nodes_num; i++){
                        double xi = polygonIter.X(nodes, i);
                        double yi = polygonIter.Y(nodes, i);
                        coordTran(xi, yi);
                        ring_x[i] = xi;
                        ring_y[i] = yi;
                        x_min = std::min(x_min, xi);
                        x_max = std::max(x_max, xi);
                        y_min = std::min(y_min, yi);
                        y_max = std::max(y_max, yi); 
                    }
                    VPQTreeNode *MBNode = polygonVPQTree->buildMBNode_polygon(x_min, y_min, x_max, y_max);

                    for (int j = 0; j < nodes_num - 1; j++){
                        double x0 = ring_x[j];
                        double y0 = ring_y[j];
                        double x1 = ring_x[j + 1];
                        double y1 = ring_y[j + 1];
                        float obj_x_min = std::min(x0, x1);
                        float obj_x_max = std::max(x0, x1);
                        float obj_y_min = std::min(y0, y1);
                        float obj_y_max = std::max(y0, y1);
                        if (((x1 >= x0) && (y1 >= y0)) or ((x1 <= x0) && (y1 <= y0))){
                            MBNode->InsertPolygonSegment(obj_x_min, obj_y_min, obj_x_max, obj_y_max, "0");
                        }
                        else if (((x1 < x0) && (y1 > y0)) or ((x1 > x0) && (y1 < y0))){
                            MBNode->InsertPolygonSegment(obj_x_min, obj_y_min, obj_x_max, obj_y_max, "1");
                        }
                    }
                    MBNode->InsertPolygonBox(x_min, y_min, x_max, y_max, ring_x, ring_y, nodes_num);
                    segment_count += nodes_num;
                }
            }
        }while(polygonIter.NextFeature() >= 0);
        feature_count += data.feature_count;
    }

    std::ofstream indexFile(file_path, std::ios::out | std::ios::binary);
    polygonVPQTree->saveToFile_polygon(indexFile);
    
    gettimeofday(&t2, NULL);
    float time_use = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    cout << "---------------------- build index ----------------------------" << endl;
    cout << "the num of the polygon is: " << feature_count << endl;
    cout << "the num of the finely segment is: " << segment_count << endl;
    cout << "the time of build VPQ-tree is: " << time_use <<  " s" << endl;
    cout << "---------------------------------------------------------------" << endl;
    return polygonVPQTree;
}




