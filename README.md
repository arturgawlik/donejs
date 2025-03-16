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
- [ ] basic `fetch`
