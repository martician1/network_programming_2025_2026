#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utility>
#include <vector>

#define DEFAULT_PORT 5555
#define MAX_N 1024
#define MAX_M 16384
#define MAX_K 100
#define MAX_EDGE_WEIGHT 16384

#define MAX_CONNECTIONS 8

std::pair<int64_t, std::vector<uint32_t>>
dijkstra(std::vector<std::vector<int32_t>> &adj, uint32_t s, uint32_t t) {
	const size_t n = adj.size();

	std::vector<uint64_t> cost(n, UINT64_MAX);
	std::vector<uint32_t> parent(n, UINT32_MAX);
	std::vector<bool> visited(n, false);

	cost[s] = 0;
	uint32_t v = s;

	while (v != UINT32_MAX && v != t) {
		visited[v] = true;

		for (uint32_t u = 0; u < n; ++u) {
			if (adj[v][u] == -1 || ((uint32_t)adj[v][u]) + cost[v] >= cost[u]) {
				continue;
			}
			cost[u] = ((uint32_t)adj[v][u]) + cost[v];
			parent[u] = v;
		}

		v = UINT32_MAX;
		for (uint32_t u = 0; u < n; ++u) {
			if (!visited[u] && cost[u] != UINT64_MAX &&
				(v == UINT32_MAX || cost[v] > cost[u])) {
				v = u;
			}
		}
	}

	if (v == UINT32_MAX) {
		return std::make_pair(-1, std::vector<uint32_t>{});
	}

	std::vector<uint32_t> path;
	while (v != s) {
		path.emplace_back(v);
		v = parent[v];
	}
	path.emplace_back(s);
	std::reverse(path.begin(), path.end());

	return std::make_pair(cost[t], path);
}

std::vector<std::vector<uint32_t>> yen(std::vector<std::vector<int32_t>> &adj,
									   uint32_t s, uint32_t t, uint32_t k) {
	std::vector<std::vector<uint32_t>> A;
	std::set<std::pair<int64_t, std::vector<uint32_t>>> B;

	{
		auto [cost, path] = dijkstra(adj, s, t);
		if (cost == -1) {
			return A;
		}
		A.emplace_back(path);
	}

	for (size_t i = 1; i < k; ++i) {
		std::vector<uint32_t> rootPath;
		uint64_t rootPathCost = 0;

		for (size_t j = 0; j < A[i - 1].size() - 1; ++j) {
			std::vector<std::tuple<uint32_t, uint32_t, int32_t>> deletedEdges;

			for (auto &p : A) {
				if (std::equal(std::begin(A[i - 1]),
							   std::begin(A[i - 1]) + (ssize_t)j + 1,
							   std::begin(p)) &&
					adj[p[j]][p[j + 1]] != -1) {
					deletedEdges.emplace_back(
						std::make_tuple(p[j], p[j + 1], adj[p[j]][p[j + 1]]));
					adj[p[j]][p[j + 1]] = -1;
				}
			}
			for (size_t x = 0; x < j; ++x) {
				deletedEdges.emplace_back(
					std::make_tuple(A[i - 1][x], A[i - 1][x + 1],
									adj[A[i - 1][x]][A[i - 1][x + 1]]));
				adj[A[i - 1][x]][A[i - 1][x + 1]] = -1;
			}

			auto [pathCost, path] = dijkstra(adj, A[i - 1][j], t);
			if (pathCost != -1) {
				path.insert(path.begin(), rootPath.begin(), rootPath.end());
				pathCost += rootPathCost;
				B.insert(std::make_pair(pathCost, path));
			}

			for (auto [a, b, w] : deletedEdges) {
				adj[a][b] = w;
			}

			rootPath.emplace_back(A[i - 1][j]);
			rootPathCost += (uint32_t)adj[A[i - 1][j]][A[i - 1][j + 1]];
		}

		if (B.size() == 0) {
			break;
		}

		auto shortestPath = B.extract(B.begin());
		A.emplace_back(std::move(shortestPath.value().second));
	}

	return A;
}

int handleConnection(int fd) {
	try {
		uint32_t n, m, k, s, t;

		uint32_t initialInput[5];
		size_t initialReceiveLen = 20;
		uint8_t *inputBuffer = (uint8_t *)initialInput;
		size_t receivedSoFar = 0;

		do {
			ssize_t bytesReceived = recv(fd, inputBuffer + receivedSoFar,
										 initialReceiveLen - receivedSoFar, 0);
			if (bytesReceived <= 0) {
				throw std::runtime_error("Failed to receive input from client");
			}
			receivedSoFar += (size_t)bytesReceived;
		} while (receivedSoFar < initialReceiveLen);

		n = ntohl(((uint32_t *)initialInput)[0]);
		m = ntohl(((uint32_t *)initialInput)[1]);
		k = ntohl(((uint32_t *)initialInput)[2]);
		s = ntohl(((uint32_t *)initialInput)[3]);
		t = ntohl(((uint32_t *)initialInput)[4]);

		if (n == 0 || n > MAX_N || m > MAX_M ||
			m > ((uint64_t)n) * (n - 1) / 2 || s >= n || t >= n || k > MAX_K) {
			throw std::runtime_error("Invalid data");
		}

		std::vector<uint32_t> edges(m * 3);
		uint8_t *edgesBuffer = (uint8_t *)edges.data();
		size_t edgesBufferLen = edges.size() * sizeof(uint32_t);

		receivedSoFar = 0;

		do {
			ssize_t bytesReceived = recv(fd, edgesBuffer + receivedSoFar,
										 edgesBufferLen - receivedSoFar, 0);
			if (bytesReceived <= 0) {
				throw std::runtime_error("Could not receive data from client");
			}
			receivedSoFar += (size_t)bytesReceived;

			assert(receivedSoFar <= edgesBufferLen);
		} while (receivedSoFar < edgesBufferLen);

		std::vector<std::vector<int32_t>> adj(n, std::vector<int32_t>(n, -1));

		for (size_t i = 0; i < 3 * m; i += 3) {
			uint32_t a = ntohl(edges[i]);
			uint32_t b = ntohl(edges[i + 1]);
			uint32_t w = ntohl(edges[i + 2]);

			if (a >= n || b >= n || a == b || w > MAX_EDGE_WEIGHT) {
				throw std::runtime_error("Invalid data");
			}
			adj[a][b] = (int32_t)w;
		}

		std::vector<uint32_t> result;

		for (auto &row : yen(adj, s, t, k)) {
			for (auto v : row) {
				result.emplace_back(htonl(v));
			}
		}

		result.insert(result.begin(),
					  htonl((uint32_t)result.size() * sizeof(uint32_t)));

		uint8_t *resultBuffer = (uint8_t *)result.data();
		size_t resultBufferLen = result.size() * sizeof(uint32_t);
		size_t sentSoFar = 0;

		do {
			ssize_t bytesSent = send(fd, resultBuffer + sentSoFar,
									 resultBufferLen - sentSoFar, 0);
			if (bytesSent <= 0) {
				throw std::runtime_error("Could not send data to client");
			}
			sentSoFar += (size_t)bytesSent;

			assert(sentSoFar <= resultBufferLen);
		} while (sentSoFar < resultBufferLen);
	} catch (std::exception &e) {
		std::cerr << e.what() << '\n';
		close(fd);
		return 1;
	}
	close(fd);
	return 0;
}

int main(int argc, char *argv[]) {
	int port = DEFAULT_PORT;

	if (argc > 1) {
		port = atoi(argv[1]);
		if (port <= 0 || port > (long)UINT16_MAX) {
			std::cerr << "Invalid port\n";
			return EXIT_FAILURE;
		}
	}

	sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((uint16_t)port);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		std::cerr << "Failed to create socket\n";
		return EXIT_FAILURE;
	}

	try {
		int reuseAddrEnabled = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddrEnabled,
					   sizeof(int)) < 0) {
			throw std::runtime_error("Failed to set socket options");
		}

		if (bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0) {
			throw std::runtime_error("Failed to bind socket to address");
		}
		if (listen(sock, MAX_CONNECTIONS) < 0) {
			throw std::runtime_error("Failed to listen on socket");
		}

		std::cout << "Listening for connections on port " << port << std::endl;

		while (1) {
			sockaddr_in clientAddr;
			socklen_t clientAddrLen = sizeof(clientAddr);
			int clientFd =
				accept(sock, (sockaddr *)&clientAddr, &clientAddrLen);
			if (clientFd == -1) {
				std::cerr << "Error accepting connection";
				continue;
			}

			char addrString[20];
			if (!inet_ntop(AF_INET, &clientAddr.sin_addr, addrString, 20)) {
				std::cerr << "Error parsing client address";
				continue;
			}

			std::cout << "Accepted connection from " << addrString << ':'
					  << ntohs(clientAddr.sin_port) << '\n';

			std::thread t(handleConnection, clientFd);
			t.detach();
		}

	} catch (std::exception &e) {
		std::cerr << e.what() << '\n';
		close(sock);
		return EXIT_FAILURE;
	}

	close(sock);
	return EXIT_SUCCESS;
}
