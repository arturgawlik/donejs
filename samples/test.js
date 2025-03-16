try {
    const resGetAddrInfo = globalThis.done.syscall.getaddrinfo(null, "5001", { ai_family: 'AF_UNSPEC', ai_socktype: 'SOCK_STREAM' });
    const resSocket = globalThis.done.syscall.socket(resGetAddrInfo.ai_family, resGetAddrInfo.ai_socktype, resGetAddrInfo.ai_protocol);
    const resConnect = globalThis.done.syscall.connect(resSocket, resGetAddrInfo.ai_addr, resGetAddrInfo.ai_addrlen);

    const bufferSize = 1024;
    const buffer = new ArrayBuffer(bufferSize);
    const recvConnect = globalThis.done.syscall.recv(resSocket, buffer, bufferSize, 0);

    const decoder = new TextDecoder();
    const decodedText = decoder.decode(buffer);

    console.log(`recv byte count: ${recvConnect}`);
    console.log(`recv data decoded: ${decodedText}`);
} catch (e) {
    console.log(e.message);
}
