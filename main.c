#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>

#define NUM_THREADS 64

char ip_interval[256][16];

void create_ip_intervals(char *target_ip_interval)
{
    target_ip_interval[15] = '\0';
    strcat(target_ip_interval, ".");
    for (int i = 0; i < 256; i++)
    {
        sprintf(ip_interval[i], "%s%d", target_ip_interval, i);
    }
}

void scan_ports(char *ip_addr)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Winsock is not initialized Error Code: %d\n", WSAGetLastError());
        return;
    }

    int sockfd;
    struct sockaddr_in target_addr;
    target_addr.sin_family = AF_INET;
    target_addr.sin_addr.s_addr = inet_addr(ip_addr);

    for (int port = 1; port <= 65537; port++)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == INVALID_SOCKET)
        {
            printf("Socket is not initialized Error Code: %d\n", WSAGetLastError());
            WSACleanup();
            return;
        }

        target_addr.sin_port = htons(port);

        if (connect(sockfd, (struct sockaddr *)&target_addr, sizeof(target_addr)) == 0)
        {
            FILE *file = fopen("ports.txt", "a+");
            fprintf(file, "IP:%s, Port:%d opened\n", ip_addr, port);
            fclose(file);
            printf("IP:%s, Port:%d opened\n", ip_addr, port);
        }
        else
        {
            printf("IP:%s, Port:%d closed\n", ip_addr, port);
        }

        closesocket(sockfd);
    }

    WSACleanup();
}

DWORD WINAPI thread_func(LPVOID lpParam)
{
    char *ip_addr = (char *)lpParam;

    scan_ports(ip_addr);
    return 0;
}

int main(int argc, char *argv[])
{
    HANDLE threads[NUM_THREADS];
    char target_ip_interval[16];

    if (argc != 2)
    {
        printf("Kullanım: %s <x.x.x>\n", argv[0]);
        return 1;
    }

    strncpy(target_ip_interval, argv[1], 15);
    create_ip_intervals(target_ip_interval);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads[i] = CreateThread(NULL, 0, thread_func, ip_interval[i], 0, NULL);
        if (threads[i] == NULL)
        {
            printf("Thread is not initialized Error Code: %d\n", GetLastError());
            return 1;
        }
    }

    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        CloseHandle(threads[i]);
    }

    return 0;
}
