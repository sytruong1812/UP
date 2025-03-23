#include "api_test.h"


static void llist_dtor(void* key, void* value) {
	(void)key;
	(void)value;
}
CURLcode TEST_llist()
{
	struct Curl_llist llist;
	Curl_llist_init(&llist, llist_dtor);

	/*===========================[Insert]=========================*/
	int index1 = 1;
	struct Curl_llist_node node1;
	int index2 = 2;
	struct Curl_llist_node node2;
	int index3 = 3;
	struct Curl_llist_node node3;

	Curl_llist_insert_next(&llist, Curl_llist_head(&llist), &index1, &node1);
	Curl_llist_insert_next(&llist, Curl_llist_head(&llist), &index2, &node2);
	Curl_llist_insert_next(&llist, Curl_llist_head(&llist), &index3, &node3);

	/*===========================[Append]=========================*/
	int index4 = 4;
	struct Curl_llist_node node4;
	Curl_llist_append(&llist, &index4, &node4);

	/*===========================[Remove]=========================*/
	int llsize = Curl_llist_count(&llist);
	auto head = Curl_llist_head(&llist);
	auto next1 = Curl_node_next(head);
	auto next2 = Curl_node_next(next1);
	auto next3 = Curl_node_next(next2);
	Curl_node_remove(next3);

	Curl_llist_destroy(&llist, NULL);

	return CURLE_OK;
}

void hash_dtor(void* p) {
	int* ptr = (int*)p;
	free(p);
}
CURLcode TEST_hash() {
	struct Curl_hash hash;
	Curl_hash_init(&hash, 3, Curl_hash_str, Curl_str_key_compare, hash_dtor);

	char key1[] = "key1";	char value1[] = "value1";
	char key2[] = "key2";	char value2[] = "value2";
	char key3[] = "key3";	char value3[] = "value3";

	char* nodep;
	char* pick;

	nodep = (char*)Curl_hash_add(&hash, &key1, strlen(key1), &value1);
	nodep = (char*)Curl_hash_add(&hash, &key2, strlen(key2), &value2);
	nodep = (char*)Curl_hash_add(&hash, &key3, strlen(key3), &value3);

	size_t count = Curl_hash_count(&hash);

	pick = (char*)Curl_hash_pick(&hash, &key2, strlen(key2));

	Curl_hash_destroy(&hash);
	return CURLE_OK;
}

void TEST_parse_url() {
	CURLUcode uc;
	CURLU* h = curl_url();
	CURLUcode rc = curl_url_set(h, CURLUPART_URL, "http://example.com:449/foo/bar?name=moo", 0);
	rc = curl_url_set(h, CURLUPART_HOST, "example.net", 0);

	char* host;
	/* extract hostname from the parsed URL */
	uc = curl_url_get(h, CURLUPART_HOST, &host, 0);
	if (!uc)
	{
		printf("Host name: %s\n", host);
	}

	char* post;
	/* extract post from the parsed URL */
	uc = curl_url_get(h, CURLUPART_PORT, &post, 0);
	if (!uc)
	{
		printf("Post: %s\n", post);
	}

	char* path;
	/* extract the path from the parsed URL */
	uc = curl_url_get(h, CURLUPART_PATH, &path, 0);
	if (!uc)
	{
		printf("Path: %s\n", path);
	}
	curl_free(host);
	curl_free(post);
	curl_free(path);
	curl_url_cleanup(h); /* free URL handle */
}
void TEST_redirect_url() {
	CURLU* h = curl_url();
	CURLUcode rc = curl_url_set(h, CURLUPART_URL, "http://example.com/foo/bar?name=moo", 0);
	rc = curl_url_set(h, CURLUPART_URL, "../test?another", 0);
	curl_url_cleanup(h); /* free URL handle */
}
void TEST_idn(const char* host_name) {
	CURLcode result = CURLE_OK;
	struct hostname h;
	h.rawalloc = strdup(host_name ? host_name : "");
	h.name = h.rawalloc;
	result = Curl_idnconvert_hostname(&h);
}