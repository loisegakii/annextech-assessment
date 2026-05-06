#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
 
using json = nlohmann::json;
 
/* ── libcurl write callback ─────────────────────────────────────────────── */
static size_t writeCallback(void* contents, size_t size, size_t nmemb,
                             std::string* output) {
    size_t total = size * nmemb;
    output->append(static_cast<char*>(contents), total);
    return total;
}
 
/* ── Fetch a single URL and return the response body as a string ─────────── */
static std::string fetchURL(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialise libcurl");
 
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL,           url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        30L);
 
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
 
    if (res != CURLE_OK)
        throw std::runtime_error(std::string("curl error: ") +
                                 curl_easy_strerror(res));
    return response;
}
 
/* ── Extract articles from one page of JSON data ────────────────────────── */
static void extractArticles(const json& pageData,
                             std::vector<std::pair<int, std::string>>& articles) {
    for (const auto& item : pageData["data"]) {
        /* Determine article name */
        std::string name;
        if (!item["title"].is_null()) {
            name = item["title"].get<std::string>();
        } else if (!item["story_title"].is_null()) {
            name = item["story_title"].get<std::string>();
        } else {
            continue;   /* both null → skip */
        }
 
        /* num_comments: treat null as 0 */
        int comments = 0;
        if (!item["num_comments"].is_null()) {
            comments = item["num_comments"].get<int>();
        }
 
        articles.emplace_back(comments, name);
    }
}
 
/* ── Main function ───────────────────────────────────────────────────────── */
std::vector<std::string> topArticles(int limit) {
    const std::string baseURL =
        "https://jsonmock.hackerrank.com/api/articles?page=";
 
    /* Fetch page 1 to learn total_pages */
    json firstPage = json::parse(fetchURL(baseURL + "1"));
    int totalPages = firstPage["total_pages"].get<int>();
 
    std::vector<std::pair<int, std::string>> articles;
 
    extractArticles(firstPage, articles);
 
    for (int page = 2; page <= totalPages; ++page) {
        json pageData = json::parse(fetchURL(baseURL + std::to_string(page)));
        extractArticles(pageData, articles);
    }
 
    /* Sort:
     *   1. num_comments descending
     *   2. article name alphabetically descending (tie-break) */
    std::sort(articles.begin(), articles.end(),
        [](const std::pair<int, std::string>& a,
           const std::pair<int, std::string>& b) {
            if (a.first != b.first) return a.first > b.first;   /* more comments first */
            return a.second > b.second;                          /* reverse alpha */
        });
 
    /* Return the top `limit` names */
    std::vector<std::string> result;
    int count = std::min(limit, static_cast<int>(articles.size()));
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.push_back(articles[i].second);
    }
    return result;
}
 
/* ── Entry point ─────────────────────────────────────────────────────────── */
int main() {
    int limit;
    std::cout << "Enter limit: ";
    std::cin >> limit;
 
    try {
        std::vector<std::string> results = topArticles(limit);
        for (const auto& name : results) {
            std::cout << name << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}