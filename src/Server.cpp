#include "Server.h"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(UserInterface &ui)
    : port(ui.getPort()), clientDB(ui.getDbFile()), logFile(ui.getLogFile()),
      ui(ui), auth(), processor(), serverSocket(-1) {}

bool Server::readLine(int sockfd, std::string &out) {
  out = "";
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  char c;
  while (true) {
    ssize_t r = recv(sockfd, &c, 1, 0);

    if (r > 0) {
      if (c == '\n' || c == '\r') {
        break;
      }
      out += c;
    } else if (r == 0) {
      logFile.log("readLine: Client disconnected", Severity::WARNING);
      break;
    } else {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      } else {
        logFile.log("readLine: recv error: " + std::string(strerror(errno)),
                    Severity::CRITICAL);
        return false;
      }
    }
  }
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  return !out.empty();
}

bool Server::writeLine(int sockfd, const std::string &msg) {
  std::string data = msg;
  return send(sockfd, data.c_str(), data.length(), 0) == (ssize_t)data.length();
}

void Server::start() {
  if (!clientDB.load()) {
    logFile.log("Failed to load client database from " + ui.getDbFile(),
                Severity::CRITICAL);
    return;
  }
  logFile.log("Client database loaded successfully", Severity::INFO);

  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    logFile.log("Socket creation error", Severity::CRITICAL);
    return;
  }

  int opt = 1;
  setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
    logFile.log("Bind error on port " + std::to_string(port),
                Severity::CRITICAL);
    close(serverSocket);
    return;
  }

  if (listen(serverSocket, 3) < 0) {
    logFile.log("Listen error", Severity::CRITICAL);
    close(serverSocket);
    return;
  }

  logFile.log("Server started on port " + std::to_string(port), Severity::INFO);

  while (true) {
    acceptConnection();
  }

  close(serverSocket);
}

void Server::acceptConnection() {
  struct sockaddr_in clientAddr;
  socklen_t addrlen = sizeof(clientAddr);
  int clientSocket =
      accept(serverSocket, (struct sockaddr *)&clientAddr, &addrlen);

  if (clientSocket < 0) {
    logFile.log("Accept connection error", Severity::WARNING);
    return;
  }

  logFile.log("Client connected", Severity::INFO);

  try {
    std::string login;
    if (!readLine(clientSocket, login))
      throw std::runtime_error("Failed to read login");

    if (!clientDB.userExists(login)) {
      writeLine(clientSocket, "ERR");
      throw std::runtime_error("User not found: " + login);
    }

    uint64_t salt = auth.generateSalt();
    std::string salt16 = auth.formatSalt16(salt);
    if (!writeLine(clientSocket, salt16))
      throw std::runtime_error("Failed to send salt");

    std::string receivedHash;
    if (!readLine(clientSocket, receivedHash))
      throw std::runtime_error("Failed to read hash");

    std::string password = clientDB.getPassword(login);
    if (!auth.verifyHash(salt16, password, receivedHash)) {
      writeLine(clientSocket, "ERR");
      throw std::runtime_error("Authentication failed for user: " + login);
    }

    if (!writeLine(clientSocket, "OK"))
      throw std::runtime_error("Failed to send OK");

    try {

      auto vectors = processor.receiveVectors(clientSocket);
      logFile.log("Session completed successfully. Processed " + std::to_string(vectors.size()) + " vectors for user: " + login, Severity::INFO);
    } catch (const std::exception &e) {
      logFile.log(std::string("Session error: ") + e.what(), Severity::WARNING);
    }

    logFile.log("Session completed successfully for user: " + login,
                Severity::INFO);

  } catch (const std::exception &e) {
    logFile.log(std::string("Session error: ") + e.what(), Severity::WARNING);
  }

  close(clientSocket);
  logFile.log("Client disconnected", Severity::INFO);
}