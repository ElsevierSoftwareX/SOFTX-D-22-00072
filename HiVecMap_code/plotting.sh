#!/bin/bash
point_indexName="china_mainland_poi"
line_indexName="china_mainland_road"
polygon_indexName="china_mainland_province"
output_path="./output/"
process=1
redishost="127.0.0.1"
redisport=6379

redis-server &
nohup ./build/tile_server > ./TMS_server.log 2>&1 &
nohup mpirun -np $process ./build/plotting $point_indexName $line_indexName $polygon_indexName $output_path $redishost $redisport > ./plotting.log 2>&1 &
python3 ./flask/release.py

