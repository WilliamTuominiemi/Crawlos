#include <iostream>
#include <string>
#include <curl/curl.h>

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *data)
{
    data->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::string fetchURL(const std::string &url)
{
    CURL *curl = curl_easy_init();
    std::string response;
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

int main()
{
    std::string url = "https://crawler-test.com/links/page_with_external_links";
    std::cout << "Fetching URL: " << url << std::endl;

    std::string content = fetchURL(url);

    std::cout << "Content fetched:" << std::endl;
    std::cout << content << std::endl;

    return 0;
}
