#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* key;
    char* value;
} ConfigPair;

typedef struct {
    ConfigPair* pairs;
    int count;
} ConfigData;

ConfigData readConfigFile(const char* filePath) {
    ConfigData config;
    config.pairs = NULL;
    config.count = 0;

    FILE* file = fopen(filePath, "r");
    if (!file) {
        return config;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* equalSign = strchr(line, '=');
        if (equalSign) {
            *equalSign = '\0';
            char* key = line;
            char* value = equalSign + 1;
            while (*value && (*value == '\r' || *value == '\n')) {
                *value = '\0';
                value++;
            }

            config.pairs = (ConfigPair*)realloc(config.pairs, (config.count + 1) * sizeof(ConfigPair));
            config.pairs[config.count].key = strdup(key);
            config.pairs[config.count].value = strdup(value);
            config.count++;
        }
    }

    fclose(file);
    return config;
}

void freeConfigData(ConfigData* config) {
    for (int i = 0; i < config->count; i++) {
        free(config->pairs[i].key);
        free(config->pairs[i].value);
    }
    free(config->pairs);
    config->pairs = NULL;
    config->count = 0;
}

int main() {
    ConfigData config = readConfigFile("config.txt");
    for (int i = 0; i < config.count; i++) {
        printf("Key: %s, Value: %s\n", config.pairs[i].key, config.pairs[i].value);
    }
    freeConfigData(&config);
    return 0;
}



