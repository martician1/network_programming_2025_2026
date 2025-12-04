#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>

#define DEFAULT_PORT 5555
#define MAX_N 1024
#define MAX_M 16384
#define MAX_K 100
#define MAX_EDGE_WEIGHT 16384

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <server-ip[:server-port]>\n";
		return EXIT_FAILURE;
	}

	int port = DEFAULT_PORT;
	const char *p;

	for (p = argv[1]; *p; ++p) {
		if (*p != ':') {
			continue;
		}

		port = atoi(p + 1);
		if (port <= 0 || port > (long)UINT16_MAX) {
			std::cerr << "Invalid port\n";
			return EXIT_FAILURE;
		}
		break;
	}

	std::string ipRaw{argv[1], (size_t)(p - argv[1])};
	sockaddr_in addr;
	if (inet_pton(AF_INET, ipRaw.c_str(), &addr.sin_addr) != 1) {
		std::cerr << "Invalid ip address\n";
		return EXIT_FAILURE;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons((uint16_t)port);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		std::cerr << "Failed to create socket\n";
		return EXIT_FAILURE;
	}

	try {
		if (connect(sock, (sockaddr *)(&addr), sizeof(addr)) < 0) {
			throw std::runtime_error(
				"Failed to establish connection with the server\n");
		}

		std::cout << "Successfully connected to " << argv[1] << '\n';
		std::cout << "Input parameters for Yen's algorithm:\n"
				  << "Number of vertices: " << std::flush;

		std::cin.exceptions(std::istream::badbit | std::istream::failbit);

		uint32_t n;
		std::cin >> n;

		if (n == 0 || n > MAX_N) {
			throw std::invalid_argument("Invalid argument");
		}

		std::cout << "Number of edges: " << std::flush;
		uint32_t m;
		std::cin >> m;

		if (m > MAX_M || m > n * (n - 1) / 2) {
			throw std::invalid_argument("Invalid argument");
		}

		std::cout << "Number of shortest paths to find: " << std::flush;
		uint32_t k;
		std::cin >> k;

		if (k > MAX_K) {
			throw std::invalid_argument("Invalid argument");
		}

		std::cout << "Source vertex: " << std::flush;
		uint32_t s;
		std::cin >> s;

		if (s >= n) {
			throw std::invalid_argument("Invalid argument");
		}

		std::cout << "Destination vertex: " << std::flush;
		uint32_t t;
		std::cin >> t;

		if (t >= n) {
			throw std::invalid_argument("Invalid argument");
		}

		std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> edges;
		for (size_t i = 0; i < m; ++i) {
			std::cout << "Edge " << i << ": " << std::flush;
			uint32_t a, b, w;
			std::cin >> a;
			std::cin >> b;
			std::cin >> w;

			if (a >= n || b >= n || w > MAX_EDGE_WEIGHT) {
				throw std::invalid_argument("Invalid argument");
			}
			edges.emplace_back(a, b, w);
		}

		std::vector<uint32_t> input;

		input.emplace_back(htonl(n));
		input.emplace_back(htonl(m));
		input.emplace_back(htonl(k));
		input.emplace_back(htonl(s));
		input.emplace_back(htonl(t));

		for (size_t i = 0; i < m; ++i) {
			input.emplace_back(htonl(std::get<0>(edges[i])));
			input.emplace_back(htonl(std::get<1>(edges[i])));
			input.emplace_back(htonl(std::get<2>(edges[i])));
		}

		size_t inputBufferLen = input.size() * sizeof(uint32_t);
		uint8_t *inputBuffer = (uint8_t *)input.data();
		size_t sentSoFar = 0;

		do {
			ssize_t bytesSent = send(sock, inputBuffer + sentSoFar,
									 inputBufferLen - sentSoFar, 0);
			if (bytesSent <= 0) {
				throw std::runtime_error("Could not send data to server");
			}
			sentSoFar += (size_t)bytesSent;

			assert(sentSoFar <= inputBufferLen);
		} while (sentSoFar < inputBufferLen);

		uint32_t resultBufferLen;
		size_t receivedSoFar = 0;

		do {
			ssize_t bytesReceived =
				recv(sock, ((uint8_t *)(&resultBufferLen)) + receivedSoFar,
					 sizeof(uint32_t) - receivedSoFar, 0);
			if (bytesReceived <= 0) {
				throw std::runtime_error(
					"Failed to receive result from server");
			}
			receivedSoFar += (size_t)bytesReceived;
		} while (receivedSoFar < sizeof(uint32_t));

		resultBufferLen = ntohl(resultBufferLen);
		assert(resultBufferLen % sizeof(uint32_t) == 0);
		std::vector<uint32_t> result(resultBufferLen / sizeof(uint32_t));

		uint8_t *resultBuffer = (uint8_t *)result.data();
		receivedSoFar = 0;

		do {
			ssize_t bytesReceived = recv(sock, resultBuffer + receivedSoFar,
										 resultBufferLen - receivedSoFar, 0);
			if (bytesReceived <= 0) {
				throw std::runtime_error(
					"Failed to receive result from server");
			}
			receivedSoFar += (size_t)bytesReceived;
		} while (receivedSoFar < resultBufferLen);

		std::cout << "Server result:\n";

		for (auto e : result) {
			uint32_t v = ntohl(e);
			std::cout << v << ((v == t) ? '\n' : ' ');
		}
	} catch (std::exception &e) {
		std::cerr << e.what() << '\n';
		close(sock);
		return EXIT_FAILURE;
	}

	close(sock);
	return EXIT_SUCCESS;
}
