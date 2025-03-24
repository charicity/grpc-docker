protoc -I. -I/usr/local/include/googleapis --include_imports \
 --include_source_info   --descriptor_set_out=helloworld.pb   helloworld.proto