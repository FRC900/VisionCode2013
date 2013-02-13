g++ -ggdb `pkg-config --cflags opencv` -o `basename test.cpp .cpp`.o test.cpp `pkg-config --libs opencv` -lboost_thread
