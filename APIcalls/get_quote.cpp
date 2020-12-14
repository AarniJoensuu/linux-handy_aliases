// built-in libraries
#include <iostream>

// 3rd-party libraries
#include <curl/curl.h> // -lcurl
#include <jsoncpp/json/json.h> // -ljsoncpp

static size_t WriteCallback(
  void *contents,
  size_t size,
  size_t nmemb,
  void *userp)
/* Function receives cURL response and returns it */
{
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

std::string extractFromJSON(std::string json)
/* Function takes a JSON-like string and
 * extracts its "content" and "author"
 * keys, which are then concatenated into
 * a human-readable string. */
{
  Json::Reader reader;
  Json::Value obj;
  reader.parse(json, obj);
  std::string result = obj["content"].asString() +
    " -" + obj["author"].asString();
  return result;
}

int main(void) {
  CURL *curl;
  CURLcode result;
  std::string readBuffer;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://api.quotable.io/random");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    result = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    std::string quote_with_author = extractFromJSON(readBuffer);
    std::cout << quote_with_author << std::endl;
  }
  return 0;
}
