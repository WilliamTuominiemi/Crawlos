#include <iostream>
#include <string>
#include <curl/curl.h>
#include <gumbo.h>
#include <vector>
#include <queue>
#include <set>

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
            std::string url = std::string(href->value);

            if (url.substr(0, 4) == "http" && url.find('.') != std::string::npos)
            {
                if (url.back() == '/')
                {
                    url.pop_back();
                }
                links.push_back(url);
            }
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

void crawl(const std::string &startURL, int maxDepth)
{
    std::queue<std::pair<std::string, int>> urlQueue;
    std::set<std::string> visited;

    urlQueue.push({startURL, 0});
    visited.insert(startURL);

    while (!urlQueue.empty())
    {
        auto [currentURL, depth] = urlQueue.front();
        urlQueue.pop();

        if (depth > maxDepth)
            continue;

        std::cout << "\nCrawling: " << currentURL << " at depth " << depth << std::endl;

        std::string content = fetchURL(currentURL);
        if (content.empty())
        {
            std::cerr << "Failed to fetch content for: " << currentURL << std::endl;
            continue;
        }

        std::vector<std::string> links = parseHTML(content);

        std::cout << "Found " << links.size() << " links:" << std::endl;
        for (const auto &link : links)
        {
            std::cout << "  " << link << std::endl;

            if (visited.find(link) == visited.end())
            {
                urlQueue.push({link, depth + 1});
                visited.insert(link);
            }
        }
    }

    std::cout << "\nCrawling complete. Visited " << visited.size() << " URLs." << std::endl;
}

int main()
{
    std::string startURL = "https://webscraper.io/test-sites/e-commerce/allinone";
    int maxDepth = 2;

    std::cout << "Starting crawler from: " << startURL << std::endl;
    crawl(startURL, maxDepth);

    return 0;
}
