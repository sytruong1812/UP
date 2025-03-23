#pragma once
#include "hash.h"
#include "urlapi.h"

CURLcode TEST_llist();
CURLcode TEST_hash();

void TEST_parse_url();
void TEST_redirect_url();
void TEST_idn(const char* host_name);