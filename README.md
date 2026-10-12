# Programming-Assignment-5

*I am submitting my screenshots as a PDF file

Problem Statement & Architecture:

For this assignment, the focus was on creating a TCP client-server application. The client requests a file from the server, and the server either sends what's in the file or returns an error message. The server uses TCP for reliable communication and listens on port 8080. It handles one client connection at a time and closes each connection after processing the request.

Socket Security & Buffer Analysis:

The server uses a 1024-byte buffer to receive filenames and checks the number of bytes received using recv() to help prevent buffer overflows. It also adds a null terminator to safely handle the data as a string. To prevent path traversal attacks, the server rejects filenames containing / or .. and limits filenames to 255 bytes.

Pros & Cons of Solution:

One pro of this implementation is that it is simple to understand, maintain, and troubleshoot. Processing one client at a time also makes it easier to manage connections and identify errors. However, this approach is less efficient when handling multiple clients because each request needs to finish before the next one can be processed. A forking server could handle multiple connections simultaneously.

AI Fuzzing Reflection:

I used ChatGPT to help generate a network fuzzer to test my server against path traversal attacks, oversized requests, unexpected disconnections, and null-byte injection. Initially, ChatGPT's net_fuzzer.c crashed and failed to connect to my server, so I had to troubleshoot and change the code to get it working. This experience showed me that AI-generated code isn't always reliable and may require additional debugging. Once I fixed the connection and crashing issues, I was able to use the fuzzer to test my server's security features. 
