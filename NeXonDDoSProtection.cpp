#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <ctime>
#include <chrono>
#include <thread>
#include <vector>
#include <curl/curl.h>
#include <sstream>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>

std::unordered_map<std::string, int> requestCounts;
std::unordered_map<std::string, std::time_t> blockedClients;
std::mutex requestMutex;
std::string targetUrl;
int rateLimit, timeWindow, proxyPort, blockDuration;

void loadEnv() {
    std::ifstream envFile(".env");
    std::string line;
    while (std::getline(envFile, line)) {
        auto delimiterPos = line.find('=');
        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);
        if (key == "TARGET_URL") targetUrl = value;
        else if (key == "RATE_LIMIT") rateLimit = std::stoi(value);
        else if (key == "TIME_WINDOW") timeWindow = std::stoi(value);
        else if (key == "PROXY_PORT") proxyPort = std::stoi(value);
        else if (key == "BLOCK_DURATION") blockDuration = std::stoi(value);
    }
}

size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

void forwardRequest(const std::string& clientRequest, int clientSocket) {
    CURL* curl = curl_easy_init();
    if (!curl) return;
    curl_easy_setopt(curl, CURLOPT_URL, targetUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, clientRequest.c_str());
    curl_easy_perform(curl);
    write(clientSocket, response.c_str(), response.size());
    curl_easy_cleanup(curl);
}

void handleClient(int clientSocket, sockaddr_in clientAddr) {
    char buffer[8192];
    std::string clientIp = inet_ntoa(clientAddr.sin_addr);
    {
        std::lock_guard<std::mutex> lock(requestMutex);
        if (blockedClients.count(clientIp) && std::time(0) - blockedClients[clientIp] < blockDuration) {
            close(clientSocket);
            return;
        }
        auto& count = requestCounts[clientIp];
        count++;
        if (count > rateLimit) {
            blockedClients[clientIp] = std::time(0);
            close(clientSocket);
            return;
        }
    }
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) forwardRequest(std::string(buffer, bytesReceived), clientSocket);
    close(clientSocket);
}

void resetRequestCounts() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(timeWindow));
        std::lock_guard<std::mutex> lock(requestMutex);
        requestCounts.clear();
    }
}

void startProxy() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(proxyPort);
    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 128);
    while (true) {
        sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrLen);
        std::thread(handleClient, clientSocket, clientAddr).detach();
    }
    close(serverSocket);
}

int main() {
    loadEnv();
    std::thread(resetRequestCounts).detach();
    startProxy();
    return 0;
}
