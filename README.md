**BUILD**

1. mkdir build && cd build
2. cmake ../
3. make
  
**RUN**

1. Server run ./remote_dictionary_server -a "127.0.0.5" -p 15000
2. Client run ./remote_dictionary_client -a "127.0.0.5" -p 15000 -t 12 -r 10000

**HELP**

Server and client have help 
./remote_dictionary_client -h
./remote_dictionary_server -h