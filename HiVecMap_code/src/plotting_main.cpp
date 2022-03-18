#include "hicore/core.hpp"
#include "hicore/core_creator.hpp"
#include "hicore/core_iterator.hpp"
#include "hicore/shp.hpp"
#include <iostream>
#include <omp.h>
#include <mpi.h>
#include "plotTile.hpp"
#include "VPQTreeNodeS.hpp"
#include "buildVPQTree.hpp"
#include "Redis.h"
#include "sys/stat.h"

#define MAX_TILE_PARAMS 256
#define TILE_SIZE	256
#define L		20037508.34

using namespace std;
using namespace HiGIS::IO;
using namespace HiGIS::Core;



string split(string shp_name){
    string shpName;
    stringstream shpName_stream(shp_name);
    while (getline(shpName_stream, shpName, '/')){
    }
    int ix = shpName.find(".");
    string output_shpName = shpName.substr(0, ix);
    return output_shpName;
}

void GetList( char* argv, char* result[], char* flag, int & count )
{
	char	* string = strdup( argv );
	char	* p;
	int	i = 0;
	while ( (p = strsep( &string, flag ) ) != NULL )
	{
		result[i] = p;
		i++;
	}
	result[i]	= string;
	count		= i;
}


int main(int argc, char **argv){
    // mpi params
    int myId, numProcs;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myId);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    // get params
    string point_indexName = argv[1]; 
	string line_indexName = argv[2]; 
	string polygon_indexName = argv[3]; 
    string output_path = argv[4];   // absolute path of index file
    char *redisHost	= argv[5];         /* Host IP of Redis */
	int	redisPort = atoi(argv[6]); 

    int	x, y, z;
	string shpType;
	int count = 0;
    char *tile_params[MAX_TILE_PARAMS];
    char *task = new char[256];
	char *tile_area = (char *) malloc(TILE_SIZE * TILE_SIZE);
	char *buffer_tmp = (char *)malloc(TILE_SIZE * TILE_SIZE);
    // connect redis
    Redis *redis = new Redis();
    if (!redis->connect(redisHost, redisPort))
    {
        cout << "connect redis error!" << endl;
        exit(0);
        MPI_Finalize();
    }

    if (myId == 0)
	{
        cout << "-----------------------------------------------------------" << endl;
		cout << "Service Start.cores: " << numProcs << endl;
		cout << "-----------------------------------------------------------" << endl;
	}

	string point_indexPath = output_path + point_indexName;
	string line_indexPath = output_path + line_indexName;
	string polygon_indexPath = output_path + polygon_indexName;
    struct stat fileStat;


	VPQTreeNodeS *pointVPQTree = new VPQTreeNodeS(0, nullptr);
	if (stat(point_indexPath.c_str(), &fileStat) == 0){
		pointVPQTree->readIndexFile(point_indexPath);
		if (myId == 0){
			cout << "<<<<<<< read point index: " << point_indexName << " successfully >>>>>>>" << endl;
		}
	}
	else{
		cout << "<<<<<<< point index doesn't exit >>>>>>>" << endl;
	}
	cout << "-----------------------------------------------------------" << endl;
	
	VPQTreeNodeS *lineVPQTree = new VPQTreeNodeS(0, nullptr);
	if (stat(line_indexPath.c_str(), &fileStat) == 0){
		lineVPQTree->readIndexFile(line_indexPath);
		if (myId == 0){
			cout << "<<<<<<< read line index: " << line_indexName << " successfully >>>>>>>" << endl;
		}
	}
	else{
		cout << "<<<<<<< line index doesn't exit >>>>>>>" << endl;
	}
	cout << "-----------------------------------------------------------" << endl;
	
	VPQTreeNodeS *polygonVPQTree = new VPQTreeNodeS(0, nullptr);
	if (stat(polygon_indexPath.c_str(), &fileStat) == 0){
		polygonVPQTree->readIndexFile_polygon(polygon_indexPath);
		if (myId == 0){
			cout << "<<<<<<< read polygon index: " << polygon_indexName << " successfully >>>>>>>" << endl;
		}
	}
	else{
		cout << "<<<<<<< polygon index doesn't exit >>>>>>>" << endl;
	}
	cout << "-----------------------------------------------------------" << endl;
	

	// receive task and render matrix
    while(1){
		sprintf(task, "%s", redis->brpop("tilelist").c_str());
		try{
			if (strlen(task) > 0){
                GetList(task, tile_params, (char *) "/", count);
				
				shpType = tile_params[0];
				z = atoi(tile_params[1]);
				x = atoi(tile_params[2]);
				y = atoi(tile_params[3]);
				
				double t1 = MPI_Wtime();
					
				if (shpType == "point"){
					VPQTreeNodeS *tile_node = locateTileNode(z, x, y, pointVPQTree);
					if (tile_node != nullptr){
						pointUpResolutionSample(z, x, y, tile_node, pointVPQTree, tile_area);
					}
					else{
						# pragma omp parallel for num_threads(4)
						for (int i = 0; i < TILE_SIZE; i++){
							for (int j = 0; j < TILE_SIZE; j++){
								int tile_index = i * 256 + j;
								tile_area[tile_index] = 0;
							}
						}
					}
				}
				else if (shpType == "line"){
					VPQTreeNodeS *tile_node = locateTileNode(z, x, y, lineVPQTree);
					if (tile_node != nullptr){
						lineUpResolutionSample(z, x, y, tile_node, lineVPQTree, tile_area);
					}
					else{
						# pragma omp parallel for num_threads(4)
						for (int i = 0; i < TILE_SIZE; i++){
							for (int j = 0; j < TILE_SIZE; j++){
								int tile_index = i * 256 + j;
								tile_area[tile_index] = 0;
							}
						}
					}
				}
				else if (shpType == "polygon"){
					polygonUpResolutionSample(z, x, y, polygonVPQTree, tile_area);
				}	

				redis->zset(task, tile_area, TILE_SIZE * TILE_SIZE);  
				while (!redis->zget(task, buffer_tmp)){             
					redis->zset(task, tile_area, TILE_SIZE * TILE_SIZE); 
				}
				redis->expire(task, "10");
				redis->pub("HiVecMapTiles", task);   

				double t2 = MPI_Wtime();
				
            }
		}
		catch (...){
			cout << "Error task: " << task << endl;
		}
	}

    delete pointVPQTree;
	delete lineVPQTree;
	delete polygonVPQTree;
    pointVPQTree = NULL;
	lineVPQTree = NULL;
	polygonVPQTree = NULL;
	MPI_Finalize();
}