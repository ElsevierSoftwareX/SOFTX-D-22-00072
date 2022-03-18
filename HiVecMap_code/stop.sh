ps -ef | grep tile_server | grep -v grep| awk '{print "kill -9 " $2}'| sh
ps -ef | grep plotting | grep -v grep| awk '{print "kill -9 " $2}'| sh
ps -ef | grep python | grep -v grep| awk '{print "kill -9 " $2}'| sh
ps -ef | grep redis-server | grep -v grep| awk '{print "kill -9 " $2}'| sh

