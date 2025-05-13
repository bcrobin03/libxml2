#include <libxml/parser.h>
#include <libxml/xmlIO.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <libxml/catalog.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlmemory.h>

#include "private/lint.h"

#include "fuzz.h"



// Custom handler: simulate coverage of custom input logic
int myMatch(const char *URI) {
    return 1;  // Always matches
}

void *myOpen(const char *URI) {
    return NULL;  // Simulate broken handler for coverage
}

int myRead(void *ctx, char *buf, int len) {
    return -1;
}

int myClose(void *ctx) {
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0 || data[size - 1] != '\0') return 0; // Ensure null-terminated input

    static int initialized = 0;
    if (!initialized) {
        xmlRegisterInputCallbacks(myMatch, myOpen, myRead, myClose);
        initialized = 1;
    }

    xmlParserInputBufferPtr out = NULL;
    xmlParserInputBufferCreateUrl((const char *)data, XML_CHAR_ENCODING_NONE, 0, &out);

    if (out) {
        xmlFreeParserInputBuffer(out);
    }

    return 0;
}