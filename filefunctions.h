#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <sstream>
using namespace std;

// Turn anything into a C++ string.
template <class T>
inline string to_string(const T& t) {
  stringstream stream;
  stream << t;
  return stream.str();
}

// Read a file into a C++ string.
string read_file(string name) {
  ifstream t(name);
  stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

// Overwrite a file with a C++ string.
void write_file(string name, string contents) {
  ofstream file(name, ofstream::trunc);
  file << contents;
}

// Most of the curl download/file writing code was modified from the cURL examples.
// cURL Copyright (C) 1998 - 2011, Daniel Stenberg, <daniel@haxx.se>, et al.

// Helpful struct for file writing.
struct file_struct {
  const char *filename;
  FILE *stream;
};

// Function to write the downloaded file from memory to the disk.
static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
  struct file_struct *out=(struct file_struct *)stream;
  if(out && !out->stream) {
    out->stream=fopen(out->filename, "wb");
    if(!out->stream)
      return -1;
  }
  return fwrite(buffer, size, nmemb, out->stream);
}

// Function to download a file and write it to the disk.
void download(const char* URL, const char* filename, bool verbose)
{
  CURL *curl;
  CURLcode res;
  struct file_struct file = {filename, NULL};
  
  curl_global_init(CURL_GLOBAL_DEFAULT);
  
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    
    if(verbose)
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
    res = curl_easy_perform(curl);
    
    curl_easy_cleanup(curl);
    
    if(CURLE_OK != res)
      fprintf(stderr, "cURL error: %d\n", res);
  }
  
  if(file.stream)
    fclose(file.stream);
  
  curl_global_cleanup();
}