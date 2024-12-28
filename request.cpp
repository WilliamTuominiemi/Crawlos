#include <iostream>
#include <string>
#include <curl/curl.h>
#include <gumbo.h>
#include <vector>

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
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "Failed to fetch URL: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
    return response;
}

void extractLinks(GumboNode *node, std::vector<std::string> &links)
{
    if (node->type != GUMBO_NODE_ELEMENT)
        return;

    if (node->v.element.tag == GUMBO_TAG_A)
    {
        GumboAttribute *href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href)
        {
            links.push_back(href->value);
        }
    }

    const GumboVector *children = &node->v.element.children;
    for (size_t i = 0; i < children->length; ++i)
    {
        extractLinks(static_cast<GumboNode *>(children->data[i]), links);
    }
}

std::vector<std::string> parseHTML(const std::string &html)
{
    GumboOutput *output = gumbo_parse(html.c_str());
    std::vector<std::string> links;
    extractLinks(output->root, links);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    return links;
}

int main()
{
    std::string url = "https://crawler-test.com/links/page_with_external_links";
    std::cout << "Fetching URL: " << url << std::endl;

    std::string content = fetchURL(url);
    if (content.empty())
    {
        std::cerr << "Failed to fetch content" << std::endl;
        return 1;
    }

    std::vector<std::string> links = parseHTML(content);

    std::cout << "\nFound " << links.size() << " links:" << std::endl;
    for (const auto &link : links)
    {
        std::cout << link << std::endl;
    }

    return 0;
}