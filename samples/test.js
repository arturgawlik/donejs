async function myLog(msg) {
    const { myLogMsg } = await import("/home/artur-gawlik/git/donejs/samples/test2.js");
    myLogMsg(msg);
}

// myLog('Hello, World!');

for (let i = 0; i < 10000; i++)
    myLog(i);
