all:
	gcc \
	-o main \
	./src/main.c \
	./src/glb.c \
	./src/utils.c \
	-I./include \
	-IE:/cpplibs/cJSON/usr/include \
	-LE:/cpplibs/cJSON/usr/lib \
	-lcjson \
	-g -Wall -Werror