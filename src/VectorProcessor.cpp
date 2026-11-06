#include "VectorProcessor.h"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
bool VectorProcessor::readExact(int sockfd, void *buf, size_t len) {
  char *ptr = static_cast<char *>(buf);
  size_t total = 0;
  while (total < len) {
    ssize_t r = recv(sockfd, ptr + total, len - total, 0);
    if (r <= 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        std::cout
            << "[ERROR] Socket timeout/non-blocking! Waiting for data failed."
            << std::endl;
      } else if (errno == ECONNRESET) {
        std::cout << "[ERROR] Connection reset by client." << std::endl;
      }
      return false;
    }
    total += r;
  }
  return true;
}
bool VectorProcessor::writeExact(int sockfd, const void *buf, size_t len) {
  const char *ptr = static_cast<const char *>(buf);
  size_t total = 0;
  while (total < len) {
    ssize_t w = send(sockfd, ptr + total, len - total, 0);
    if (w <= 0)
      return false;
    total += w;
  }
  return true;
}
std::vector<std::vector<uint16_t>> VectorProcessor::receiveVectors(int sockfd) {
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  uint32_t numVectors;
  if (!readExact(sockfd, &numVectors, sizeof(numVectors))) {
    throw std::runtime_error("Failed to read number of vectors");
  }
  if (numVectors > 10000) {
    throw std::runtime_error("Invalid number of vectors: " + std::to_string(numVectors));
  }
  std::vector<std::vector<uint16_t>> vectors(numVectors);
  for (uint32_t i = 0; i < numVectors; ++i) {
    uint32_t size;
    if (!readExact(sockfd, &size, sizeof(size))) {
      throw std::runtime_error("Failed to read vector size #" + std::to_string(i));
    }
    if (size > 1000000) {
      throw std::runtime_error("Suspicious vector size: " + std::to_string(size));
    }
    vectors[i].resize(size);
    if (size > 0) {
      size_t bytesToRead = size * sizeof(uint16_t);
      if (!readExact(sockfd, vectors[i].data(), bytesToRead)) {
        throw std::runtime_error("Failed to read vector data #" + std::to_string(i));
      }
    }
    uint16_t avg = average(vectors[i]);
    sendResult(sockfd, avg);
  }
  return vectors;
}
uint16_t VectorProcessor::average(std::vector<uint16_t> vec) {
  if (vec.empty())
    return 0;
  uint32_t sum = 0;
  for (uint16_t val : vec) {
    sum += val;
  }
  uint32_t avg = sum / vec.size();
  return saturate(avg);
}
uint16_t VectorProcessor::saturate(uint32_t value) {
  if (value > 65535)
    return 65535;
  return static_cast<uint16_t>(value);
}
void VectorProcessor::sendResult(int sockfd, uint16_t result) {
  if (!writeExact(sockfd, &result, sizeof(result))) {
    throw std::runtime_error("Failed to send result");
  }
}