hobby attempt to implement V8 based JS runtime

### RUNNING

```bash
$ make
$ ./done ./samples/test.js
```

### Features:

- [x] very basic `console.log`
- [x] es modules `import`
- [x] es modules dynamic `import()`
- [x] `exit(code: number)` implemented as winter TC proposed standard https://github.com/wintercg/proposal-cli-api?tab=readme-ov-file#exiting
- [x] `TextDecoder` with support only for `utf-8`/`utf8` decoding
- [x] `TextEncoder`
- [x] `prGocess.argv` all program arguments
- [x] `done.interpretArgv` function to retrieve all interpreted `argv`s
- [x] `done.syscall` exposes linux syscall's to js side
  - [x] `getaddrinfo`
  - [x] `socket`
  - [x] `connect`
  - [x] `recv`
  - [x] `send`
  - [x] `close`
  - [x] `glob`
  - [x] `posix_spawn`
  - [x] `waitpid`
- [x] `--test` flag which is very basic test runner (used in done.js to actually test js land)
- [x] basic `URL` (only features needed in `fetch`)
- [ ] basic `fetch`
