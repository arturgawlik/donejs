import assert from "done:assert.js";

const getaddrinfo = globalThis.done.syscall.getaddrinfo;
const socket = globalThis.done.syscall.socket;
const connect = globalThis.done.syscall.connect;
const close = globalThis.done.syscall.close;

// `connect` syscall should end without error
(() => {
    const domain = 2; /* PF_INET */
    const type = 1; /* SOCK_STREAM */
    const protocol = 0; /* IPPROTO_IP */
    const host = "www.google.com";
    const port = "443";
    const hints = { ai_family: 'AF_UNSPEC', ai_socktype: 'SOCK_STREAM' };
    const resGetAddrInfo = {};

    getaddrinfo(host, port, hints, resGetAddrInfo);
    const fd = socket(domain, type, protocol);
    const connectRes = connect(fd, resGetAddrInfo.ai_addr, resGetAddrInfo.ai_addrlen);

    assert.check(connectRes === 0, 'connection attempt did not finished without errors.');

    // after test
    close(fd);
})();
