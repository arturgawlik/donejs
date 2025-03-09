try {
    // const res = globalThis.done.syscall.socket(2, 1, 0);
    const res = globalThis.done.syscall.getaddrinfo("www.google.com", "http", { ai_family: 'AF_UNSPEC', ai_socktype: 'SOCK_STREAM' });
    console.log(res.ai_addr);
} catch (e) {
    console.log(e.message);
}
