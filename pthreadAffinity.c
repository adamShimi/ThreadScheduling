#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <sched.h>
#include "ThreadScheduling.h"

int nb_cpu;
JavaVM* jvm;
pthread_mutex_t* mutexes;
jobject* slaves;

//Init function, launched the first time the JNI is called
JNIEXPORT jint JNI_OnLoad(JavaVM* trueJvm, void* reserved) {
    //Cache the jvm
    jvm = trueJvm;

    //Get CPU number and allocate mutexes and slaves in consequences
    nb_cpu = get_nprocs();
    printf("Using %d CPUs\n", nb_cpu);
    mutexes = malloc(sizeof(pthread_mutex_t)*nb_cpu);
    slaves = malloc(sizeof(jobject)*nb_cpu);

    //Init the mutexes
    int i;
    for(i = 0; i < nb_cpu; i++) {
        pthread_mutex_init(&mutexes[i], NULL);
    }

    return JNI_VERSION_1_6;
}

//Pthread start function, which launch the Java thread inside
void* startSlave(void* jaffinity) {

    jmethodID start;
    jmethodID join;

    //Cast back affinity
    jint* affinity = (jint*) jaffinity;
    //Lock the slave jobject
    pthread_mutex_lock(&mutexes[*affinity]);

    //Get a new JNIEnv* and add the current thread in the JVM
    JNIEnv* env;
    (**jvm).AttachCurrentThread(jvm, (void **)&env, NULL);
    (**jvm).GetEnv(jvm, (void **)&env, JNI_VERSION_1_6);

    //Extract start and join methods from the slave jobject
    jclass slaveClass = (*env)->GetObjectClass(env, slaves[*affinity]);
    while((start = (*env)->GetMethodID(env, slaveClass, "start", "()V"))==NULL){}
    while((join = (*env)->GetMethodID(env, slaveClass, "join", "()V"))==NULL){}

    //Start and join the slave
    (*env)->CallVoidMethod(env, slaves[*affinity], start);
    (*env)->CallVoidMethod(env, slaves[*affinity], join);

    pthread_mutex_unlock(&mutexes[*affinity]);

    //Detach the thread from the JVM
    (**jvm).DetachCurrentThread(jvm);
    pthread_exit(NULL);
}

//Variable exporting function
JNIEXPORT jint JNICALL Java_ThreadScheduling_getCPUNumber(JNIEnv *env, jobject thisObj) {
    return (jint) nb_cpu;
}

//Set affinity and launch a pthread with startSlave as a start function
JNIEXPORT void JNICALL Java_ThreadScheduling_setAffinity(JNIEnv *env, jobject thisObj) {

    //Declaring variables
    pthread_t pthread;
    void* status;
    cpu_set_t cpu_set;
    jfieldID affinityField;
    jfieldID slaveField;

    //Initializing cpu_set
    CPU_ZERO(&cpu_set);

    //Take the instance variable affinity to create the wanted cpu_set
    jclass threadAffClass = (*env)->GetObjectClass(env, thisObj);
    while((affinityField=(*env)->GetFieldID(env, threadAffClass, "affinity", "I"))==NULL){}
    jint affinity = (*env)->GetIntField(env, thisObj, affinityField);

    //Create the wanted cpu_set
    CPU_SET(affinity, &cpu_set);
    sched_setaffinity(pthread, 1, &cpu_set);

    //Obtain the thread instance variable
    while((slaveField=(*env)->GetFieldID(env, threadAffClass, "thread", "LSlave;"))==NULL){}
    jobject jslave = (*env)->GetObjectField(env, thisObj, slaveField);
    slaves[affinity] = (*env)->NewGlobalRef(env, jslave);

    int returnCode = pthread_create(&pthread, NULL, startSlave, (void*) &affinity);
    if(returnCode) {
        printf("pthread_create failed with error code : %d", returnCode);
        exit(-1);
    }

    pthread_join(pthread, &status);


}

