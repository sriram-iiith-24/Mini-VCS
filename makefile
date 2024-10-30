all: mygit

mygit: mygit.cpp init.cpp utilities.cpp hash_object.cpp header.h
	g++ -Wall -std=c++17 mygit.cpp init.cpp utilities.cpp hash_object.cpp cat_file.cpp write_tree.cpp ls_tree.cpp add.cpp commit.cpp log.cpp checkout.cpp -o mygit -lssl -lcrypto -lzstd

clean:
	rm -f mygit