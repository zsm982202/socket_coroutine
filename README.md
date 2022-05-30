# socket_coroutine
## 运行

make

./bin/main

## redis-benchmark压力测试

redis-benchmark -p 8888 -n 100000 -c 50 -q

可以达到每秒11W的并发量
