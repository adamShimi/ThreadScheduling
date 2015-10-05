gcc: pthreadAffinity.c
	gcc -Wall -Wextra -pthread -fpic -I/usr/lib/jvm/java-1.7.0-openjdk-amd64/include -c pthreadAffinity.c
	gcc -shared -o libpthreadAffinity.so pthreadAffinity.o

javac: ThreadScheduling.java Slave.java
	javac ThreadScheduling.java

javah: ThreadScheduling.class Slave.class
	javah -jni ThreadScheduling

java:
	java -Djava.library.path=/home/hardkey/Java/ThreadAffinity/ThreadScheduling/ ThreadScheduling ${ARGS}
