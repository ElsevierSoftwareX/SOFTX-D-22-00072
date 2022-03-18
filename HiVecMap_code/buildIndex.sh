#!/bin/bash
################## point shapfile ##################
shp_fileName="./data/china_mainland_poi"
shp_name="china_mainland_poi.shp"
################## line shapfile ##################
# shp_fileName="./data/china_mainland_road"
# shp_name="china_mainland_road.shp"
################## polygon shapefile ##################
# shp_fileName="./data/china_mainland_province"
# shp_name="china_mainland_province.shp"
output_path="./output/"

./build/buildIndex $shp_fileName $shp_name $output_path
