#Wrapper around a Thread object to use low-level tricks as binding a thread to a CPU.

To compile and link it, run the commands :

> make javac
> make javah
> make gcc

To run it (with as argument the number of threads wanted), run the following :

> make java ARGS="<nb_thread>"
