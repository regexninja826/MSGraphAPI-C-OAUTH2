#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <json-c12/json.h>
#include <curl/curl.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t

WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

const char * gettoken(void){

  const char* AUTHURI = getenv("AUTHURI");
  const char* OAUTHPOSTDATA = getenv("OAUTHPOSTDATA");
  struct MemoryStruct chunk;
  chunk.memory = malloc(1);
  chunk.size = 0;
  const char* token;
  CURL *curl_handle;
  CURLcode res;
  curl_global_init(CURL_GLOBAL_ALL);
  curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_URL,AUTHURI);
  curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS,OAUTHPOSTDATA);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  res = curl_easy_perform(curl_handle);
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
    curl_easy_strerror(res));
  }
  else {
        json_object * new_obj = json_tokener_parse(chunk.memory);
        json_object *obj_token;
        obj_token = json_object_object_get(new_obj,"access_token");
        token = json_object_get_string(obj_token);
  }
  curl_easy_cleanup(curl_handle);
  curl_global_cleanup();
  free(chunk.memory);
  return token;

}


int main(int argc, char *argv[]){

  struct curl_slist *chunk_curl = NULL;
  char* bearer_str = "Authorization: Bearer ";
  char* token = strdup(gettoken());
  char *graphuri = argv[1];
  struct MemoryStruct chunk;
  chunk.memory = malloc(2);
  chunk.size = 0;
  const int MAX_BUF = 2000;
  char* Buffer = malloc(MAX_BUF);
  int length = 0;
  CURL *curl_handle;
  CURLcode res;

  length += snprintf(Buffer+length, MAX_BUF-length, bearer_str);
  length += snprintf(Buffer+length, MAX_BUF-length, token);
    
  chunk_curl = curl_slist_append(chunk_curl,"Accept: application/json");
  chunk_curl = curl_slist_append(chunk_curl,"Content-Type: application/x-www-form-urlencoded"); 
  chunk_curl = curl_slist_append(chunk_curl,Buffer);

  curl_global_init(CURL_GLOBAL_ALL);
  curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, chunk_curl);
  curl_easy_setopt(curl_handle, CURLOPT_URL, graphuri);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  res = curl_easy_perform(curl_handle);
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
    curl_easy_strerror(res));
  }
  curl_easy_cleanup(curl_handle);
  curl_global_cleanup();
  printf("%s",chunk.memory);  
  free(chunk.memory);
  
  return 0;
}


