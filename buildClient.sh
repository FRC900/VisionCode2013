g++ -ggdb `pkg-config --cflags opencv` -o `basename client.cpp .cpp`.o client.cpp `pkg-config --libs opencv` -lboost_thread
