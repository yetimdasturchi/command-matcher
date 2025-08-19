#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"

#define SUCCESS 0

typedef char* (*StringFunction)();
typedef int (*IntFunction)(char*);

typedef struct {
    char* (*get_domain)();
    char* (*get_operational_label)();
    char* (*get_original)();
    char* (*get_answer)();
    char* (*get_json)();
} CommandFunctions;

void* loadFunction(void *handle, const char *funcName) {
    dlerror();
    void *funcPtr = dlsym(handle, funcName);
    const char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, RED "Error loading %s: %s\n" RESET, funcName, error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }
    return funcPtr;
}

void testInitRes(void *handle) {
    printf(YELLOW "Initializing library...\n" RESET);
    IntFunction initRes = (IntFunction)loadFunction(handle, "init_res");
    if (initRes("./static/") != SUCCESS) {
        fprintf(stderr, RED "Error: Failed to initialize library!\n" RESET);
        exit(EXIT_FAILURE);
    }
    printf(GREEN "OK: Library initialized successfully!\n" RESET);
}

void testGetFromText(void *handle) {
    printf(YELLOW "Testing command processing...\n" RESET);
    IntFunction getFromText = (IntFunction)loadFunction(handle, "get_from_text");
    char * input = "chiroqni yoq";
    if (getFromText(input) != SUCCESS) {
        fprintf(stderr, RED "Error: Command processing failed!\n" RESET);
        exit(EXIT_FAILURE);
    }
    printf(GREEN "OK!\n" RESET);
    printf(CYAN "-------------------\n" RESET);
    printf(YELLOW "Input: %s\n" RESET, input);
}

void testResponses(void *handle) {
    CommandFunctions command;
    command.get_domain = (StringFunction)loadFunction(handle, "get_domain");
    command.get_operational_label = (StringFunction)loadFunction(handle, "get_operational_label");
    command.get_original = (StringFunction)loadFunction(handle, "get_original");
    command.get_answer = (StringFunction)loadFunction(handle, "get_answer");
    command.get_json = (StringFunction)loadFunction(handle, "get_json");

    printf("Domain: %s\n", command.get_domain());
    printf("Operational label: %s\n", command.get_operational_label());
    printf("Index: %s\n", command.get_original());
    printf("Answer: %s\n", command.get_answer());
    printf("Json: %s\n", command.get_json());
}

int main() {
    void *handle = dlopen("./out/libmanulu.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, RED "Error: %s\n" RESET, dlerror());
        return 1;
    }

    printf(CYAN "-------------------\n" RESET);
    testInitRes(handle);
    printf(CYAN "-------------------\n" RESET);
    
    testGetFromText(handle);
    printf(CYAN "-------------------\n" RESET);
    testResponses(handle);

    dlclose(handle);
    return 0;
}