g++ -ggdb `pkg-config --cflags opencv` -o `basename main.cpp .cpp`.o main.cpp `pkg-config --libs opencv` -lboost_thread
