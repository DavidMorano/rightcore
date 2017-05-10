
gcc -o te -I${HOME}/include te.c sfcookkey.c expandcookie.c -L${HOME}/lib -ldebug -ldam -luc -lu -lsecdb -lproject -lrt -lpthread -lxnet -lsocket -lnsl 
