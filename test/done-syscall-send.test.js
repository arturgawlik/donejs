import assert from "done:assert.js";

const getaddrinfo = globalThis.done.syscall.getaddrinfo;
const socket = globalThis.done.syscall.socket;
const connect = globalThis.done.syscall.connect;
const recv = globalThis.done.syscall.recv;
const send = globalThis.done.syscall.send;
const close = globalThis.done.syscall.close;

// `send` syscall should send all bytes to some http server
(() => {
    const domain = 2; /* PF_INET */
    const type = 1; /* SOCK_STREAM */
    const protocol = 0; /* IPPROTO_IP */
    const host = "www.google.com";
    const port = "80";
    const hints = { ai_family: 'AF_UNSPEC', ai_socktype: 'SOCK_STREAM' };
    const resGetAddrInfo = {};

    getaddrinfo(host, port, hints, resGetAddrInfo);
    const fd = socket(domain, type, protocol);
    connect(fd, resGetAddrInfo.ai_addr, resGetAddrInfo.ai_addrlen);

    const encoder = new TextEncoder();
    const encodedText = encoder.encode('GET /\r\n\r\n');
    const encodedTextSize = encodedText.byteLength;
    const sendBytes = send(fd, encodedText, encodedTextSize, 0);

    assert.check(sendBytes === encodedTextSize, '`send` syscall should send all bytes.');

    // after test
    close(fd);
})();
