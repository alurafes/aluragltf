all:
	gcc -o main ./src/main.c -I./include -IE:/cpplibs/cJSON/usr/include -LE:/cpplibs/cJSON/usr/lib -lcjson -g -Wall -Werror