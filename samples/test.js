try {
    const resGetAddrInfo = globalThis.done.syscall.getaddrinfo("www.google.com", "http", { ai_family: 'AF_UNSPEC', ai_socktype: 'SOCK_STREAM' });
    const resSocket = globalThis.done.syscall.socket(resGetAddrInfo.ai_family, resGetAddrInfo.ai_socktype, resGetAddrInfo.ai_protocol);
    const resConnect = globalThis.done.syscall.connect(resSocket, resGetAddrInfo.ai_addr, resGetAddrInfo.ai_addrlen);
    console.log(resConnect);
} catch (e) {
    console.log(e.message);
}
