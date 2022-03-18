#include <iostream>
#include "crow.h"
#include <png.h>
#include "Redis.h"

using namespace std;
#define TILE_SIZE 256
#define L	20037508.34




class HiVecMapPara
{
public:
	string dataPath = "";         
	string outputPath = "";        
	int	servicePort;        
	char *redisHost = NULL;
	int redisPort;
};
HiVecMapPara para;



class ServerLogHandler : public crow::ILogHandler {
public:
	void log( std::string /*message*/, crow::LogLevel /*level*/ ) override
	{
	}
};


struct ServerMiddleware
{
	std::string message;

	ServerMiddleware()
	{
		message = "foo";
	}


	void setMessage( std::string newMsg )
	{
		message = newMsg;
	}


	struct context
	{
	};

	void before_handle( crow::request & /*req*/, crow::response & /*res*/, context & /*ctx*/ )
	{
		CROW_LOG_DEBUG << " - MESSAGE: " << message;
	}


	void after_handle( crow::request & /*req*/, crow::response & /*res*/, context & /*ctx*/ )
	{
		/* no-op */
	}
};



int main(int argc, char **argv){
    crow::App<ServerMiddleware> app;

    CROW_ROUTE(app, "/HiVecMap/<string>/<int>/<int>/<int>/<double>/<int>/<int>/<int>.png" ).name("HiVecMap")
		([] (const crow::request & req, crow::response & res, string shpType, int R, int G, int B, double AD, int z, int x, int y) {

			int A = AD * 64;
			int AH = AD * 256;
			int Ax = (1 - AD) * 64;
			int Ay = - 64;
			long size;
			vector<char> pos;
			char *task = new char[32];
			char *new_task = new char[32];
			char *redisHost = "127.0.0.1";
			int redisPort = 6379;

			// connect redis
			Redis *redis = new Redis();
			if (!redis->connect(redisHost, redisPort))
			{
				printf( "connect redis error!\n" );
			}

            png_bytep * row_pointers = (png_bytep *) malloc(256 * sizeof(png_bytep));
			for (int n = 0; n < TILE_SIZE; n++)
				row_pointers[n] = (png_bytep) malloc(1024);

			// lpush z/x/y to redis
            try{
				sprintf(task, "%s/%d/%d/%d", shpType.c_str(), z, x, y);
				char (*buffer_area)[TILE_SIZE] = (char(*)[TILE_SIZE])malloc( TILE_SIZE * TILE_SIZE );
				if (!redis->zget(task, (char *) buffer_area)){
					memset( new_task, 0, 32 );
					redisContext* rc = redisConnect( redisHost, redisPort );
					redisReply* reply = (redisReply *) redisCommand( rc, "subscribe HiVecMapTiles" );
					redis->lpush( "tilelist", task);
					while ( strcmp( new_task, task) != 0 )
					{
						if ( redisGetReply( rc, (void * *) &reply ) == REDIS_OK && reply->type == REDIS_REPLY_ARRAY )
							sprintf( new_task, "%s", reply->element[2]->str );
					}
					redis->zget( task, (char *) buffer_area );
					freeReplyObject( reply );
					redisFree( rc );
				}

				png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
				png_infop info_ptr = png_create_info_struct(png_ptr); 
				FILE *temp_png = tmpfile();
				png_init_io(png_ptr, temp_png); 
				png_set_IHDR(png_ptr, info_ptr, TILE_SIZE, TILE_SIZE, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
				png_write_info(png_ptr, info_ptr); 

				if (shpType == "point" or shpType == "line"){
					for (int i = 0; i < TILE_SIZE; i++){
						for (int j = 0; j < TILE_SIZE; j++){
							if (buffer_area[i][j] >= 4){
								row_pointers[i][4 * j] = R;     /* red */
								row_pointers[i][4 * j + 1] = G; /* green */
								row_pointers[i][4 * j + 2] = B; /* blue */
								row_pointers[i][4 * j + 3] = AH + Ax * (buffer_area[i][j] - 4) - 1;
							}else if (buffer_area[i][j] > 0){
								row_pointers[i][4 * j] = R;     /* red */
								row_pointers[i][4 * j + 1] = G; /* green */
								row_pointers[i][4 * j + 2] = B; /* blue */
								row_pointers[i][4 * j + 3] = buffer_area[i][j] * A - 1;
							}else  {
								row_pointers[i][4 * j] = 0;     /* red */
								row_pointers[i][4 * j + 1] = 0; /* green */
								row_pointers[i][4 * j + 2] = 0; /* blue */
								row_pointers[i][4 * j + 3] = 0;
							}
						}
					}
				}
				else if (shpType == "polygon"){
					for (int i = 0; i < TILE_SIZE; i++){
						for (int j = 0; j < TILE_SIZE; j++){
							if (buffer_area[i][j] >= 4){
								row_pointers[i][4 * j] = 255;     /* red */
								row_pointers[i][4 * j + 1] = 0; /* green */
								row_pointers[i][4 * j + 2] = 0; /* blue */
								row_pointers[i][4 * j + 3] = AH + Ax * (buffer_area[i][j] - 4) - 1;
							}else if (buffer_area[i][j] > 0){
								row_pointers[i][4 * j] = 255;     /* red */
								row_pointers[i][4 * j + 1] = 0; /* green */
								row_pointers[i][4 * j + 2] = 0; /* blue */
								row_pointers[i][4 * j + 3] = buffer_area[i][j] * A - 1;
							}
							else if (buffer_area[i][j] == -1){
								row_pointers[i][4 * j] = R;     /* red */
								row_pointers[i][4 * j + 1] = G; /* green */
								row_pointers[i][4 * j + 2] = B; /* blue */
								row_pointers[i][4 * j + 3] = AH + Ax * (buffer_area[i][j] - 4) - 1;
							}
							else  {
								row_pointers[i][4 * j] = 0;     /* red */
								row_pointers[i][4 * j + 1] = 0; /* green */
								row_pointers[i][4 * j + 2] = 0; /* blue */
								row_pointers[i][4 * j + 3] = 0;
							}
						}
					}
					
				}
				

				free(buffer_area); 
				png_write_image(png_ptr, row_pointers);   
				png_write_end(png_ptr, NULL);
				fseek(temp_png, 0, SEEK_END);  
				size = ftell(temp_png); 
				rewind(temp_png); 
				pos.resize(size);
				fread(&pos[0], 1, size, temp_png); 
				fclose(temp_png);

				string pos_tostr = string(pos.begin(), pos.end());
				res.write(pos_tostr);
				res.set_header("Content-Type", "image/png" );
				res.set_header("Access-Control-Allow-Origin", "*");

				
				// free memory
				redis->freeredis();
				delete task;
				delete new_task;
				for (int k = 0; k < 256; k++)
					free(row_pointers[k]);
				free(row_pointers);
				res.end();

            }
            catch(const char* msg){
				redis->freeredis();
				delete task;
				delete new_task;

                for (int k = 0; k < 256; k++)
					free(row_pointers[k]);
				free(row_pointers);
				res.code = 500;
                ostringstream os;
				os << msg << "\n";
				res.write(os.str());
				res.set_header("Content-Type", "text/html");
				res.end();
            }
        });

    app.port(10085).multithreaded().run();
}