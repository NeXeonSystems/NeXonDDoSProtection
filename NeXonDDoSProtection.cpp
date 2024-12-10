#include <iostream>
#include <unordered_map>
#include <mutex>
#include <ctime>
#include <chrono>
#include <thread>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>

std::unordered_map<std::string, int> requestCounts;
std::unordered_map<std::string, std::time_t> blockedClients;
std::mutex requestMutex;

int rateLimit = 100;
int timeWindow = 60;
int blockDuration = 300;

void blockIPWithIptables(const std::string& ip) {
    std::string cmd = "sudo iptables -A INPUT -s " + ip + " -j DROP";
    system(cmd.c_str());
}

void logAttack(const std::string& clientIp) {
    std::ofstream logFile("ddos_attacks.log", std::ios_base::app);
    logFile << "Detected DDoS attempt from IP: " << clientIp
            << " at " << std::time(nullptr) << std::endl;
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
            logAttack(clientIp);
            blockIPWithIptables(clientIp);
            close(clientSocket);
            return;
        }
    }

    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
    }
    close(clientSocket);
}

void resetRequestCounts() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(timeWindow));
        std::lock_guard<std::mutex> lock(requestMutex);
        requestCounts.clear();
    }
}

void startProtection() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);
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
    std::thread(resetRequestCounts).detach();
    startProtection();
    return 0;
}
